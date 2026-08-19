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
$constraintSummaryLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING (CLIENT|SERVER) constraint_summary ' })
$constraintFrameLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING (CLIENT|SERVER) constraint ' -and $_ -notmatch 'constraint_summary' })
$tetherRenderLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING CLIENT tether_render ' })
$constraintSummaries = @()
$softCorrectionCount = 0
$hardCorrectionCount = 0
$maxRadialCorrection = 0.0
$maxVelocityDirectionDelta = 0.0
foreach ($line in $constraintSummaryLines) {
    $summaryMatch = [regex]::Match($line, 'samples=(\d+) soft_corrections=(\d+) radial_corrections=(\d+) hard_corrections=(\d+) max_error=([-+0-9.eE]+) avg_error=([-+0-9.eE]+) max_radial_correction=([-+0-9.eE]+) avg_radial_correction=([-+0-9.eE]+) max_velocity_dir_delta=([-+0-9.eE]+)')
    if (-not $summaryMatch.Success) { continue }
    $softCorrections = [int]$summaryMatch.Groups[2].Value
    $radialCorrections = [int]$summaryMatch.Groups[3].Value
    $hardCorrections = [int]$summaryMatch.Groups[4].Value
    $maxRadialCorrection = [math]::Max($maxRadialCorrection, [double]::Parse($summaryMatch.Groups[7].Value, [Globalization.CultureInfo]::InvariantCulture))
    $maxVelocityDirectionDelta = [math]::Max($maxVelocityDirectionDelta, [double]::Parse($summaryMatch.Groups[9].Value, [Globalization.CultureInfo]::InvariantCulture))
    $softCorrectionCount += $softCorrections
    $hardCorrectionCount += $hardCorrections
    $constraintSummaries += [pscustomobject]@{
        samples = [int]$summaryMatch.Groups[1].Value
        softCorrections = $softCorrections
        radialCorrections = $radialCorrections
        hardCorrections = $hardCorrections
        maxError = [double]::Parse($summaryMatch.Groups[5].Value, [Globalization.CultureInfo]::InvariantCulture)
        avgError = [double]::Parse($summaryMatch.Groups[6].Value, [Globalization.CultureInfo]::InvariantCulture)
        maxRadialCorrection = [double]::Parse($summaryMatch.Groups[7].Value, [Globalization.CultureInfo]::InvariantCulture)
        avgRadialCorrection = [double]::Parse($summaryMatch.Groups[8].Value, [Globalization.CultureInfo]::InvariantCulture)
        maxVelocityDirectionDelta = [double]::Parse($summaryMatch.Groups[9].Value, [Globalization.CultureInfo]::InvariantCulture)
    }
}
$constraintDiagnosticsPass = $constraintSummaries.Count -ge 5 -and $hardCorrectionCount -eq 0
$anchorFanEvidencePass = $selectedServerAttempts.Count -ge 5 -and @($selectedServerAttempts | Where-Object {
    $match = [regex]::Match($_, 'probes=(\d+)')
    $match.Success -and [int]$match.Groups[1].Value -ge 15
}).Count -ge 5
$detachSpeeds = @()
foreach ($line in $detachLines) {
    $detachMatch = [regex]::Match($line, 'detach speed=([-+0-9.eE]+)')
    if ($detachMatch.Success) {
        $detachSpeeds += [double]::Parse($detachMatch.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture)
    }
}
$maxDetachSpeed = if ($detachSpeeds.Count -gt 0) { ($detachSpeeds | Measure-Object -Maximum).Maximum } else { 0.0 }
$retainedMomentumDetachPass = $detachSpeeds.Count -ge 1 -and $maxDetachSpeed -gt 0.25
$steeringLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING SERVER steering ' })
$steeringEvidence = @{}
foreach ($line in $steeringLines) {
    $match = [regex]::Match($line, 'forward=\(([-+0-9.eE]+) ([-+0-9.eE]+) ([-+0-9.eE]+)\) right=\(([-+0-9.eE]+) ([-+0-9.eE]+) ([-+0-9.eE]+)\) input_world=\(([-+0-9.eE]+) ([-+0-9.eE]+) ([-+0-9.eE]+)\)')
    if (-not $match.Success) { continue }

    $fx = [double]::Parse($match.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture)
    $fz = [double]::Parse($match.Groups[3].Value, [Globalization.CultureInfo]::InvariantCulture)
    $rx = [double]::Parse($match.Groups[4].Value, [Globalization.CultureInfo]::InvariantCulture)
    $rz = [double]::Parse($match.Groups[6].Value, [Globalization.CultureInfo]::InvariantCulture)
    $ix = [double]::Parse($match.Groups[7].Value, [Globalization.CultureInfo]::InvariantCulture)
    $iz = [double]::Parse($match.Groups[9].Value, [Globalization.CultureInfo]::InvariantCulture)
    $inputLength = [math]::Sqrt($ix * $ix + $iz * $iz)
    if ($inputLength -le 0.01) { continue }

    $forwardDot = ($ix * $fx + $iz * $fz) / $inputLength
    $rightDot = ($ix * $rx + $iz * $rz) / $inputLength
    $yawBucket = if ([math]::Abs($fz) -gt 0.8 -and [math]::Abs($fx) -lt 0.2) {
        'yaw0'
    } elseif ([math]::Abs($fx) -gt 0.8 -and [math]::Abs($fz) -lt 0.2) {
        'yaw90'
    } else {
        $null
    }
    if (-not $yawBucket) { continue }

    $direction = if ($forwardDot -gt 0.75) { 'forward' }
                 elseif ($rightDot -lt -0.75) { 'left' }
                 elseif ($rightDot -gt 0.75) { 'right' }
                 else { $null }
    if ($direction) {
        $key = "$yawBucket/$direction"
        $steeringEvidence[$key] = 1
    }
}
$requiredSteeringEvidence = @('yaw0/forward', 'yaw0/left', 'yaw0/right', 'yaw90/forward', 'yaw90/left', 'yaw90/right')
$missingSteeringEvidence = @($requiredSteeringEvidence | Where-Object { -not $steeringEvidence.ContainsKey($_) })
$steeringEvidencePass = $missingSteeringEvidence.Count -eq 0
$fullSequence = $attachLines.Count -ge 2 -and $detachLines.Count -ge 1 -and $swingLines.Count -ge 1
$combinedText = (($stdoutText, $stderrText) -join "`n")
$statusWritten = Test-Path -LiteralPath $statusLog
$statusText = if ($statusWritten) { Get-Content -Raw -LiteralPath $statusLog } else { '' }
$smokeComplete = $statusText -match 'webswing_smoke_complete=1'
$clientDiagnosticsAvailable = $selectedClientAttempts.Count -ge 3 -and $clientEnabledAttempts.Count -ge 3
$serverSequencePass = $smokeComplete -and
                      ($selectedServerAttempts.Count -ge 5) -and
                      $fullSequence -and
                      $retainedMomentumDetachPass -and
                      $constraintDiagnosticsPass -and
                      $anchorFanEvidencePass
