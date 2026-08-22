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
    [switch]$WebSwingDev,
    [switch]$WebSwingCanary
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

function Get-LocalWorkflowClients {
    $expectedPaths = @($ouroboros, (Join-Path $binDir 'Ouroboros_Debug.exe')) |
        Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } |
        ForEach-Object { (Resolve-Path -LiteralPath $_).Path.ToLowerInvariant() }
    $managed = @()
    $unmanaged = @()

    $processes = @(Get-CimInstance Win32_Process | Where-Object {
        $_.Name -in @('Ouroboros.exe', 'Ouroboros_Debug.exe')
    })
    foreach ($process in $processes) {
        if (-not $process.ExecutablePath) { continue }
        $resolvedPath = try { [System.IO.Path]::GetFullPath($process.ExecutablePath).ToLowerInvariant() } catch { '' }
        if ($resolvedPath -notin $expectedPaths) { continue }

        $commandLine = [string]$process.CommandLine
        $client = [pscustomobject]@{
            Id = [int]$process.ProcessId
            Path = $resolvedPath
            CommandLine = $commandLine
            WebSwingDev = [bool]($commandLine -match '(?i)(^|\s)-webswingdev(?:\s|$)')
            DirectDbWorkflow = [bool]($commandLine -match '(?i)(^|\s)-db\s+127\.0\.0\.1(?:\s|$)')
        }
        if ($client.DirectDbWorkflow) { $managed += $client } else { $unmanaged += $client }
    }

    [pscustomobject]@{ Managed = @($managed); Unmanaged = @($unmanaged) }
}

function Stop-LocalWorkflowClients {
    param([object[]]$Clients)
    foreach ($client in @($Clients)) {
        $process = Get-Process -Id $client.Id -ErrorAction SilentlyContinue
        if (-not $process) { continue }
        Write-Host "Restarting local City of Heroes client PID $($client.Id) for the requested runtime mode."
        if ($process.MainWindowHandle -ne 0) { [void]$process.CloseMainWindow() }
        $deadline = [DateTime]::UtcNow.AddSeconds(10)
        do {
            Start-Sleep -Milliseconds 250
            $stillRunning = Get-Process -Id $client.Id -ErrorAction SilentlyContinue
        } while ($stillRunning -and [DateTime]::UtcNow -lt $deadline)
        if (Get-Process -Id $client.Id -ErrorAction SilentlyContinue) {
            Stop-Process -Id $client.Id -Force
        }
    }
}

function Ensure-WebSwingAnimationRuntime {
    $statusArguments = @('-Action', 'Status', '-RepositoryRoot', $repoRoot)
    if ($WebSwingCanary) { $statusArguments += '-IncludeCanary' }
    $status = Invoke-JsonScript -Path $webSwingInstaller -Arguments $statusArguments
    $expectedOverlayHash = if ($WebSwingCanary) { $status.trackedCanaryOverlaySha256 } else { $status.trackedOverlaySha256 }
    $expectedStateBitsHash = if ($WebSwingCanary) { $status.trackedCanaryStateBitsSha256 } else { $status.trackedStateBitsSha256 }
    $overlaySynchronized = $status.overlayPresent -and
        ($status.overlaySha256 -eq $expectedOverlayHash)
    $includeSynchronized = $status.includePresent -and
        ($status.includeSha256 -eq $status.trackedIncludeSha256)
    $stateBitsSynchronized = $status.stateBitsPresent -and
        ($status.stateBitsSha256 -eq $expectedStateBitsHash)
    $canaryIncludeSynchronized = (-not $WebSwingCanary) -or ($status.canaryIncludePresent -and
        ($status.canaryIncludeSha256 -eq $status.trackedCanaryIncludeSha256) -and
        $status.canaryAssetPresent)

    if (-not $status.installed -or -not $overlaySynchronized -or
        -not $includeSynchronized -or -not $stateBitsSynchronized -or
        -not $canaryIncludeSynchronized) {
        Write-Host 'Synchronizing tracked Web Swing animation data into loose runtime data...'
        $installArguments = @('-Action', 'Install', '-RepositoryRoot', $repoRoot)
        if ($WebSwingCanary) { $installArguments += '-IncludeCanary' }
        $status = Invoke-JsonScript -Path $webSwingInstaller -Arguments $installArguments
    } else {
        Write-Host 'Web Swing animation runtime data is already synchronized.'
    }

    if (-not $status.installed -or
        $status.overlaySha256 -ne $expectedOverlayHash -or
        $status.includeSha256 -ne $status.trackedIncludeSha256 -or
        $status.stateBitsSha256 -ne $expectedStateBitsHash -or
        (-not $status.animationAssetsRuntimeValid) -or
        ($WebSwingCanary -and $status.canaryIncludeSha256 -ne $status.trackedCanaryIncludeSha256)) {
        throw 'Web Swing animation runtime data did not reach tracked hash parity.'
    }
    return $status
}

