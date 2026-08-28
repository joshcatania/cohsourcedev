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
        $status.canaryAssetPresent -and
        ($status.canaryAssetSha256 -eq $status.trackedCanaryAssetSha256))

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

function Clear-ClientCrashPrompt {
    param([string]$ShadowRegistryPath)
    foreach ($name in @('gameprogressuserstring','gameprogressdialogtype')) {
        $path = Join-Path $ShadowRegistryPath $name
        if (Test-Path -LiteralPath $path) {
            Remove-Item -LiteralPath $path -Force
        }
    }
}

function Get-LastStartupTrace {
    param([string]$BinRoot, [string]$LogRoot, [string]$RepoRoot)
    $candidates = @()
    foreach ($dir in @($BinRoot, (Join-Path $BinRoot 'logs'), $LogRoot, $RepoRoot)) {
        if (Test-Path -LiteralPath $dir) {
            $candidates += Get-ChildItem -Path $dir -Filter '*startup*.trace' -File -Recurse -ErrorAction SilentlyContinue
        }
    }
    $latest = $candidates | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1
    if (-not $latest) { return @{ path=$null; lastMarker=$null; tail=@(); exists=$false; lastWriteTimeUtc=$null; currentForLaunch=$false } }
    $rawLines = Get-Content -LiteralPath $latest.FullName -Tail 50 -ErrorAction SilentlyContinue
    $lines = @($rawLines | ForEach-Object { [string]$_ })
    $last = $lines | Where-Object { $_ -match 'marker=' } | Select-Object -Last 1
    if ($last) { $last = [string]$last }
    return @{ path=$latest.FullName; lastMarker=$last; tail=@($lines); exists=$true; lastWriteTimeUtc=$latest.LastWriteTimeUtc.ToString('o'); currentForLaunch=$false }
}

function Format-RedactedClientArgs {
    param([string[]]$Arguments)
    $redacted = @()
    $skipNext = $false
    for ($i = 0; $i -lt $Arguments.Count; $i++) {
        if ($skipNext) { $redacted += "<redacted>"; $skipNext = $false; continue }
        if ($Arguments[$i] -eq "-password") { $redacted += "-password"; $skipNext = $true; continue }
        $redacted += $Arguments[$i]
    }
    return $redacted
}

function Get-RecentClientLogs {
    param([DateTime]$LaunchStartedUtc, [string]$BinRoot, [string]$LogRoot, [string]$RepoRoot)
    $roots = @($BinRoot, (Join-Path $BinRoot 'logs'), (Join-Path $BinRoot 'logs/game'), (Join-Path $BinRoot 'logs/client'), $LogRoot, $RepoRoot) | Where-Object { Test-Path -LiteralPath $_ }
    $candidates = @()
    foreach ($root in $roots) {
        $candidates += Get-ChildItem -Path $root -Filter "*.log" -File -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTimeUtc -ge $LaunchStartedUtc.AddSeconds(-3) }
        $candidates += Get-ChildItem -Path $root -Filter "*.txt" -File -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTimeUtc -ge $LaunchStartedUtc.AddSeconds(-3) }
        $candidates += Get-ChildItem -Path $root -Filter "*.trace" -File -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTimeUtc -ge $LaunchStartedUtc.AddSeconds(-3) }
    }
    $known = @((Join-Path $BinRoot 'Ouroboros.log'), (Join-Path $BinRoot 'logs/game/webswing.log'), (Join-Path $BinRoot 'logs/client/webswing.log'))
    foreach ($p in $known) {
        if (Test-Path -LiteralPath $p) {
            $item = Get-Item -LiteralPath $p
            if ($candidates.FullName -notcontains $item.FullName) { $candidates += $item }
        }
    }
    $sorted = $candidates | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 8
    $result = @()
    foreach ($file in $sorted) {
        try {
            $rawLines = Get-Content -LiteralPath $file.FullName -Tail 40 -ErrorAction SilentlyContinue
            $lines = @($rawLines | ForEach-Object { [string]$_ })
            $isCurrent = $file.LastWriteTimeUtc -ge $LaunchStartedUtc.AddSeconds(-3)
            $result += [ordered]@{ path=$file.FullName; lastWriteTimeUtc=$file.LastWriteTimeUtc.ToString('o'); currentForLaunch=$isCurrent; tail=@($lines) }
        } catch {
            $result += [ordered]@{ path=$file.FullName; error=$_.Exception.Message }
        }
    }
    return $result
}

