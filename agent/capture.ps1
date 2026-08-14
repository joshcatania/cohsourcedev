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
$screenshotRoot = Join-Path $binRoot 'screenshots'
$captureRoot = Join-Path $repoRoot 'agent\captures'
$logRoot = Join-Path $repoRoot 'agent\logs'
$serverConfig = Join-Path $binRoot 'data\server\db\servers.cfg'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$safeTarget = $Target -replace '[^A-Za-z0-9_-]', '_'
$stdoutPath = Join-Path $logRoot "capture-$safeTarget-$stamp.stdout.log"
$stderrPath = Join-Path $logRoot "capture-$safeTarget-$stamp.stderr.log"
$resultPath = Join-Path $logRoot "capture-$safeTarget-$stamp.json"
$outputPath = Join-Path $captureRoot "$safeTarget.jpg"
$startedAt = Get-Date

New-Item -ItemType Directory -Force -Path $screenshotRoot, $captureRoot, $logRoot | Out-Null

function New-CaptureResult {
    param(
        [bool]$Passed,
        [string]$Reason,
        [int]$ExitCode,
        [bool]$TimedOut,
        [string]$ScreenshotPath
    )

    [ordered]@{
        passed = $Passed
        reason = $Reason
        target = $Target
        accountName = $AccountName
        exitCode = $ExitCode
        timedOut = $TimedOut
        screenshot = $ScreenshotPath
        startedAt = $startedAt.ToString('o')
        finishedAt = (Get-Date).ToString('o')
        stdout = $stdoutPath
        stderr = $stderrPath
        result = $resultPath
    }
}

try {
    if (-not (Test-Path -LiteralPath $ouroboros)) {
        throw "Ouroboros.exe was not found at $ouroboros"
    }

    if (-not (Test-Path -LiteralPath $serverConfig)) {
        throw "DbServer configuration was not found at $serverConfig"
    }

    $configText = Get-Content -Raw -LiteralPath $serverConfig
    if ($configText -notmatch '(?m)^\s*UseFakeAuth\s+1\s*$') {
        throw 'Direct-DB capture requires UseFakeAuth 1 in servers.cfg'
    }

    $requiredProcesses = @('ServerMonitor', 'DBServer', 'Launcher')
    $missing = @($requiredProcesses | Where-Object {
        -not (Get-Process -Name $_ -ErrorAction SilentlyContinue)
    })
    if ($missing.Count -gt 0) {
        throw "Required shard processes are missing: $($missing -join ', ')"
    }

    $existingClient = Get-Process -Name 'Ouroboros' -ErrorAction SilentlyContinue
    if ($existingClient) {
        throw 'An Ouroboros.exe process is already running'
    }

    $before = @{}
    Get-ChildItem -LiteralPath $screenshotRoot -Filter '*.jpg' -File -ErrorAction SilentlyContinue | ForEach-Object {
        $before[$_.FullName] = $_.LastWriteTimeUtc
    }

    $arguments = @(
        '-db', '127.0.0.1',
        '-authname', $AccountName,
        '-password', $Password,
        '-noverify',
        '-quicklogin', '1',
        '-noversioncheck',
        '-capture', $Target,
        '-fullscreen', '0',
        '-screen', $Width.ToString(), $Height.ToString(),
        '-stopinactivedisplay', '0'
    )

    $client = Start-Process -FilePath $ouroboros -WorkingDirectory $binRoot -ArgumentList $arguments `
        -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath -PassThru
    $exited = $client.WaitForExit($TimeoutSeconds * 1000)

    if (-not $exited) {
        Stop-Process -Id $client.Id -Force -ErrorAction SilentlyContinue
        $result = New-CaptureResult -Passed $false -Reason 'Ouroboros timed out before clean capture exit' `
            -ExitCode 124 -TimedOut $true -ScreenshotPath ''
    }
    else {
        $client.Refresh()
        $newScreenshots = @(Get-ChildItem -LiteralPath $screenshotRoot -Filter '*.jpg' -File -ErrorAction SilentlyContinue | Where-Object {
            $oldTime = $before[$_.FullName]
            (-not $oldTime) -or $_.LastWriteTimeUtc -gt $oldTime
        } | Sort-Object LastWriteTimeUtc -Descending)

        if ($newScreenshots.Count -eq 0) {
            $result = New-CaptureResult -Passed $false -Reason 'Ouroboros exited without producing a JPG screenshot' `
                -ExitCode $client.ExitCode -TimedOut $false -ScreenshotPath ''
        }
        else {
            Copy-Item -LiteralPath $newScreenshots[0].FullName -Destination $outputPath -Force
            $result = New-CaptureResult -Passed ($client.ExitCode -eq 0) `
                -Reason $(if ($client.ExitCode -eq 0) { 'Deterministic capture completed and Ouroboros exited cleanly' } else { 'Capture produced an image but Ouroboros returned a non-zero exit code' }) `
                -ExitCode $client.ExitCode -TimedOut $false -ScreenshotPath $outputPath
        }
    }
}
catch {
    $result = New-CaptureResult -Passed $false -Reason $_.Exception.Message -ExitCode 1 -TimedOut $false -ScreenshotPath ''
}

$result | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $resultPath -Encoding UTF8

if ($Json) {
    $result | ConvertTo-Json -Depth 5
}
elseif ($result.passed) {
    Write-Host "CAPTURE PASS - $($result.screenshot)"
    Write-Host "Result: $resultPath"
}
else {
    Write-Host "CAPTURE FAIL - $($result.reason)"
    Write-Host "Result: $resultPath"
}

if ($result.passed) { exit 0 }
exit 1