try {
    if ($WebSwingCanary -and -not $WebSwingDev) {
        throw '-WebSwingCanary requires -WebSwingDev.'
    }

    if (-not (Test-Path -LiteralPath $ouroboros)) {
        throw "Ouroboros.exe was not found at $ouroboros. Build the client first."
    }

    $clientInventory = Get-LocalWorkflowClients
    if ($clientInventory.Unmanaged.Count -gt 0) {
        $pids = $clientInventory.Unmanaged.Id -join ', '
        throw "Found a local Ouroboros process not launched through the direct-DB workflow (PID $pids); refusing to manage or duplicate it."
    }

    $incompatibleClients = @($clientInventory.Managed | Where-Object { $_.WebSwingDev -ne [bool]$WebSwingDev })
    if ($incompatibleClients.Count -gt 0) {
        Stop-LocalWorkflowClients -Clients $incompatibleClients
    }

    $clientWorkingDirectory = $binDir
    if ($WebSwingDev) {
        $runtimeStatusArguments = @('-Action', 'Status', '-RepositoryRoot', $repoRoot)
        if ($WebSwingCanary) { $runtimeStatusArguments += '-IncludeCanary' }
        $runtimeStatus = Invoke-JsonScript -Path $webSwingInstaller -Arguments $runtimeStatusArguments
        $expectedOverlayHash = if ($WebSwingCanary) { $runtimeStatus.trackedCanaryOverlaySha256 } else { $runtimeStatus.trackedOverlaySha256 }
        $expectedStateBitsHash = if ($WebSwingCanary) { $runtimeStatus.trackedCanaryStateBitsSha256 } else { $runtimeStatus.trackedStateBitsSha256 }
        $runtimeSynchronized = $runtimeStatus.installed -and
            $runtimeStatus.overlayPresent -and
            $runtimeStatus.includePresent -and $runtimeStatus.stateBitsPresent -and
            $runtimeStatus.overlaySha256 -eq $expectedOverlayHash -and
            $runtimeStatus.includeSha256 -eq $runtimeStatus.trackedIncludeSha256 -and
            $runtimeStatus.stateBitsSha256 -eq $expectedStateBitsHash -and
            $runtimeStatus.animationAssetsRuntimeValid -and
            ((-not $WebSwingCanary) -or ($runtimeStatus.canaryIncludePresent -and
                $runtimeStatus.canaryIncludeSha256 -eq $runtimeStatus.trackedCanaryIncludeSha256 -and
                $runtimeStatus.canaryAssetPresent))
        if (-not $runtimeSynchronized) {
            $compatibleClients = @($clientInventory.Managed | Where-Object { $_.WebSwingDev })
            if ($compatibleClients.Count -gt 0) {
                Stop-LocalWorkflowClients -Clients $compatibleClients
            }
        }
        Ensure-WebSwingAnimationRuntime
        Write-Host "Web Swing development client: $ouroboros"
        $modeDescription = if ($WebSwingCanary) { 'compiled player plus explicit animation canary audition' } else { 'compiled player plus five-state custom Web Swing overlay' }
        Write-Host "Web Swing development mode: $modeDescription"
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

    $clientInventory = Get-LocalWorkflowClients
    if ($clientInventory.Unmanaged.Count -gt 0) {
        $pids = $clientInventory.Unmanaged.Id -join ', '
        throw "Found a local Ouroboros process not launched through the direct-DB workflow (PID $pids); refusing to manage or duplicate it."
    }
    $existingClient = @($clientInventory.Managed | Where-Object { $_.WebSwingDev -eq [bool]$WebSwingDev }) | Select-Object -First 1
    if ($existingClient) {
        Write-Host "City of Heroes is already running in the requested mode (PID $($existingClient.Id)); leaving the client and warm shard in place."
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
