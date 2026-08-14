[CmdletBinding()]
param(
    [string]$DbAddress = '127.0.0.1',
    [string]$AccountName = 'Dummy00001',
    [int]$TimeoutSeconds = 90,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$binDir = Join-Path $repoRoot 'bin'
$testClient = Join-Path $binDir 'TestClient.exe'
$serverCfg = Join-Path $binDir 'data\server\db\servers.cfg'
$logDir = Join-Path $PSScriptRoot 'logs'
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$stdoutLog = Join-Path $logDir "smoke-directdb-$stamp.out.log"
$stderrLog = Join-Path $logDir "smoke-directdb-$stamp.err.log"
$resultLog = Join-Path $logDir "smoke-directdb-$stamp.json"

function Finish([object]$Result, [int]$ExitCode) {
    $Result | ConvertTo-Json -Depth 6 | Set-Content -Encoding UTF8 $resultLog
    if ($Json) {
        $Result | ConvertTo-Json -Depth 6
    } else {
        Write-Host ''
        if ($Result.passed) {
            Write-Host 'SMOKE PASS - direct DbServer login path verified'
        } else {
            Write-Host 'SMOKE FAIL - direct DbServer login path not verified'
        }
        Write-Host ("Stage: {0}" -f $Result.stage)
        Write-Host ("Duration: {0:N1}s" -f $Result.durationSeconds)
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
            Write-Host 'No TestClient console output was captured.'
        }
    }
    exit $ExitCode
}

if (-not (Test-Path $testClient)) {
    $r = [pscustomobject]@{
        passed = $false; stage = 'preflight'; reason = 'bin/TestClient.exe is missing. Run .\agent\build.ps1 first.'
        durationSeconds = 0; testClientExitCode = $null; dbAddress = $DbAddress; tail = @()
    }
    Finish $r 1
}

if (-not (Test-Path $serverCfg)) {
    $r = [pscustomobject]@{
        passed = $false; stage = 'preflight'; reason = "Server config is missing: $serverCfg"
        durationSeconds = 0; testClientExitCode = $null; dbAddress = $DbAddress; tail = @()
    }
    Finish $r 1
}

# OuroDev's local development workflow bypasses AuthServer and connects clients
# directly to DbServer with -db 127.0.0.1. The server side must also be in its
# built-in fake-auth mode. servercfg.c explicitly requires exactly one of
# AuthServer or UseFakeAuth, so detect a production/auth-server configuration
# here instead of allowing a later TestClient login failure to obscure it.
$cfgText = Get-Content -Raw -Path $serverCfg
$fakeAuthEnabled = [bool]($cfgText -match '(?im)^\s*UseFakeAuth\s+1\s*(?:#.*)?$')
$activeAuthServer = [bool]($cfgText -match '(?im)^\s*AuthServer\s+\S+')
if (-not $fakeAuthEnabled -or $activeAuthServer) {
    $state = "UseFakeAuth enabled=$fakeAuthEnabled; active AuthServer directive=$activeAuthServer"
    $r = [pscustomobject]@{
        passed = $false
        stage = 'fake-auth-config'
        reason = "Direct-DB development requires DbServer fake-auth mode, but servers.cfg is not configured for it ($state). Do not hand-edit it yet; this diagnostic intentionally stops before TestClient so the development-mode switch can be made explicitly and reversibly."
        durationSeconds = 0
        testClientExitCode = $null
        dbAddress = $DbAddress
        fakeAuthEnabled = $fakeAuthEnabled
        activeAuthServer = $activeAuthServer
        tail = @()
    }
    Finish $r 1
}

$required = @('ServerMonitor','DbServer','Launcher')
$missing = @()
foreach ($name in $required) {
    if (-not (Get-Process -Name $name -ErrorAction SilentlyContinue)) { $missing += $name }
}
if ($missing.Count -gt 0) {
    $r = [pscustomobject]@{
        passed = $false; stage = 'preflight'; reason = "Required process(es) not running: $($missing -join ', ')."
        durationSeconds = 0; testClientExitCode = $null; dbAddress = $DbAddress; tail = @()
    }
    Finish $r 1
}

# -justlogin intentionally tests only the direct DbServer connection and exits.
# It does not request a character or MapServer yet. -dontpause prevents legacy
# error paths from waiting for keyboard input. -selfversion avoids depending on
# TestClientLauncher for a version handshake. No password is required by the
# direct local development path.
$argsList = @(
    '-db', $DbAddress,
    '-authname', $AccountName,
    '-justlogin',
    '-dontpause',
    '-selfversion',
    '-silent'
)

if (-not $Json) {
    Write-Host 'COH DIRECT-DB SMOKE TEST'
    Write-Host ("DbServer address: {0}" -f $DbAddress)
    Write-Host ("Development account label: {0}" -f $AccountName)
    Write-Host ("Timeout: {0}s" -f $TimeoutSeconds)
    Write-Host ''
    Write-Host 'AuthServer: BYPASSED (intentional local-dev path)'
    Write-Host 'DbServer fake auth: ENABLED'
    Write-Host 'Launching TestClient...'
}

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
        durationSeconds = 0; testClientExitCode = $null; dbAddress = $DbAddress; tail = @()
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
$tail = @($combinedLines | Select-Object -Last 45)

$exitCode = $null
if (-not $timedOut) {
    try { $exitCode = [int]$proc.ExitCode } catch { $exitCode = $null }
}

$hasLoginError = $combinedText -match '(?im)Error logging on|DBServer Error:|Error connecting|Failed to connect'
$hasFatal = $combinedText -match '(?im)Fatal Error:|CRT Error:|Exception caught:'
$hasConnectMarker = $combinedText -match '(?im)Connecting as\s+'
$hasDirectDbArg = $combinedText -match '(?im)cmdline args:.*-db\s+'

$passed = (-not $timedOut) -and ($exitCode -eq 0) -and (-not $hasLoginError) -and (-not $hasFatal) -and $hasConnectMarker
$reason = $null
$stage = 'direct-db-login'
if ($timedOut) {
    $reason = "TestClient did not exit within $TimeoutSeconds seconds."
} elseif ($hasFatal) {
    $reason = 'TestClient reported a fatal/CRT/exception condition.'
} elseif ($hasLoginError) {
    $reason = 'TestClient reported a DbServer connection/login error. See useful output below.'
} elseif ($null -eq $exitCode) {
    $reason = 'TestClient exited, but Windows did not expose an exit code. Inspect captured output.'
} elseif ($exitCode -ne 0) {
    $reason = "TestClient exited with code $exitCode. See useful output below."
} elseif (-not $hasConnectMarker) {
    $reason = 'TestClient exited without the expected connection marker; direct-DB success is not proven.'
}

$result = [pscustomobject]@{
    passed = $passed
    stage = $stage
    reason = $reason
    durationSeconds = [math]::Round($sw.Elapsed.TotalSeconds, 2)
    testClientExitCode = $exitCode
    timedOut = $timedOut
    dbAddress = $DbAddress
    accountName = $AccountName
    authBypassed = $true
    fakeAuthEnabled = $fakeAuthEnabled
    sawConnectMarker = $hasConnectMarker
    sawDirectDbArgument = $hasDirectDbArg
    sawLoginError = $hasLoginError
    sawFatalError = $hasFatal
    stdoutLog = $stdoutLog
    stderrLog = $stderrLog
    resultLog = $resultLog
    tail = $tail
}

Finish $result $(if ($passed) { 0 } else { 1 })