function Write-ClientLaunchDiagnostics {
    param(
        [System.Diagnostics.Process]$Client,
        [DateTime]$LaunchStartedUtc,
        [string[]]$ClientArgs,
        [bool]$WebSwingDev,
        [bool]$WebSwingCanary,
        [object]$WebSwingRuntimeStatus
    )
    $logRoot = Join-Path $repoRoot 'agent/logs'
    New-Item -ItemType Directory -Force -Path $logRoot | Out-Null
    $stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $diagPath = Join-Path $logRoot "play-client-launch-$stamp.json"
    $redactedArgs = Format-RedactedClientArgs -Arguments $ClientArgs
    $exitCode = $null
    try { $Client.Refresh(); $exitCode = $Client.ExitCode } catch { $exitCode = -1 }
    $trace = Get-LastStartupTrace -BinRoot $binDir -LogRoot $logRoot -RepoRoot $repoRoot
    if ($trace.exists -and $trace.lastWriteTimeUtc) {
        try { $traceTime = [DateTime]::Parse($trace.lastWriteTimeUtc); $trace.currentForLaunch = $traceTime -ge $LaunchStartedUtc.AddSeconds(-3) } catch { $trace.currentForLaunch = $false }
    }
    $recentLogs = Get-RecentClientLogs -LaunchStartedUtc $LaunchStartedUtc -BinRoot $binDir -LogRoot $logRoot -RepoRoot $repoRoot
    $shardStatus = $null
    try { $shardStatus = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $statusScript -Json 2>$null | ConvertFrom-Json } catch {}
    $webSwingRuntime = $null
    if ($WebSwingRuntimeStatus) {
        $webSwingRuntime = [ordered]@{
            installed=$WebSwingRuntimeStatus.installed
            overlayPresent=$WebSwingRuntimeStatus.overlayPresent
            overlaySha256=$WebSwingRuntimeStatus.overlaySha256
            trackedOverlaySha256=$WebSwingRuntimeStatus.trackedOverlaySha256
            includePresent=$WebSwingRuntimeStatus.includePresent
            includeSha256=$WebSwingRuntimeStatus.includeSha256
            trackedIncludeSha256=$WebSwingRuntimeStatus.trackedIncludeSha256
            stateBitsPresent=$WebSwingRuntimeStatus.stateBitsPresent
            stateBitsSha256=$WebSwingRuntimeStatus.stateBitsSha256
            trackedStateBitsSha256=$WebSwingRuntimeStatus.trackedStateBitsSha256
            animationAssetsRuntimeValid=$WebSwingRuntimeStatus.animationAssetsRuntimeValid
        }
        if ($WebSwingCanary) {
            $webSwingRuntime.canaryIncludePresent = $WebSwingRuntimeStatus.canaryIncludePresent
            $webSwingRuntime.canaryIncludeSha256 = $WebSwingRuntimeStatus.canaryIncludeSha256
            $webSwingRuntime.trackedCanaryIncludeSha256 = $WebSwingRuntimeStatus.trackedCanaryIncludeSha256
        }
    }
    $diag = [ordered]@{
        passed=$false
        reason="Ouroboros exited during startup probe"
        startedAt=$LaunchStartedUtc.ToString('o')
        finishedAt=[DateTime]::UtcNow.ToString('o')
        pid=($Client.Id)
        exitCode=$exitCode
        executable=$ouroboros
        workingDirectory=$binDir
        arguments=@($redactedArgs)
        webSwingDev=[bool]$WebSwingDev
        webSwingCanary=[bool]$WebSwingCanary
        webSwingRuntime=$webSwingRuntime
        startupTrace=$trace
        recentLogs=@($recentLogs)
        shardStatus=$shardStatus
    }
    $diag | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath $diagPath -Encoding UTF8
    return $diagPath
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
    $webSwingRuntimeStatus = $null
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
                $runtimeStatus.canaryAssetPresent -and
                $runtimeStatus.canaryAssetSha256 -eq $runtimeStatus.trackedCanaryAssetSha256))
        if (-not $runtimeSynchronized) {
            $compatibleClients = @($clientInventory.Managed | Where-Object { $_.WebSwingDev })
            if ($compatibleClients.Count -gt 0) {
                Stop-LocalWorkflowClients -Clients $compatibleClients
            }
        }
        $webSwingRuntimeStatus = Ensure-WebSwingAnimationRuntime
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
    if ($WebSwingDev) {
        # WebSwingDev's human-ready profile is deliberately explicit: the
        # character, server backend request, and authored mode-3 animation
        # selector must not depend on account ordering or normal defaults.
        $clientArgs += @('-capturecharacter', 'Swingv3', '-nosharedmemory', '-webswingdev', '-webswingphysics', '1', '-webswinganim', '3', '-nopopups')
    }
    if ($WebSwingCanary) { $clientArgs += @('-animcanary', '1') }

    $shadowRegistry = Join-Path $binDir 'registry-keys/hkey_current_user/software/cryptic/coh'
    if (Test-Path -LiteralPath $shadowRegistry) {
        Clear-ClientCrashPrompt -ShadowRegistryPath $shadowRegistry
    }

    $launchStartedUtc = [DateTime]::UtcNow
    $client = Start-Process -FilePath $ouroboros -ArgumentList $clientArgs -WorkingDirectory $clientWorkingDirectory -PassThru
    $exitedDuringStartup = $client.WaitForExit(5000)
    if ($exitedDuringStartup) {
        $exitCode = -1
        try { $client.Refresh(); $exitCode = $client.ExitCode } catch { $exitCode = -1 }
        $diagPath = Write-ClientLaunchDiagnostics -Client $client -LaunchStartedUtc $launchStartedUtc -ClientArgs $clientArgs -WebSwingDev ([bool]$WebSwingDev) -WebSwingCanary ([bool]$WebSwingCanary) -WebSwingRuntimeStatus $webSwingRuntimeStatus
        $traceInfo = Get-LastStartupTrace -BinRoot $binDir -LogRoot (Join-Path $repoRoot 'agent/logs') -RepoRoot $repoRoot
        $startupMarker = if ($traceInfo.lastMarker) { $traceInfo.lastMarker } else { "<unavailable>" }
        $webSwingParity = "UNKNOWN"
        if ($webSwingRuntimeStatus) {
            $expectedOverlay = if ($WebSwingCanary) { $webSwingRuntimeStatus.trackedCanaryOverlaySha256 } else { $webSwingRuntimeStatus.trackedOverlaySha256 }
            $expectedStateBits = if ($WebSwingCanary) { $webSwingRuntimeStatus.trackedCanaryStateBitsSha256 } else { $webSwingRuntimeStatus.trackedStateBitsSha256 }
            $parity = ($webSwingRuntimeStatus.overlaySha256 -eq $expectedOverlay) -and ($webSwingRuntimeStatus.stateBitsSha256 -eq $expectedStateBits) -and $webSwingRuntimeStatus.animationAssetsRuntimeValid
            $webSwingParity = if ($parity) { "PASS" } else { "FAIL" }
        } elseif (-not $WebSwingDev) {
            $webSwingParity = "N/A (no WebSwingDev)"
        }
        Write-Host ''
        Write-Host 'PLAY-COH CLIENT STARTUP FAILURE' -ForegroundColor Red
        Write-Host "Exit code: $exitCode"
        Write-Host "Startup marker: $startupMarker"
        Write-Host "Web Swing runtime parity: $webSwingParity"
        Write-Host "Diagnostic: $diagPath"
        if ($traceInfo.tail -and $traceInfo.tail.Count -gt 0) {
            Write-Host '--- Startup trace tail (last 5) ---'
            $traceInfo.tail | Select-Object -Last 5 | ForEach-Object { Write-Host $_ }
        }
        $recentLogs = Get-RecentClientLogs -LaunchStartedUtc $launchStartedUtc -BinRoot $binDir -LogRoot (Join-Path $repoRoot 'agent/logs') -RepoRoot $repoRoot
        if ($recentLogs -and $recentLogs.Count -gt 0) {
            $firstLog = $recentLogs | Select-Object -First 1
            if ($firstLog.tail -and $firstLog.tail.Count -gt 0) {
                Write-Host ("--- Recent log tail: {0} ---" -f $firstLog.path)
                $firstLog.tail | Select-Object -Last 5 | ForEach-Object { Write-Host $_ }
            }
        }
        throw "Ouroboros.exe exited during startup probe (exit code $exitCode). Diagnostic: $diagPath"
    }

    Write-Host "City of Heroes launched (PID $($client.Id))."
    exit 0
} catch {
    Write-Host ''
    Write-Host "PLAY-COH ERROR: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host 'Check the shard status and logs, then retry PLAY-COH.cmd.' -ForegroundColor Yellow
    exit 1
}
