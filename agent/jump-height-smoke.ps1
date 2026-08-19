[CmdletBinding()]
param(
    [string]$DbAddress = '127.0.0.1',
    [string]$AccountName = 'Dummy00010',
    [int]$TimeoutSeconds = 180,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$binDir = Join-Path $repoRoot 'bin'
$testClient = Join-Path $binDir 'TestClient.exe'
$logDir = Join-Path $PSScriptRoot 'logs'
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$stdoutLog = Join-Path $logDir "jump-height-smoke-$stamp.out.log"
$stderrLog = Join-Path $logDir "jump-height-smoke-$stamp.err.log"
$statusLog = Join-Path $logDir "jump-height-smoke-$stamp.status"
$resultLog = Join-Path $logDir "jump-height-smoke-$stamp.json"

function Read-Status([string]$Path) {
    $status = @{}
    if (Test-Path -LiteralPath $Path) {
        foreach ($line in (Get-Content -LiteralPath $Path)) {
            $parts = $line -split '=', 2
            if ($parts.Count -eq 2) { $status[$parts[0]] = $parts[1] }
        }
    }
    return $status
}

function Finish($result, [int]$exitCode) {
    $result | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $resultLog -Encoding UTF8
    if ($Json) {
        $result | ConvertTo-Json -Depth 6
    } else {
        if ($result.passed) {
            Write-Host "JUMP HEIGHT SMOKE PASS - OFF=$($result.offHeight) ON=$($result.onHeight)"
        } else {
            Write-Host "JUMP HEIGHT SMOKE FAIL - $($result.reason)"
        }
        Write-Host "Stdout log: $stdoutLog"
        Write-Host "Stderr log: $stderrLog"
        Write-Host "Status: $statusLog"
        Write-Host "JSON result: $resultLog"
    }
    exit $exitCode
}

if (-not (Test-Path -LiteralPath $testClient)) {
    Finish ([pscustomobject]@{ passed = $false; reason = 'bin/TestClient.exe is missing.'; offHeight = 0; onHeight = 0 }) 1
}
foreach ($name in @('ServerMonitor', 'DbServer', 'Launcher')) {
    if (-not (Get-Process -Name $name -ErrorAction SilentlyContinue)) {
        Finish ([pscustomobject]@{ passed = $false; reason = "Required process is not running: $name"; offHeight = 0; onHeight = 0 }) 1
    }
}
foreach ($name in @('Ouroboros', 'TestClient')) {
    if (Get-Process -Name $name -ErrorAction SilentlyContinue) {
        Finish ([pscustomobject]@{ passed = $false; reason = "An old $name process is still running."; offHeight = 0; onHeight = 0 }) 1
    }
}

$argsList = @(
    '-db', $DbAddress,
    '-authname', $AccountName,
    '-dontpause',
    '-selfversion',
    '-nosharedmemory',
    '-silent',
    '-webswing-jump-smoke',
    '-agent-status', $statusLog
)
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
if (-not $proc.Start()) { Finish ([pscustomobject]@{ passed = $false; reason = 'Could not start TestClient.'; offHeight = 0; onHeight = 0 }) 1 }
$stdoutTask = $proc.StandardOutput.ReadToEndAsync()
$stderrTask = $proc.StandardError.ReadToEndAsync()
$exited = $proc.WaitForExit([math]::Max(1, $TimeoutSeconds) * 1000)
$timedOut = -not $exited
if ($timedOut) {
    try { $proc.Kill() } catch {}
    $proc.WaitForExit()
}
$stdoutText = $stdoutTask.Result
$stderrText = $stderrTask.Result
Set-Content -LiteralPath $stdoutLog -Value $stdoutText -Encoding UTF8
Set-Content -LiteralPath $stderrLog -Value $stderrText -Encoding UTF8
$sw.Stop()

$status = Read-Status $statusLog
$offHeight = 0.0
$onHeight = 0.0
[double]::TryParse($status['webswing_jump_off_height'], [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref]$offHeight) | Out-Null
[double]::TryParse($status['webswing_jump_on_height'], [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref]$onHeight) | Out-Null
$passed = (-not $timedOut) -and $proc.ExitCode -eq 0 -and
          $status['webswing_jump_smoke_complete'] -eq '1' -and
          $offHeight -gt 0 -and $onHeight -gt ($offHeight * 1.4)
$reason = if ($timedOut) { "TestClient timed out after $TimeoutSeconds seconds." }
          elseif ($proc.ExitCode -ne 0) { "TestClient exited with code $($proc.ExitCode)." }
          elseif ($status['webswing_jump_smoke_complete'] -ne '1') { 'Jump smoke did not write its completion marker.' }
          elseif ($offHeight -le 0 -or $onHeight -le ($offHeight * 1.4)) { "Expected ON height to exceed OFF by at least 40% (OFF=$offHeight, ON=$onHeight)." }
          else { $null }

Finish ([pscustomobject]@{
    passed = $passed
    reason = $reason
    durationSeconds = [math]::Round($sw.Elapsed.TotalSeconds, 2)
    testClientExitCode = if ($timedOut) { $null } else { $proc.ExitCode }
    timedOut = $timedOut
    accountName = $AccountName
    dbAddress = $DbAddress
    offHeight = [math]::Round($offHeight, 3)
    onHeight = [math]::Round($onHeight, 3)
    ratio = if ($offHeight -gt 0) { [math]::Round($onHeight / $offHeight, 3) } else { 0 }
    stdoutLog = $stdoutLog
    stderrLog = $stderrLog
    statusLog = $statusLog
    resultLog = $resultLog
}) $(if ($passed) { 0 } else { 1 })
