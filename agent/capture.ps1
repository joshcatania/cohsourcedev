[CmdletBinding()]
param(
    [string]$Target = 'AtlasPlaza_CityHall_03',
    [string]$AccountName = 'Dummy00010',
    [string]$Password = '11111111',
    [int]$TimeoutSeconds = 180,
    [int]$Width = 1280,
    [int]$Height = 720,
    [switch]$Json
)
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$binRoot = Join-Path $repoRoot 'bin'
$ouroboros = Join-Path $binRoot 'Ouroboros.exe'
$ouroPdb = Join-Path $binRoot 'Ouroboros.pdb'
$screenshotRoot = Join-Path $binRoot 'screenshots'
$captureRoot = Join-Path $repoRoot 'agent\captures'
$logRoot = Join-Path $repoRoot 'agent\logs'
$serverConfig = Join-Path $binRoot 'data\server\db\servers.cfg'
$dumpStk = Join-Path $repoRoot 'Utilities\dumpstk\bin\x86\Release\dumpstk.exe'
$dumpHelper = Join-Path $PSScriptRoot 'dump-process.ps1'
$statusScript = Join-Path $PSScriptRoot 'status.ps1'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$safeTarget = $Target -replace '[^A-Za-z0-9_-]', '_'
$stdoutPath = Join-Path $logRoot "capture-$safeTarget-$stamp.stdout.log"
$stderrPath = Join-Path $logRoot "capture-$safeTarget-$stamp.stderr.log"
$resultPath = Join-Path $logRoot "capture-$safeTarget-$stamp.json"
$dumpPath = Join-Path $logRoot "capture-$safeTarget-$stamp.dmp"
$dumpMetaPath = "$dumpPath.json"
$dumpStkPath = Join-Path $logRoot "capture-$safeTarget-$stamp.stack.log"
$serverStatusPath = Join-Path $logRoot "capture-$safeTarget-$stamp.server.json"
$outputPath = Join-Path $captureRoot "$safeTarget.jpg"
$startedAt = Get-Date
New-Item -ItemType Directory -Force -Path $screenshotRoot, $captureRoot, $logRoot | Out-Null
function Get-LastStartupTrace {
    param([string]$BinRoot, [string]$LogRoot)
    $candidates = @()
    foreach ($dir in @($BinRoot, (Join-Path $BinRoot 'logs'), $LogRoot, $repoRoot)) {
        if (Test-Path -LiteralPath $dir) {
            $candidates += Get-ChildItem -Path $dir -Filter '*startup*.trace' -File -Recurse -ErrorAction SilentlyContinue
        }
    }
    $latest = $candidates | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1
    if (-not $latest) { return @{ path=$null; lastMarker=$null; tail=@(); exists=$false } }
    $lines = Get-Content -LiteralPath $latest.FullName -Tail 50 -ErrorAction SilentlyContinue
    $last = $lines | Where-Object { $_ -match 'marker=' } | Select-Object -Last 1
    return @{ path=$latest.FullName; lastMarker=$last; tail=$lines; exists=$true; lastWriteTimeUtc=$latest.LastWriteTimeUtc.ToString('o') }
}
function Get-ProcessSnapshot {
    param([int]$ProcessId, [string]$WorkingDirectory)
    try {
        $cim = Get-CimInstance Win32_Process -Filter "ProcessId=$ProcessId" -ErrorAction Stop
        $proc = Get-Process -Id $ProcessId -ErrorAction Stop
        return [ordered]@{
            pid=$ProcessId; executablePath=$cim.ExecutablePath; commandLine=$cim.CommandLine; parentProcessId=$cim.ParentProcessId
            creationDate=$cim.CreationDate; workingDirectory=$WorkingDirectory; processStartTime=$proc.StartTime.ToString('o')
            responding=$proc.Responding; workingSetBytes=$proc.WorkingSet64; threadCount=$proc.Threads.Count
        }
    } catch { return @{ error=$_.Exception.Message; pid=$ProcessId } }
}
function New-CaptureResult {
    param([bool]$Passed,[string]$Reason,[int]$ExitCode,[bool]$TimedOut,[string]$ScreenshotPath,
          [int]$ProcessId,[string]$CommandLine,[string]$WorkingDirectory,[object]$ProcessSnapshot,
          [string]$DumpPath,[string]$DumpMetaPath,[bool]$DumpSucceeded,[string]$DumpError,
          [string]$DumpStkPath,[string]$TracePath,[string]$LastTraceMarker,[string[]]$TraceTail,
          [string]$ServerStatusPath,[object]$ServerStatus)
    [ordered]@{
        passed=$Passed; reason=$Reason; target=$Target; accountName=$AccountName; exitCode=$ExitCode; timedOut=$TimedOut
        screenshot=$ScreenshotPath; pid=$ProcessId; commandLine=$CommandLine; workingDirectory=$WorkingDirectory; processSnapshot=$ProcessSnapshot
        dumpPath=$DumpPath; dumpMetadata=$DumpMetaPath; dumpSucceeded=$DumpSucceeded; dumpError=$DumpError; dumpStkOutput=$DumpStkPath
        tracePath=$TracePath; lastTraceMarker=$LastTraceMarker; traceTail=$TraceTail
        serverStatusPath=$ServerStatusPath; serverStatus=$ServerStatus
        startedAt=$startedAt.ToString('o'); finishedAt=(Get-Date).ToString('o')
        stdout=$stdoutPath; stderr=$stderrPath; result=$resultPath
    }
}
try {
    if (-not (Test-Path -LiteralPath $ouroboros)) { throw "Ouroboros.exe was not found at $ouroboros" }
    if (-not (Test-Path -LiteralPath $serverConfig)) { throw "DbServer configuration was not found at $serverConfig" }
    $configText = Get-Content -Raw -LiteralPath $serverConfig
    if ($configText -notmatch '(?m)^\s*UseFakeAuth\s+1\s*$') { throw 'Direct-DB capture requires UseFakeAuth 1 in servers.cfg' }
    $requiredProcesses = @('ServerMonitor','DBServer','Launcher')
    $missing = @($requiredProcesses | Where-Object { -not (Get-Process -Name $_ -ErrorAction SilentlyContinue) })
    if ($missing.Count -gt 0) { throw "Required shard processes are missing: $($missing -join ', ')" }
    if (Get-Process -Name 'Ouroboros' -ErrorAction SilentlyContinue) { throw 'An Ouroboros.exe process is already running' }
    $before = @{}
    Get-ChildItem -LiteralPath $screenshotRoot -Filter '*.jpg' -File -ErrorAction SilentlyContinue | ForEach-Object { $before[$_.FullName]=$_.LastWriteTimeUtc }
    $arguments = @('-db','127.0.0.1','-authname',$AccountName,'-password',$Password,'-noverify','-quicklogin','1','-noversioncheck','-capture',$Target,'-fullscreen','0','-screen',$Width.ToString(),$Height.ToString(),'-stopinactivedisplay','0')
    $argString = ($arguments | ForEach-Object { if ($_ -match '[\s"]') { '"' + ($_ -replace '"','\"') + '"' } else { $_ } }) -join ' '
    # Start-Process -PassThru with redirected streams never populates ExitCode on
    # this PowerShell build, and DataReceivedEventHandler scriptblocks run on
    # threadpool threads where they crash the host. Instead, let cmd.exe perform
    # the redirection and record the client's real exit code via !ERRORLEVEL!.
    $exitCodePath = Join-Path $logRoot "capture-$safeTarget-$stamp.exitcode"
    $cmdLine = '/v:on /c ""' + $ouroboros + '" ' + $argString + ' 1> "' + $stdoutPath + '" 2> "' + $stderrPath + '" & echo !ERRORLEVEL!> "' + $exitCodePath + '""'
    $client = Start-Process -FilePath $env:ComSpec -ArgumentList $cmdLine -WorkingDirectory $binRoot -PassThru
    Start-Sleep -Seconds 2
    $ouroProc = Get-Process -Name 'Ouroboros' -ErrorAction SilentlyContinue | Select-Object -First 1
    $ouroPid = if ($ouroProc) { $ouroProc.Id } else { 0 }
    $processSnapshot = Get-ProcessSnapshot -ProcessId $ouroPid -WorkingDirectory $binRoot
    $commandLine = if ($processSnapshot.commandLine) { $processSnapshot.commandLine } else { "$ouroboros $argString" }
    $exited = $client.WaitForExit($TimeoutSeconds*1000)
    $clientExitCode = $null
    if ($exited) {
        if (Test-Path -LiteralPath $exitCodePath) {
            $clientExitCode = [int](Get-Content -Raw -LiteralPath $exitCodePath).Trim()
        }
        if ($null -eq $clientExitCode) { $clientExitCode = -1 }
    }
    $dumpSucceeded=$false; $dumpError=$null; $dumpStkOutput=$null
    if (-not $exited) {
        try {
            if (Test-Path -LiteralPath $dumpHelper) {
                $dumpArgs = @('-TargetPid',$ouroPid,'-OutputPath',$dumpPath,'-MetadataPath',$dumpMetaPath,'-WorkingDirectory',$binRoot,'-Json')
                $dumpJson = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $dumpHelper @dumpArgs 2>&1 | Out-String
                if (Test-Path -LiteralPath $dumpPath) { $dumpSucceeded=$true } else { $dumpError=$dumpJson }
                if ($dumpSucceeded -and (Test-Path -LiteralPath $dumpStk) -and (Test-Path -LiteralPath $ouroPdb)) {
                    & $dumpStk -f $dumpPath -i $ouroboros -y $binRoot 2>&1 | Set-Content -LiteralPath $dumpStkPath -Encoding UTF8
                    if (Test-Path -LiteralPath $dumpStkPath) { $dumpStkOutput=$dumpStkPath }
                }
            }
        } catch { $dumpError=$_.Exception.Message }
        Stop-Process -Id $ouroPid -Force -ErrorAction SilentlyContinue
        Stop-Process -Id $client.Id -Force -ErrorAction SilentlyContinue
        Start-Sleep -Milliseconds 800
        $trace = Get-LastStartupTrace -BinRoot $binRoot -LogRoot $logRoot
        $serverStatus=$null
        try { if (Test-Path -LiteralPath $statusScript) { $serverStatus = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $statusScript -Json 2>$null | ConvertFrom-Json } } catch {}
        if ($serverStatus) { $serverStatus | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $serverStatusPath -Encoding UTF8 }
        $result = New-CaptureResult -Passed $false -Reason 'Ouroboros timed out before clean capture exit (dump attempted)' -ExitCode 124 -TimedOut $true -ScreenshotPath '' -ProcessId $ouroPid -CommandLine $commandLine -WorkingDirectory $binRoot -ProcessSnapshot $processSnapshot -DumpPath $dumpPath -DumpMetaPath $dumpMetaPath -DumpSucceeded $dumpSucceeded -DumpError $dumpError -DumpStkPath $dumpStkPath -TracePath $trace.path -LastTraceMarker $trace.lastMarker -TraceTail $trace.tail -ServerStatusPath $serverStatusPath -ServerStatus $serverStatus
    } else {
        if ($null -eq $clientExitCode) { $clientExitCode = -1 }
        $trace = Get-LastStartupTrace -BinRoot $binRoot -LogRoot $logRoot
        $serverStatus=$null
        try { if (Test-Path -LiteralPath $statusScript) { $serverStatus = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $statusScript -Json 2>$null | ConvertFrom-Json } } catch {}
        if ($serverStatus) { $serverStatus | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $serverStatusPath -Encoding UTF8 }
        $newScreenshots = @(Get-ChildItem -LiteralPath $screenshotRoot -Filter '*.jpg' -File -ErrorAction SilentlyContinue | Where-Object { $oldTime=$before[$_.FullName]; (-not $oldTime) -or $_.LastWriteTimeUtc -gt $oldTime } | Sort-Object LastWriteTimeUtc -Descending)
        if ($newScreenshots.Count -eq 0) {
            $result = New-CaptureResult -Passed $false -Reason 'Ouroboros exited without producing a JPG screenshot' -ExitCode $clientExitCode -TimedOut $false -ScreenshotPath '' -ProcessId $ouroPid -CommandLine $commandLine -WorkingDirectory $binRoot -ProcessSnapshot $processSnapshot -DumpPath $dumpPath -DumpMetaPath $dumpMetaPath -DumpSucceeded $false -DumpError $null -DumpStkPath $null -TracePath $trace.path -LastTraceMarker $trace.lastMarker -TraceTail $trace.tail -ServerStatusPath $serverStatusPath -ServerStatus $serverStatus
        } else {
            Copy-Item -LiteralPath $newScreenshots[0].FullName -Destination $outputPath -Force
            $isPass = ($clientExitCode -eq 0)
            $result = New-CaptureResult -Passed $isPass -Reason $(if ($isPass) {'Deterministic capture completed and Ouroboros exited cleanly'} else {'Capture produced an image but Ouroboros returned a non-zero exit code'}) -ExitCode $clientExitCode -TimedOut $false -ScreenshotPath $outputPath -ProcessId $ouroPid -CommandLine $commandLine -WorkingDirectory $binRoot -ProcessSnapshot $processSnapshot -DumpPath $dumpPath -DumpMetaPath $dumpMetaPath -DumpSucceeded $false -DumpError $null -DumpStkPath $null -TracePath $trace.path -LastTraceMarker $trace.lastMarker -TraceTail $trace.tail -ServerStatusPath $serverStatusPath -ServerStatus $serverStatus
        }
    }
} catch {
    $trace = Get-LastStartupTrace -BinRoot $binRoot -LogRoot $logRoot
    $result = New-CaptureResult -Passed $false -Reason $_.Exception.Message -ExitCode 1 -TimedOut $false -ScreenshotPath '' -Pid 0 -CommandLine '' -WorkingDirectory $binRoot -ProcessSnapshot $null -DumpPath $dumpPath -DumpMetaPath $dumpMetaPath -DumpSucceeded $false -DumpError $null -DumpStkPath $null -TracePath $trace.path -LastTraceMarker $trace.lastMarker -TraceTail $trace.tail -ServerStatusPath $null -ServerStatus $null
}
$result | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath $resultPath -Encoding UTF8
if ($Json) { $result | ConvertTo-Json -Depth 7 } elseif ($result.passed) { Write-Host "CAPTURE PASS - $($result.screenshot)"; Write-Host "Result: $resultPath" } else { Write-Host "CAPTURE FAIL - $($result.reason)"; Write-Host "Result: $resultPath"; if ($result.lastTraceMarker) { Write-Host "LastTrace: $($result.lastTraceMarker)" } }
if ($result.passed) { exit 0 } else { exit 1 }