$exitCodeValue = $null
if (-not $timedOut) { try { $exitCodeValue = [int]$proc.ExitCode } catch {} }

$passed = (-not $timedOut) -and ($exitCodeValue -eq 0) -and $serverSequencePass -and
          $steeringEvidencePass -and
          (-not $RequireClientDiagnostics -or $clientDiagnosticsAvailable)
$reason = $null
if ($timedOut) {
    $reason = "TestClient did not exit within $TimeoutSeconds seconds."
} elseif ($exitCodeValue -ne 0) {
    $reason = "TestClient exited with code $exitCodeValue."
} elseif (-not $smokeComplete) {
    $reason = 'The autonomous Web Swing driver did not write its completion marker.'
} elseif ($selectedServerAttempts.Count -lt 5) {
    $reason = "Expected five selected server anchors across the awkward-pose matrix (server=$($selectedServerAttempts.Count))."
} elseif (-not $fullSequence) {
    $reason = "Full sequence evidence was incomplete (attach=$($attachLines.Count), swing=$($swingLines.Count), detach=$($detachLines.Count))."
} elseif (-not $retainedMomentumDetachPass) {
    $reason = "Space-release momentum evidence was incomplete; expected a non-trivial detach speed (max=$([math]::Round($maxDetachSpeed, 3)))."
} elseif (-not $steeringEvidencePass) {
    $reason = "World-space W/A/D steering evidence was incomplete; missing: $($missingSteeringEvidence -join ', ')."
} elseif (-not $constraintDiagnosticsPass) {
    $reason = "Soft constraint diagnostics were incomplete or reported hard corrections (summaries=$($constraintSummaries.Count), hard=$hardCorrectionCount)."
} elseif (-not $anchorFanEvidencePass) {
    $reason = "Broad anchor fan evidence was incomplete; expected five selected attempts with probes>=15."
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
    detachSpeeds = @($detachSpeeds | ForEach-Object { [math]::Round($_, 3) })
    maxDetachSpeed = [math]::Round($maxDetachSpeed, 3)
    retainedMomentumDetachPass = $retainedMomentumDetachPass
    fullSequence = $fullSequence
    steeringLines = $steeringLines.Count
    steeringEvidencePass = $steeringEvidencePass
    steeringEvidence = @($steeringEvidence.Keys | Sort-Object)
    missingSteeringEvidence = $missingSteeringEvidence
    constraintDiagnosticsPass = $constraintDiagnosticsPass
    constraintSummaryCount = $constraintSummaries.Count
    constraintSummaries = $constraintSummaries
    softCorrectionCount = $softCorrectionCount
    hardCorrectionCount = $hardCorrectionCount
    maxRadialCorrection = [math]::Round($maxRadialCorrection, 4)
    maxVelocityDirectionDelta = [math]::Round($maxVelocityDirectionDelta, 4)
    anchorFanEvidencePass = $anchorFanEvidencePass
    tetherRenderEvidenceAvailable = $tetherRenderLines.Count -gt 0
    tetherRenderLines = $tetherRenderLines.Count
    poseAttempts = $clientAttempts
    stdoutLog = $stdoutLog
    stderrLog = $stderrLog
    serverWebswingLog = $serverWebswingCapture
    clientWebswingLog = $clientWebswingCapture
    statusLog = $statusLog
    resultLog = $resultLog
}

Finish $result $(if ($passed) { 0 } else { 1 })
