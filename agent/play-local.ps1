[CmdletBinding()]
param(
    [string]$AccountName = 'Dummy00018',
    [string]$Password = '11111111',
    [int]$ReadinessTimeoutSeconds = 300,
    [int]$SmokeTimeoutSeconds = 90,
    [int]$RetryDelaySeconds = 5,
    [switch]$SkipReadinessSmoke,
    [ValidateSet('FastDev', 'Full')]
    [string]$ShardProfile = 'FastDev',
    [switch]$Full,
    [switch]$FullShard,
    [switch]$NoShardRestart,
    [switch]$RestartFastShard,
    [switch]$WebSwingDev
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$binDir = Join-Path $repoRoot 'bin'
$ouroboros = Join-Path $binDir 'Ouroboros.exe'
$webSwingInstaller = Join-Path $PSScriptRoot 'install-webswing-animation.ps1'
$directDbScript = Join-Path $PSScriptRoot 'set-directdb-mode.ps1'
$profileScript = Join-Path $PSScriptRoot 'set-shard-profile.ps1'
$startScript = Join-Path $PSScriptRoot 'start-shard.ps1'
$stopScript = Join-Path $PSScriptRoot 'stop-shard.ps1'
$statusScript = Join-Path $PSScriptRoot 'status.ps1'
$smokeScript = Join-Path $PSScriptRoot 'smoke.ps1'

function Invoke-ExistingScript {
    param([string]$Path, [string[]]$Arguments)
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Path @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$(Split-Path -Leaf $Path) failed with exit code $LASTEXITCODE." }
}

function Get-ShardProcessState {
    $required = @('ServerMonitor', 'DbServer', 'Launcher')
    $missing = @($required | Where-Object { -not (Get-Process -Name $_ -ErrorAction SilentlyContinue) })
    [pscustomobject]@{
        Healthy = ($missing.Count -eq 0)
        Missing = $missing
    }
}

function Test-ShardConfigChangedSinceStart {
    $monitor = Get-Process -Name ServerMonitor -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $monitor) { return $false }
    $startedUtc = $monitor.StartTime.ToUniversalTime()
    foreach ($path in @(
        (Join-Path $repoRoot 'bin\data\server\db\servers.cfg'),
        (Join-Path $repoRoot 'bin\data\server\db\loadBalanceShardSpecific.cfg')
    )) {
        if ((Get-Item -LiteralPath $path).LastWriteTimeUtc -gt $startedUtc) { return $true }
    }
    return $false
}

function Show-StatusSnapshot {
    if (Test-Path -LiteralPath $statusScript) {
        & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $statusScript
    }
}

function Invoke-JsonScript {
    param([string]$Path, [string[]]$Arguments)
    $output = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Path @Arguments 2>&1)
    $exitCode = $LASTEXITCODE
    if ($exitCode -ne 0) { throw "$(Split-Path -Leaf $Path) failed with exit code $exitCode. $($output -join ' ')" }
    try { return (($output | ForEach-Object { $_.ToString() }) -join "`n" | ConvertFrom-Json) }
    catch { throw "Could not parse $(Split-Path -Leaf $Path) JSON output: $($output -join ' ')" }
}

function Ensure-WebSwingAnimationRuntime {
    $status = Invoke-JsonScript -Path $webSwingInstaller -Arguments @('-Action', 'Status', '-RepositoryRoot', $repoRoot)
    $includeSynchronized = $status.includePresent -and
        ($status.includeSha256 -eq $status.trackedIncludeSha256)
    $stateBitsSynchronized = $status.stateBitsPresent -and
        ($status.stateBitsSha256 -eq $status.trackedStateBitsSha256)

    if (-not $status.installed -or -not $includeSynchronized -or -not $stateBitsSynchronized) {
        Write-Host 'Synchronizing tracked Web Swing animation data into loose runtime data...'
        $status = Invoke-JsonScript -Path $webSwingInstaller -Arguments @('-Action', 'Install', '-RepositoryRoot', $repoRoot)
    } else {
        Write-Host 'Web Swing animation runtime data is already synchronized.'
    }

    if (-not $status.installed -or
        $status.includeSha256 -ne $status.trackedIncludeSha256 -or
        $status.stateBitsSha256 -ne $status.trackedStateBitsSha256) {
        throw 'Web Swing animation runtime data did not reach tracked hash parity.'
    }
}

