[CmdletBinding()]
param(
    [ValidateSet('Current', 'Full', 'FastDev')]
    [string]$Profile = 'Current',
    [string]$AccountName = 'Dummy00036',
    [ValidateSet('On', 'Off')]
    [string]$TsrMode = 'Off',
    [switch]$DisableChatServer,
    [int]$StartupWaitSeconds = 30,
    [int]$LoginAttemptTimeoutSeconds = 45,
    [int]$MapAttemptTimeoutSeconds = 180,
    [int]$OverallTimeoutSeconds = 600,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$logDir = Join-Path $PSScriptRoot 'logs'
$startScript = Join-Path $PSScriptRoot 'start-shard.ps1'
$stopScript = Join-Path $PSScriptRoot 'stop-shard.ps1'
$directDbScript = Join-Path $PSScriptRoot 'set-directdb-mode.ps1'
$profileScript = Join-Path $PSScriptRoot 'set-shard-profile.ps1'
$smokeScript = Join-Path $PSScriptRoot 'smoke.ps1'
New-Item -ItemType Directory -Force -Path $logDir | Out-Null

$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$resultPath = Join-Path $logDir "benchmark-shard-startup-$Profile-$stamp.json"
$startOutputPath = Join-Path $logDir "benchmark-shard-startup-$Profile-$stamp.start.log"
$startErrorPath = Join-Path $logDir "benchmark-shard-startup-$Profile-$stamp.start.err.log"
$processNames = @('ServerMonitor', 'DbServer', 'Launcher')
$clientNames = @('Ouroboros', 'TestClient')

function Invoke-ToolScript {
    param([string]$Path, [string[]]$Arguments)
    $output = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Path @Arguments 2>&1)
    $exitCode = $LASTEXITCODE
    if ($exitCode -ne 0) {
        throw "$(Split-Path -Leaf $Path) failed with exit code $exitCode. $($output -join ' ')"
    }
    return $output
}

function Get-ProcessPresence([string]$Name) {
    return $null -ne (Get-Process -Name $Name -ErrorAction SilentlyContinue)
}

function Get-RelativeSeconds([DateTime]$Now, [DateTime]$Origin) {
    return [math]::Round(($Now.ToUniversalTime() - $Origin).TotalSeconds, 2)
}

function Read-JsonOutput([object[]]$Output) {
    $text = ($Output | ForEach-Object { $_.ToString() }) -join "`n"
    try { return ($text | ConvertFrom-Json) } catch {
        throw "Could not parse JSON tool output: $text"
    }
}

function Invoke-Smoke([switch]$ExerciseCharacter) {
    $argsList = @('-AccountName', $AccountName, '-TimeoutSeconds', $(if ($ExerciseCharacter) { $MapAttemptTimeoutSeconds } else { $LoginAttemptTimeoutSeconds }), '-Json')
    if ($ExerciseCharacter) { $argsList += '-ExerciseCharacter' }
    $output = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $smokeScript @argsList 2>&1)
    $exitCode = $LASTEXITCODE
    $result = $null
    try { $result = Read-JsonOutput $output } catch {
        $result = [pscustomobject]@{ passed = $false; reason = "Smoke JSON was unavailable (exit $exitCode): $($output -join ' ')" }
    }
    return [pscustomobject]@{ exitCode = $exitCode; result = $result }
}

function Stop-LocalShard {
    $running = @()
    foreach ($name in ($processNames + @('MapServer','Mapserver-TSR','AccountServer','ChatServer','AuctionServer','MissionServer','TurnstileServer','QueueServer','StatServer','ArenaServer','RaidServer','LogServer','BeaconServer','BeaconClient'))) {
        $running += @(Get-Process -Name $name -ErrorAction SilentlyContinue)
    }
    if ($running.Count -gt 0) {
        [void](Invoke-ToolScript -Path $stopScript -Arguments @('-ForceProcessStop', '-Json'))
    }
}

foreach ($name in $clientNames) {
    if (Get-ProcessPresence $name) {
        throw "Refusing to benchmark while $name is running. Close Ouroboros/TestClient first."
    }
}

# A benchmark always starts from a disposable, explicitly configured local shard.
Stop-LocalShard
[void](Invoke-ToolScript -Path $directDbScript -Arguments @('-Enable', '-Json'))
if ($Profile -ne 'Current') {
    if (-not (Test-Path -LiteralPath $profileScript)) {
        throw "Shard profile tooling is not present yet: $profileScript"
    }
    $profileArgs = @('-Profile', $Profile, '-TsrMode', $TsrMode, '-Json')
    if ($DisableChatServer) { $profileArgs += '-DisableChatServer' }
    [void](Invoke-ToolScript -Path $profileScript -Arguments $profileArgs)
}

$t0 = [DateTime]::UtcNow
$watch = [System.Diagnostics.Stopwatch]::StartNew()
$observed = @{}
$startExit = $null
$startProcess = $null

