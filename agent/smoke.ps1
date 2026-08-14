[CmdletBinding()]
param(
    [string]$AuthAddress = '127.0.0.1',
    [int]$AuthPort = 2106,
    [string]$AuthName = 'Dummy00001',
    [string]$Password = '11111111',
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
        Write-Host ("Auth TCP port open: {0}" -f $Result.authPortOpen)
        Write-Host ("TestClient exit code: {0}" -f $(if ($null -eq $Result.testClientExitCode) { '<unavailable>' } else { $Result.testClientExitCode }))
        if ($Result.reason) { Write-Host ("Reason: {0}" -f $Result.reason) }
        Write-Host ("Stdout log: {0}" -f $stdoutLog)
        Write-Host ("Stderr log: {0}" -f $stderrLog)
        Write-Host ("JSON result: {0}" -f $resultLog)
        if ($Result.tail -and $Result.tail.Count -gt 0) {
            Write-Host ''
            Write-Host 'Useful TestClient output:'
            $Result.tail | ForEach-Object { Write-Host $_ }
        } elseif (-not $Result.passed) {
            Write-Host ''
            Write-Host 'No TestClient console output was captured. This is itself diagnostic; inspect bin\logs\TestClient if present.'
        }
    }
    exit $ExitCode
}

function Test-TcpPort([string]$HostName, [int]$Port, [int]$TimeoutMs = 3000) {
    $client = New-Object System.Net.Sockets.TcpClient
    try {
        $async = $client.BeginConnect($HostName, $Port, $null, $null)
        if (-not $async.AsyncWaitHandle.WaitOne($TimeoutMs, $false)) { return $false }
        $client.EndConnect($async)
        return $true
    } catch {
        return $false
    } finally {
        $client.Close()
    }
}

if (-not (Test-Path $testClient)) {
    $r = [pscustomobject]@{
        passed = $false; stage = 'preflight'; reason = 'bin/TestClient.exe is missing. Run .\agent\build.ps1 first.'
        durationSeconds = 0; testClientExitCode = $null; authAddress = $AuthAddress; authPort = $AuthPort
        authPortOpen = $false; tail = @()
    }
    Finish $r 1
}

$required = @('ServerMonitor','AuthServer','DbServer','Launcher')
$missing = @()
foreach ($name in $required) {
    if (-not (Get-Process -Name $name -ErrorAction SilentlyContinue)) { $missing += $name }
}
if ($missing.Count -gt 0) {
    $r = [pscustomobject]@{
        passed = $false; stage = 'preflight'; reason = "Required process(es) not running: $($missing -join ', ')."
        durationSeconds = 0; testClientExitCode = $null; authAddress = $AuthAddress; authPort = $AuthPort
        authPortOpen = $false; tail = @()
    }
    Finish $r 1
}

$authPortOpen = Test-TcpPort $AuthAddress $AuthPort
if (-not $authPortOpen) {
    $r = [pscustomobject]@{
        passed = $false; stage = 'auth-listener'; reason = "AuthServer process exists but TCP $AuthAddress`:$AuthPort did not accept a connection."
        durationSeconds = 0; testClientExitCode = $null; authAddress = $AuthAddress; authPort = $AuthPort
        authPortOpen = $false; tail = @()
    }
    Finish $r 1
}

# TestClient's source defines Dummy00001 / 11111111 as its default generated
# credentials, but that does NOT prove the account exists in the local cohauth DB.
# Using them explicitly makes this test deterministic and lets a login error tell
# us whether account bootstrapping is the next missing layer.
$argsList = @(
    '-auth', $AuthAddress,
    '-authname', $AuthName,
    '-password', $Password,
    '-justlogin',
    '-dontpause',
    '-silent'
)

if (-not $Json) {
    Write-Host 'COH LOGIN SMOKE TEST'
    Write-Host ("Auth address: {0}:{1}" -f $AuthAddress, $AuthPort)
    Write-Host ("Account: {0}" -f $AuthName)
    Write-Host ("Timeout: {0}s" -f $TimeoutSeconds)
    Write-Host ''
    Write-Host 'Auth TCP listener... PASS'
    Write-Host 'Launching TestClient...'
}