try {
    if (-not (Test-Path -LiteralPath $ouroboros)) {
        throw "Ouroboros.exe was not found at $ouroboros. Build the client first."
    }

    $clientWorkingDirectory = $binDir
    if ($WebSwingDev) {
        Ensure-WebSwingAnimationRuntime
        Write-Host "Web Swing development client: $ouroboros"
        Write-Host 'Web Swing development mode: explicit loose sequencer override'
    }

    $requestedProfile = if ($Full -or $FullShard) { 'Full' } else { $ShardProfile }
    Write-Host ("Starting local shard (profile {0})..." -f $requestedProfile)
    $beforeMode = Get-ShardProcessState
    $modeResult = Invoke-JsonScript -Path $directDbScript -Arguments @('-Enable', '-Json')
    $profileResult = Invoke-JsonScript -Path $profileScript -Arguments @('-Profile', $requestedProfile, '-Json')
    $modeChanged = [bool]$modeResult.changed
    $profileChanged = [bool]$profileResult.changed
    # A profile/configuration may have been changed by an explicit tooling
    # command while the shard was running. File timestamps close that gap so
    # the next normal PLAY-COH invocation self-heals instead of trusting a
    # process family that was started with older directives.
    $configChangedWhileRunning = Test-ShardConfigChangedSinceStart
    $restartRequired = $modeChanged -or $profileChanged -or $configChangedWhileRunning -or $RestartFastShard
    if ($restartRequired -and $beforeMode.Healthy) {
        if ($NoShardRestart) { throw 'The requested profile/configuration differs from the running shard, but -NoShardRestart was specified. Use the matching profile or allow the disposable shard to restart.' }
        Write-Host 'Profile or direct-DB configuration changed; restarting the disposable local shard automatically.'
        Invoke-ExistingScript -Path $stopScript -Arguments @('-ForceProcessStop')
    }

    $state = Get-ShardProcessState
    if (-not $state.Healthy) {
        Invoke-ExistingScript -Path $startScript -Arguments @('-StartupWaitSeconds', '30')
    } else {
        Write-Host 'Local shard is already running; leaving it in place.'
    }

    if (-not $SkipReadinessSmoke) {
        Write-Host 'Waiting for login readiness...'
        $deadline = [DateTime]::UtcNow.AddSeconds([math]::Max(1, $ReadinessTimeoutSeconds))
        $attempt = 0
        $lastReason = 'No readiness attempt completed.'
        do {
            $attempt++
            $state = Get-ShardProcessState
            if (-not $state.Healthy) {
                $lastReason = "Missing shard process(es): $($state.Missing -join ', ')."
            } else {
                $smokeOutput = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $smokeScript -AccountName $AccountName -TimeoutSeconds $SmokeTimeoutSeconds -Json 2>&1
                $smokeExit = $LASTEXITCODE
                $smokeResult = $null
                try { $smokeResult = ($smokeOutput -join "`n") | ConvertFrom-Json } catch {}
                if ($smokeExit -eq 0 -and $smokeResult -and $smokeResult.passed) {
                    Write-Host 'Shard ready.'
                    break
                }
                if ($smokeResult.reason) { $lastReason = $smokeResult.reason } else { $lastReason = 'Direct-DB readiness smoke did not pass.' }
            }
            if ([DateTime]::UtcNow -ge $deadline) {
                Show-StatusSnapshot
                throw "Shard did not become login-ready within $ReadinessTimeoutSeconds seconds. Last check: $lastReason"
            }
            Write-Host ("  Readiness attempt {0} not ready; retrying..." -f $attempt)
            Start-Sleep -Seconds ([math]::Max(1, $RetryDelaySeconds))
        } while ($true)
    } else {
        Write-Host 'Readiness smoke skipped by request.'
    }

    $existingClient = Get-Process -Name @('Ouroboros', 'Ouroboros_Debug') -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($existingClient) {
        Write-Host "City of Heroes is already running (PID $($existingClient.Id)); leaving it alone."
        exit 0
    }

    Write-Host 'Launching City of Heroes...'
    $clientArgs = @('-db', '127.0.0.1', '-authname', $AccountName, '-password', $Password, '-noverify', '-quicklogin', '1', '-noversioncheck', '-fullscreen', '0', '-screen', '1280', '720', '-stopinactivedisplay', '0')
    if ($WebSwingDev) { $clientArgs += @('-webswingdev', '-nopopups') }
    $client = Start-Process -FilePath $ouroboros -ArgumentList $clientArgs -WorkingDirectory $clientWorkingDirectory -PassThru
    Start-Sleep -Milliseconds 1500
    if (-not (Get-Process -Id $client.Id -ErrorAction SilentlyContinue)) {
        throw "Ouroboros.exe exited immediately after launch. Inspect the client/runtime logs under $binDir."
    }
    Write-Host "City of Heroes launched (PID $($client.Id))."
    exit 0
} catch {
    Write-Host ''
    Write-Host "PLAY-COH ERROR: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host 'Check the shard status and logs, then retry PLAY-COH.cmd.' -ForegroundColor Yellow
    exit 1
}