$startProcess = Start-Process -FilePath 'powershell.exe' -ArgumentList @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $startScript, '-StartupWaitSeconds', $StartupWaitSeconds) -WorkingDirectory $repoRoot -WindowStyle Hidden -RedirectStandardOutput $startOutputPath -RedirectStandardError $startErrorPath -PassThru

$startupDeadline = $t0.AddSeconds([math]::Max(1, $OverallTimeoutSeconds))
while ([DateTime]::UtcNow -lt $startupDeadline) {
    foreach ($name in $processNames) {
        if (-not $observed.ContainsKey($name) -and (Get-ProcessPresence $name)) {
            $now = [DateTime]::UtcNow
            $observed[$name] = [pscustomobject]@{ observedAtUtc = $now.ToString('o'); secondsFromT0 = Get-RelativeSeconds $now $t0 }
        }
    }
    if ($observed.Count -eq $processNames.Count) { break }
    Start-Sleep -Milliseconds 250
}

if ($startProcess) {
    $startProcess.Refresh()
    if ($startProcess.HasExited) { $startExit = $startProcess.ExitCode }
}

if ($observed.Count -ne $processNames.Count) {
    throw "Startup did not expose all required processes before the benchmark timeout. Missing: $($processNames | Where-Object { -not $observed.ContainsKey($_) } -join ', '). See $startOutputPath and $startErrorPath"
}

function Wait-ForSmoke([switch]$ExerciseCharacter) {
    $deadline = $t0.AddSeconds([math]::Max(1, $OverallTimeoutSeconds))
    $attempts = @()
    while ([DateTime]::UtcNow -lt $deadline) {
        $attemptStart = [DateTime]::UtcNow
        $attempt = Invoke-Smoke -ExerciseCharacter:$ExerciseCharacter
        $attemptEnd = [DateTime]::UtcNow
        $attempts += [pscustomobject]@{
            startedAtUtc = $attemptStart.ToString('o')
            completedAtUtc = $attemptEnd.ToString('o')
            secondsFromT0 = Get-RelativeSeconds $attemptEnd $t0
            exitCode = $attempt.exitCode
            passed = [bool]$attempt.result.passed
            stage = $attempt.result.stage
            reason = $attempt.result.reason
            resultLog = $attempt.result.resultLog
            mapConnected = $attempt.result.mapConnected
            characterCreated = $attempt.result.characterCreated
        }
        if ($attempt.result.passed -and (-not $ExerciseCharacter -or ($attempt.result.mapConnected -and $attempt.result.characterCreated))) {
            return [pscustomobject]@{ passed = $true; completedAtUtc = $attemptEnd.ToString('o'); secondsFromT0 = Get-RelativeSeconds $attemptEnd $t0; attempts = $attempts }
        }
        Start-Sleep -Seconds 2
    }
    return [pscustomobject]@{ passed = $false; completedAtUtc = $null; secondsFromT0 = $null; attempts = $attempts }
}

$login = Wait-ForSmoke
$map = if ($login.passed) { Wait-ForSmoke -ExerciseCharacter } else { [pscustomobject]@{ passed = $false; completedAtUtc = $null; secondsFromT0 = $null; attempts = @() } }
$watch.Stop()

$result = [pscustomobject]@{
    benchmark = 'shard-startup'
    profile = $Profile
    accountName = $AccountName
    startedAtUtc = $t0.ToString('o')
    completedAtUtc = [DateTime]::UtcNow.ToString('o')
    elapsedWallSeconds = [math]::Round($watch.Elapsed.TotalSeconds, 2)
    serverMonitor = $observed['ServerMonitor']
    dbServer = $observed['DbServer']
    launcher = $observed['Launcher']
    startScriptExitCode = $startExit
    startScriptLog = $startOutputPath
    startScriptErrorLog = $startErrorPath
    firstDirectDbLogin = $login
    firstMapConnected = $map
    coldStartToMapConnectedSeconds = $map.secondsFromT0
    passed = [bool]$map.passed
    note = 'Map-connected is the completion of smoke.ps1 -ExerciseCharacter, which proves character creation and MapServer entry on the direct-DB path. Process observations are diagnostic only.'
}
$result | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath $resultPath -Encoding UTF8
if ($Json) {
    $result | ConvertTo-Json -Depth 12
} else {
    Write-Host ("Profile: {0}" -f $Profile)
    Write-Host ("ServerMonitor observed: {0:N2}s" -f $result.serverMonitor.secondsFromT0)
    Write-Host ("DbServer observed: {0:N2}s" -f $result.dbServer.secondsFromT0)
    Write-Host ("Launcher observed: {0:N2}s" -f $result.launcher.secondsFromT0)
    if ($result.coldStartToMapConnectedSeconds) {
        Write-Host ("COLD START -> MAP CONNECTED: {0:N2}s" -f $result.coldStartToMapConnectedSeconds)
    } else {
        Write-Host 'COLD START -> MAP CONNECTED: FAIL'
    }
    Write-Host "JSON result: $resultPath"
}
if (-not $result.passed) { exit 1 }
exit 0
