[CmdletBinding()]
param(
    [string]$AuthAddress = '127.0.0.1',
    [string]$AuthName = '',
    [string]$Password = '',
    [int]$TimeoutSeconds = 90,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$binDir = Join-Path $repoRoot 'bin'
$testClient = Join-Path $binDir 'TestClient.exe'
$logDir = Join-Path $PSScriptRoot 'logs'
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$stdoutLog = Join-Path $logDir "smoke-login-$stamp.out.log"
$stderrLog = Join-Path $logDir "smoke-login-$stamp.err.log"
$resultLog = Join-Path $logDir "smoke-login-$stamp.json"

function Finish([object]$Result, [int]$ExitCode) {
    $Result | ConvertTo-Json -Depth 6 | Set-Content -Encoding UTF8 $resultLog
    if ($Json) {
        $Result | ConvertTo-Json -Depth 6
    } else {
        Write-Host ''
        if ($Result.passed) {
            Write-Host 'SMOKE PASS - AuthServer + DbServer login path verified'
        } else {
            Write-Host 'SMOKE FAIL - login/DB path not verified'
        }
        Write-Host ("Stage: {0}" -f $Result.stage)
        Write-Host ("Duration: {0:N1}s" -f $Result.durationSeconds)
        Write-Host ("TestClient exit code: {0}" -f $Result.testClientExitCode)
        if ($Result.reason) { Write-Host ("Reason: {0}" -f $Result.reason) }
        Write-Host ("Stdout log: {0}" -f $stdoutLog)
        Write-Host ("Stderr log: {0}" -f $stderrLog)
        Write-Host ("JSON result: {0}" -f $resultLog)
        if ($Result.tail -and $Result.tail.Count -gt 0) {
            Write-Host ''
            Write-Host 'Useful TestClient output:'
            $Result.tail | ForEach-Object { Write-Host $_ }
        }
    }
    exit $ExitCode
}

if (-not (Test-Path $testClient)) {
    $r = [pscustomobject]@{
        passed = $false
        stage = 'preflight'
        reason = 'bin/TestClient.exe is missing. Run .\agent\build.ps1 first.'
        durationSeconds = 0
        testClientExitCode = $null
        authAddress = $AuthAddress
        tail = @()
    }
    Finish $r 1
}

$required = @('ServerMonitor','AuthServer','DbServer')
$missing = @()
foreach ($name in $required) {
    if (-not (Get-Process -Name $name -ErrorAction SilentlyContinue)) { $missing += $name }
}
if ($missing.Count -gt 0) {
    $r = [pscustomobject]@{
        passed = $false
        stage = 'preflight'
        reason = "Required process(es) not running: $($missing -join ', '). Run .\agent\start-shard.ps1 and inspect .\agent\status.ps1."
        durationSeconds = 0
        testClientExitCode = $null
        authAddress = $AuthAddress
        tail = @()
    }
    Finish $r 1
}

# -justlogin is an existing TestClient mode that authenticates, connects to the
# selected DbServer, then returns without creating/resuming a character.
# -dontpause makes failures automation-safe instead of waiting for a keypress.
$argsList = @('-auth', $AuthAddress, '-justlogin', '-dontpause')
if ($AuthName) { $argsList += @('-authname', $AuthName) }
if ($Password) { $argsList += @('-password', $Password) }

if (-not $Json) {
    Write-Host 'COH LOGIN SMOKE TEST'
    Write-Host ("Auth address: {0}" -f $AuthAddress)
    if ($AuthName) {
        Write-Host ("Account: {0}" -f $AuthName)
    } else {
        Write-Host 'Account: TestClient built-in default (Dummy00001 / built-in test password)'
    }
    Write-Host ("Timeout: {0}s" -f $TimeoutSeconds)
    Write-Host ''
    Write-Host 'Launching TestClient...'
}

$sw = [System.Diagnostics.Stopwatch]::StartNew()
$proc = Start-Process -FilePath $testClient `
    -ArgumentList $argsList `
    -WorkingDirectory $binDir `
    -RedirectStandardOutput $stdoutLog `
    -RedirectStandardError $stderrLog `
    -PassThru

$timedOut = -not $proc.WaitForExit($TimeoutSeconds * 1000)
if ($timedOut) {
    try { Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue } catch {}
    $proc.WaitForExit()
}
$sw.Stop()

$stdout = if (Test-Path $stdoutLog) { @(Get-Content $stdoutLog -ErrorAction SilentlyContinue) } else { @() }
$stderr = if (Test-Path $stderrLog) { @(Get-Content $stderrLog -ErrorAction SilentlyContinue) } else { @() }
$combined = @($stdout + $stderr)
$text = $combined -join "`n"
$tail = @($combined | Select-Object -Last 25)

$hasLoginError = $text -match '(?im)Error logging on|AuthServer Error:|DBServer Error:'
$hasFatal = $text -match '(?im)Fatal Error:|CRT Error:|Exception caught:'
$hasServerSelection = $text -match '(?im)Using server\s+\d+'
$exitCode = if ($timedOut) { $null } else { $proc.ExitCode }

$passed = (-not $timedOut) -and ($exitCode -eq 0) -and (-not $hasLoginError) -and (-not $hasFatal)
$reason = $null
$stage = 'auth-db-login'
if ($timedOut) {
    $reason = "TestClient did not exit within $TimeoutSeconds seconds."
} elseif ($hasFatal) {
    $reason = 'TestClient reported a fatal/CRT/exception condition.'
} elseif ($hasLoginError) {
    $reason = 'TestClient reported an AuthServer or DbServer login error.'
} elseif ($exitCode -ne 0) {
    $reason = "TestClient exited with code $exitCode."
} elseif (-not $hasServerSelection) {
    # Exit 0 is still meaningful, but flag the missing expected marker so we
    # do not overstate what was observed from stdout.
    $reason = 'TestClient exited successfully, but the expected server-selection marker was not captured. Inspect the log before treating this as a strong pass.'
    $passed = $false
}

$result = [pscustomobject]@{
    passed = $passed
    stage = $stage
    reason = $reason
    durationSeconds = [math]::Round($sw.Elapsed.TotalSeconds, 2)
    testClientExitCode = $exitCode
    timedOut = $timedOut
    authAddress = $AuthAddress
    usedExplicitAccount = [bool]$AuthName
    sawServerSelection = $hasServerSelection
    sawLoginError = $hasLoginError
    sawFatalError = $hasFatal
    stdoutLog = $stdoutLog
    stderrLog = $stderrLog
    resultLog = $resultLog
    tail = $tail
}

Finish $result $(if ($passed) { 0 } else { 1 })