# Use System.Diagnostics.Process directly instead of Start-Process. On Windows
# PowerShell, Start-Process + redirected output can leave ExitCode unavailable
# even after the child exits, which made the first smoke result ambiguous.
$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $testClient
$psi.WorkingDirectory = $binDir
$psi.UseShellExecute = $false
$psi.CreateNoWindow = $true
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError = $true
$psi.Arguments = (($argsList | ForEach-Object {
    if ($_ -match '[\s"]') { '"' + ($_ -replace '"','\"') + '"' } else { $_ }
}) -join ' ')

$proc = New-Object System.Diagnostics.Process
$proc.StartInfo = $psi
$sw = [System.Diagnostics.Stopwatch]::StartNew()
$started = $proc.Start()
if (-not $started) {
    $r = [pscustomobject]@{
        passed = $false; stage = 'launch'; reason = 'System.Diagnostics.Process failed to start TestClient.'
        durationSeconds = 0; testClientExitCode = $null; authAddress = $AuthAddress; authPort = $AuthPort
        authPortOpen = $authPortOpen; tail = @()
    }
    Finish $r 1
}

$stdoutTask = $proc.StandardOutput.ReadToEndAsync()
$stderrTask = $proc.StandardError.ReadToEndAsync()
$exited = $proc.WaitForExit($TimeoutSeconds * 1000)
$timedOut = -not $exited
if ($timedOut) {
    try { $proc.Kill() } catch {}
    $proc.WaitForExit()
}
$proc.Refresh()
$sw.Stop()

$stdoutText = $stdoutTask.Result
$stderrText = $stderrTask.Result
Set-Content -Path $stdoutLog -Value $stdoutText -Encoding UTF8
Set-Content -Path $stderrLog -Value $stderrText -Encoding UTF8

$combinedText = (($stdoutText, $stderrText) -join "`n")
$combinedLines = @($combinedText -split "`r?`n" | Where-Object { $_ -and $_.Trim().Length -gt 0 })
$tail = @($combinedLines | Select-Object -Last 35)

$exitCode = $null
if (-not $timedOut) {
    try { $exitCode = [int]$proc.ExitCode } catch { $exitCode = $null }
}

$hasLoginError = $combinedText -match '(?im)Error logging on|AuthServer Error:|DBServer Error:'
$hasFatal = $combinedText -match '(?im)Fatal Error:|CRT Error:|Exception caught:'
$hasServerSelection = $combinedText -match '(?im)Using server\s+\d+'
$hasConnectMarker = $combinedText -match '(?im)Connecting as\s+'

$passed = (-not $timedOut) -and ($exitCode -eq 0) -and (-not $hasLoginError) -and (-not $hasFatal) -and $hasServerSelection
$reason = $null
$stage = 'auth-db-login'
if ($timedOut) {
    $reason = "TestClient did not exit within $TimeoutSeconds seconds."
} elseif ($hasFatal) {
    $reason = 'TestClient reported a fatal/CRT/exception condition.'
} elseif ($hasLoginError) {
    $reason = 'TestClient reported an AuthServer or DbServer login error. The useful output below should identify which one.'
} elseif ($null -eq $exitCode) {
    $reason = 'TestClient exited, but Windows did not expose an exit code. Inspect captured output.'
} elseif ($exitCode -ne 0) {
    $reason = "TestClient exited with code $exitCode."
} elseif (-not $hasServerSelection) {
    $reason = 'TestClient exited without the expected server-selection marker; login/DB success is not proven.'
}

$result = [pscustomobject]@{
    passed = $passed
    stage = $stage
    reason = $reason
    durationSeconds = [math]::Round($sw.Elapsed.TotalSeconds, 2)
    testClientExitCode = $exitCode
    timedOut = $timedOut
    authAddress = $AuthAddress
    authPort = $AuthPort
    authPortOpen = $authPortOpen
    authName = $AuthName
    sawConnectMarker = $hasConnectMarker
    sawServerSelection = $hasServerSelection
    sawLoginError = $hasLoginError
    sawFatalError = $hasFatal
    stdoutLog = $stdoutLog
    stderrLog = $stderrLog
    resultLog = $resultLog
    tail = $tail
}

Finish $result $(if ($passed) { 0 } else { 1 })
