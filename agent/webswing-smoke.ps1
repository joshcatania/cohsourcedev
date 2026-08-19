[CmdletBinding()]
param(
    [string]$DbAddress = '127.0.0.1',
    [string]$AccountName = 'Dummy00009',
    [int]$TimeoutSeconds = 180,
    [switch]$RequireClientDiagnostics,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$binDir = Join-Path $repoRoot 'bin'
$testClient = Join-Path $binDir 'TestClient.exe'
$logDir = Join-Path $PSScriptRoot 'logs'
$serverWebswingLog = Join-Path $binDir 'logs\mapserver\webswing.log'
$clientWebswingLog = Join-Path $binDir 'logs\TestClient\webswing.log'
New-Item -ItemType Directory -Force -Path $logDir | Out-Null

$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$stdoutLog = Join-Path $logDir "webswing-smoke-$stamp.out.log"
$stderrLog = Join-Path $logDir "webswing-smoke-$stamp.err.log"
$serverWebswingCapture = Join-Path $logDir "webswing-smoke-$stamp.server-webswing.log"
$clientWebswingCapture = Join-Path $logDir "webswing-smoke-$stamp.client-webswing.log"
$statusLog = Join-Path $logDir "webswing-smoke-$stamp.status"
$resultLog = Join-Path $logDir "webswing-smoke-$stamp.json"

function Finish([object]$Result, [int]$ExitCode) {
    $Result | ConvertTo-Json -Depth 8 | Set-Content -Encoding UTF8 $resultLog
    if ($Json) {
        $Result | ConvertTo-Json -Depth 8
    } else {
        Write-Host ''
        if ($Result.passed -and $Result.clientDiagnosticsAvailable) {
            Write-Host 'WEB SWING SMOKE PASS - client/server three-pose attach/swing/detach/reattach sequence verified'
        } elseif ($Result.passed) {
            Write-Host 'WEB SWING SMOKE PASS - server three-pose attach/swing/detach/reattach sequence verified; client diagnostics unavailable in TestClient'
        } else {
            Write-Host 'WEB SWING SMOKE FAIL - inspect the captured client/server diagnostics'
        }
        Write-Host ("Duration: {0:N1}s" -f $Result.durationSeconds)
        Write-Host ("TestClient exit code: {0}" -f $(if ($null -eq $Result.testClientExitCode) { '<unavailable>' } else { $Result.testClientExitCode }))
        if ($Result.reason) { Write-Host ("Reason: {0}" -f $Result.reason) }
        Write-Host ("Stdout log: {0}" -f $stdoutLog)
        Write-Host ("Stderr log: {0}" -f $stderrLog)
        Write-Host ("Server Web Swing log: {0}" -f $serverWebswingCapture)
        Write-Host ("Client Web Swing log: {0}" -f $clientWebswingCapture)
        Write-Host ("JSON result: {0}" -f $resultLog)
        if ($Result.poseAttempts) {
            Write-Host ''
            Write-Host 'Client attach-attempt diagnostics:'
            $Result.poseAttempts | ForEach-Object { Write-Host $_ }
        }
    }
    exit $ExitCode
}

if (-not (Test-Path -LiteralPath $testClient)) {
    Finish ([pscustomobject]@{
        passed = $false; reason = 'bin/TestClient.exe is missing. Run .\agent\build.ps1 first.'
        durationSeconds = 0; testClientExitCode = $null; poseAttempts = @()
    }) 1
}

$webswingLogDefinitions = @(
    [pscustomobject]@{ Source = $serverWebswingLog; Capture = $serverWebswingCapture },
    [pscustomobject]@{ Source = $clientWebswingLog; Capture = $clientWebswingCapture }
)
$webswingTextBefore = @{}
foreach ($definition in $webswingLogDefinitions) {
    $webswingTextBefore[$definition.Source] = if (Test-Path -LiteralPath $definition.Source) {
        Get-Content -Raw -LiteralPath $definition.Source
    } else {
        ''
    }
}

foreach ($name in @('ServerMonitor', 'DbServer', 'Launcher')) {
    if (-not (Get-Process -Name $name -ErrorAction SilentlyContinue)) {
        Finish ([pscustomobject]@{
            passed = $false; reason = "Required process is not running: $name. Start the fresh shard first."
            durationSeconds = 0; testClientExitCode = $null; poseAttempts = @()
        }) 1
    }
}

foreach ($name in @('Ouroboros', 'TestClient')) {
    if (Get-Process -Name $name -ErrorAction SilentlyContinue) {
        Finish ([pscustomobject]@{
            passed = $false; reason = "An old $name process is still running. Stop it before this fresh-binary test."
            durationSeconds = 0; testClientExitCode = $null; poseAttempts = @()
        }) 1
    }
}

$argsList = @(
    '-db', $DbAddress,
    '-authname', $AccountName,
    '-dontpause',
    '-selfversion',
    '-nosharedmemory',
    '-silent',
    '-webswing-smoke',
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
$started = $proc.Start()
if (-not $started) {
    Finish ([pscustomobject]@{
        passed = $false; reason = 'System.Diagnostics.Process failed to start TestClient.'
        durationSeconds = 0; testClientExitCode = $null; poseAttempts = @()
    }) 1
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

$webswingTextParts = @()
foreach ($definition in $webswingLogDefinitions) {
    if (Test-Path -LiteralPath $definition.Source) {
        Copy-Item -LiteralPath $definition.Source -Destination $definition.Capture -Force
        $webswingAllText = Get-Content -Raw -LiteralPath $definition.Source
        $webswingBefore = $webswingTextBefore[$definition.Source]
        if ($webswingBefore -and $webswingAllText.StartsWith($webswingBefore)) {
            $webswingTextParts += $webswingAllText.Substring($webswingBefore.Length)
        } else {
            $webswingTextParts += $webswingAllText
        }
    } else {
        Set-Content -Path $definition.Capture -Value '' -Encoding UTF8
    }
}
$webswingText = $webswingTextParts -join "`n"

$webswingLines = @($webswingText -split "`r?`n" | Where-Object { $_ -match 'WEB_SWING' })
$clientAttempts = @($webswingLines | Where-Object { $_ -match 'WEB_SWING CLIENT attach_attempt' })
$serverAttempts = @($webswingLines | Where-Object { $_ -match 'WEB_SWING SERVER attach_attempt' })
$selectedClientAttempts = @($clientAttempts | Where-Object { $_ -match 'selected=1' })
$selectedServerAttempts = @($serverAttempts | Where-Object { $_ -match 'selected=1' })
$clientEnabledAttempts = @($clientAttempts | Where-Object { $_ -match 'web_swing_enabled=1' -and $_ -match 'up=[^ ]*[1-9]' })
$attachLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING (CLIENT|SERVER) attach ' })
$detachLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING (CLIENT|SERVER) detach ' })
$swingLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING (CLIENT|SERVER) swing ' })
$fullSequence = $attachLines.Count -ge 2 -and $detachLines.Count -ge 1 -and $swingLines.Count -ge 1
$combinedText = (($stdoutText, $stderrText) -join "`n")
$statusWritten = Test-Path -LiteralPath $statusLog
$statusText = if ($statusWritten) { Get-Content -Raw -LiteralPath $statusLog } else { '' }
$smokeComplete = $statusText -match 'webswing_smoke_complete=1'
$clientDiagnosticsAvailable = $selectedClientAttempts.Count -ge 3 -and $clientEnabledAttempts.Count -ge 3
$serverSequencePass = $smokeComplete -and
                      ($selectedServerAttempts.Count -ge 3) -and
                      $fullSequence
$exitCodeValue = $null
if (-not $timedOut) { try { $exitCodeValue = [int]$proc.ExitCode } catch {} }

$passed = (-not $timedOut) -and ($exitCodeValue -eq 0) -and $serverSequencePass -and
          (-not $RequireClientDiagnostics -or $clientDiagnosticsAvailable)
$reason = $null
if ($timedOut) {
    $reason = "TestClient did not exit within $TimeoutSeconds seconds."
} elseif ($exitCodeValue -ne 0) {
    $reason = "TestClient exited with code $exitCodeValue."
} elseif (-not $smokeComplete) {
    $reason = 'The autonomous Web Swing driver did not write its completion marker.'
} elseif ($selectedServerAttempts.Count -lt 3) {
    $reason = "Expected three selected server anchors (server=$($selectedServerAttempts.Count))."
} elseif (-not $fullSequence) {
    $reason = "Full sequence evidence was incomplete (attach=$($attachLines.Count), swing=$($swingLines.Count), detach=$($detachLines.Count))."
} elseif ($RequireClientDiagnostics -and -not $clientDiagnosticsAvailable) {
    $reason = "Client diagnostics were required but unavailable (client=$($selectedClientAttempts.Count) selected, enabled=$($clientEnabledAttempts.Count))."
} elseif (-not $clientDiagnosticsAvailable) {
    $reason = 'Server sequence passed; client diagnostics were unavailable in TestClient and remain a manual GUI checkpoint.'
}

$result = [pscustomobject]@{
    passed = $passed
    reason = $reason
    durationSeconds = [math]::Round($sw.Elapsed.TotalSeconds, 2)
    testClientExitCode = $exitCodeValue
    timedOut = $timedOut
    accountName = $AccountName
    dbAddress = $DbAddress
    statusWritten = $statusWritten
    clientAnchorAttempts = $clientAttempts.Count
    serverAnchorAttempts = $serverAttempts.Count
    clientSelectedAnchors = $selectedClientAttempts.Count
    serverSelectedAnchors = $selectedServerAttempts.Count
    clientEnabledAttempts = $clientEnabledAttempts.Count
    clientDiagnosticsAvailable = $clientDiagnosticsAvailable
    requireClientDiagnostics = [bool]$RequireClientDiagnostics
    attachLines = $attachLines.Count
    swingLines = $swingLines.Count
    detachLines = $detachLines.Count
    fullSequence = $fullSequence
    poseAttempts = $clientAttempts
    stdoutLog = $stdoutLog
    stderrLog = $stderrLog
    serverWebswingLog = $serverWebswingCapture
    clientWebswingLog = $clientWebswingCapture
    statusLog = $statusLog
    resultLog = $resultLog
}

Finish $result $(if ($passed) { 0 } else { 1 })
