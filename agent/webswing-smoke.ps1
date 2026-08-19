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
$directionDeltaThreshold = 0.30
$radialVelocityThreshold = 0.25
$maxAllowedConsecutiveDirectionDeltas = 3
$maxAllowedDirectionDeltaPercent = 12.5

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
$totalConstraintSamples = 0
$velocityDirectionDeltaSum = 0.0
$velocityDirectionDeltaLargeCount = 0
$maxConsecutiveVelocityDirectionDelta = 0
$radialVelocityRemovedCount = 0
$radialVelocityRemovedSum = 0.0
$maxRadialVelocityRemoved = 0.0
$radialVelocityLargeCount = 0
$smoothnessThresholdsMatch = $true
$summaryPattern = 'samples=(?<samples>\d+) soft_corrections=(?<soft_corrections>\d+) radial_corrections=(?<radial_corrections>\d+) hard_corrections=(?<hard_corrections>\d+) max_error=(?<max_error>[-+0-9.eE]+) avg_error=(?<avg_error>[-+0-9.eE]+) max_radial_correction=(?<max_radial_correction>[-+0-9.eE]+) avg_radial_correction=(?<avg_radial_correction>[-+0-9.eE]+) max_velocity_dir_delta=(?<max_velocity_dir_delta>[-+0-9.eE]+) avg_velocity_dir_delta=(?<avg_velocity_dir_delta>[-+0-9.eE]+) velocity_dir_delta_sum=(?<velocity_dir_delta_sum>[-+0-9.eE]+) velocity_dir_delta_large_count=(?<velocity_dir_delta_large_count>\d+) velocity_dir_delta_large_pct=(?<velocity_dir_delta_large_pct>[-+0-9.eE]+) max_consecutive_velocity_dir_delta=(?<max_consecutive_velocity_dir_delta>\d+) radial_velocity_removed_count=(?<radial_velocity_removed_count>\d+) radial_velocity_removed_pct=(?<radial_velocity_removed_pct>[-+0-9.eE]+) avg_radial_velocity_removed=(?<avg_radial_velocity_removed>[-+0-9.eE]+) max_radial_velocity_removed=(?<max_radial_velocity_removed>[-+0-9.eE]+) radial_velocity_large_count=(?<radial_velocity_large_count>\d+) radial_velocity_large_pct=(?<radial_velocity_large_pct>[-+0-9.eE]+) direction_delta_threshold=(?<direction_delta_threshold>[-+0-9.eE]+) radial_velocity_threshold=(?<radial_velocity_threshold>[-+0-9.eE]+)'
foreach ($line in $constraintSummaryLines) {
    $summaryMatch = [regex]::Match($line, $summaryPattern)
    if (-not $summaryMatch.Success) { continue }
    $samples = [int]$summaryMatch.Groups['samples'].Value
    $softCorrections = [int]$summaryMatch.Groups['soft_corrections'].Value
    $radialCorrections = [int]$summaryMatch.Groups['radial_corrections'].Value
    $hardCorrections = [int]$summaryMatch.Groups['hard_corrections'].Value
    $maxError = [double]::Parse($summaryMatch.Groups['max_error'].Value, [Globalization.CultureInfo]::InvariantCulture)
    $avgError = [double]::Parse($summaryMatch.Groups['avg_error'].Value, [Globalization.CultureInfo]::InvariantCulture)
    $maxRadial = [double]::Parse($summaryMatch.Groups['max_radial_correction'].Value, [Globalization.CultureInfo]::InvariantCulture)
    $avgRadial = [double]::Parse($summaryMatch.Groups['avg_radial_correction'].Value, [Globalization.CultureInfo]::InvariantCulture)
    $maxDirectionDelta = [double]::Parse($summaryMatch.Groups['max_velocity_dir_delta'].Value, [Globalization.CultureInfo]::InvariantCulture)
    $avgDirectionDelta = [double]::Parse($summaryMatch.Groups['avg_velocity_dir_delta'].Value, [Globalization.CultureInfo]::InvariantCulture)
    $directionDeltaSum = [double]::Parse($summaryMatch.Groups['velocity_dir_delta_sum'].Value, [Globalization.CultureInfo]::InvariantCulture)
    $directionDeltaLarge = [int]$summaryMatch.Groups['velocity_dir_delta_large_count'].Value
    $directionDeltaLargePct = [double]::Parse($summaryMatch.Groups['velocity_dir_delta_large_pct'].Value, [Globalization.CultureInfo]::InvariantCulture)
    $maxDirectionDeltaRun = [int]$summaryMatch.Groups['max_consecutive_velocity_dir_delta'].Value
    $radialRemoved = [int]$summaryMatch.Groups['radial_velocity_removed_count'].Value
    $radialRemovedPct = [double]::Parse($summaryMatch.Groups['radial_velocity_removed_pct'].Value, [Globalization.CultureInfo]::InvariantCulture)
    $avgRadialVelocityRemoved = [double]::Parse($summaryMatch.Groups['avg_radial_velocity_removed'].Value, [Globalization.CultureInfo]::InvariantCulture)
    $maxRadialVelocityRemovedForSummary = [double]::Parse($summaryMatch.Groups['max_radial_velocity_removed'].Value, [Globalization.CultureInfo]::InvariantCulture)
    $radialLarge = [int]$summaryMatch.Groups['radial_velocity_large_count'].Value
    $radialLargePct = [double]::Parse($summaryMatch.Groups['radial_velocity_large_pct'].Value, [Globalization.CultureInfo]::InvariantCulture)
    $reportedDirectionThreshold = [double]::Parse($summaryMatch.Groups['direction_delta_threshold'].Value, [Globalization.CultureInfo]::InvariantCulture)
    $reportedRadialThreshold = [double]::Parse($summaryMatch.Groups['radial_velocity_threshold'].Value, [Globalization.CultureInfo]::InvariantCulture)
    if ([math]::Abs($reportedDirectionThreshold - $directionDeltaThreshold) -gt 0.001 -or
        [math]::Abs($reportedRadialThreshold - $radialVelocityThreshold) -gt 0.001) {
        $smoothnessThresholdsMatch = $false
    }
    $maxRadialCorrection = [math]::Max($maxRadialCorrection, $maxRadial)
    $maxVelocityDirectionDelta = [math]::Max($maxVelocityDirectionDelta, $maxDirectionDelta)
    $softCorrectionCount += $softCorrections
    $hardCorrectionCount += $hardCorrections
    $totalConstraintSamples += $samples
    $velocityDirectionDeltaSum += $directionDeltaSum
    $velocityDirectionDeltaLargeCount += $directionDeltaLarge
    $maxConsecutiveVelocityDirectionDelta = [math]::Max($maxConsecutiveVelocityDirectionDelta, $maxDirectionDeltaRun)
    $radialVelocityRemovedCount += $radialRemoved
    $radialVelocityRemovedSum += $avgRadialVelocityRemoved * $radialRemoved
    $maxRadialVelocityRemoved = [math]::Max($maxRadialVelocityRemoved, $maxRadialVelocityRemovedForSummary)
    $radialVelocityLargeCount += $radialLarge
    $constraintSummaries += [pscustomobject]@{
        samples = $samples
        softCorrections = $softCorrections
        radialCorrections = $radialCorrections
        hardCorrections = $hardCorrections
        maxError = $maxError
        avgError = $avgError
        maxRadialCorrection = $maxRadial
        avgRadialCorrection = $avgRadial
        maxVelocityDirectionDelta = $maxDirectionDelta
        avgVelocityDirectionDelta = $avgDirectionDelta
        velocityDirectionDeltaSum = $directionDeltaSum
        velocityDirectionDeltaLargeCount = $directionDeltaLarge
        velocityDirectionDeltaLargePercent = $directionDeltaLargePct
        maxConsecutiveVelocityDirectionDelta = $maxDirectionDeltaRun
        radialVelocityRemovedCount = $radialRemoved
        radialVelocityRemovedPercent = $radialRemovedPct
        avgRadialVelocityRemoved = $avgRadialVelocityRemoved
        maxRadialVelocityRemoved = $maxRadialVelocityRemovedForSummary
        radialVelocityLargeCount = $radialLarge
        radialVelocityLargePercent = $radialLargePct
    }
}
$constraintDiagnosticsPass = $constraintSummaries.Count -ge 5 -and $hardCorrectionCount -eq 0
$aggregateAvgVelocityDirectionDelta = if ($totalConstraintSamples -gt 0) { $velocityDirectionDeltaSum / $totalConstraintSamples } else { 0.0 }
$aggregateVelocityDirectionDeltaPercent = if ($totalConstraintSamples -gt 0) { 100.0 * $velocityDirectionDeltaLargeCount / $totalConstraintSamples } else { 0.0 }
$aggregateRadialVelocityRemovedPercent = if ($totalConstraintSamples -gt 0) { 100.0 * $radialVelocityRemovedCount / $totalConstraintSamples } else { 0.0 }
$aggregateAvgRadialVelocityRemoved = if ($radialVelocityRemovedCount -gt 0) { $radialVelocityRemovedSum / $radialVelocityRemovedCount } else { 0.0 }
$aggregateRadialVelocityLargePercent = if ($totalConstraintSamples -gt 0) { 100.0 * $radialVelocityLargeCount / $totalConstraintSamples } else { 0.0 }
$smoothnessDiagnosticsPass = $constraintDiagnosticsPass -and
                             $smoothnessThresholdsMatch -and
                             $maxConsecutiveVelocityDirectionDelta -le $maxAllowedConsecutiveDirectionDeltas -and
                             $aggregateVelocityDirectionDeltaPercent -le $maxAllowedDirectionDeltaPercent
$anchorFanEvidencePass = $selectedServerAttempts.Count -ge 5 -and @($selectedServerAttempts | Where-Object {
    $match = [regex]::Match($_, 'probes=(\d+)')
    $match.Success -and [int]$match.Groups[1].Value -ge 15
}).Count -ge 5
$momentumSelectedAttempts = @($selectedServerAttempts | Where-Object { $_ -match 'momentum_basis=1' })
$divergent45Attempts = @($momentumSelectedAttempts | Where-Object {
    $match = [regex]::Match($_, 'facing_travel_dot=([-+0-9.eE]+)')
    if (-not $match.Success) { return $false }
    $dot = [double]::Parse($match.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture)
    [math]::Abs($dot) -ge 0.55 -and [math]::Abs($dot) -le 0.85
})
$divergent90Attempts = @($momentumSelectedAttempts | Where-Object {
    $match = [regex]::Match($_, 'facing_travel_dot=([-+0-9.eE]+)')
    if (-not $match.Success) { return $false }
    $dot = [double]::Parse($match.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture)
    [math]::Abs($dot) -le 0.35
})
$divergentAnchorEvidencePass = $divergent45Attempts.Count -ge 1 -and $divergent90Attempts.Count -ge 1
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
                      $smoothnessDiagnosticsPass -and
                      $anchorFanEvidencePass -and
                      $divergentAnchorEvidencePass
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
} elseif (-not $smoothnessDiagnosticsPass) {
    $reason = "Repeated smoothness discontinuity evidence exceeded the deterministic gate (avg_direction_delta=$([math]::Round($aggregateAvgVelocityDirectionDelta, 4)), large_direction_delta_pct=$([math]::Round($aggregateVelocityDirectionDeltaPercent, 2)), max_consecutive_large=$maxConsecutiveVelocityDirectionDelta, radial_velocity_removed_pct=$([math]::Round($aggregateRadialVelocityRemovedPercent, 2)))."
} elseif (-not $anchorFanEvidencePass) {
    $reason = "Broad anchor fan evidence was incomplete; expected five selected attempts with probes>=15."
} elseif (-not $divergentAnchorEvidencePass) {
    $reason = "Facing/travel-divergent anchor evidence was incomplete; expected selected momentum attempts near 45 and 90 degrees."
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
    smoothnessDiagnosticsPass = $smoothnessDiagnosticsPass
    smoothnessThresholdsMatch = $smoothnessThresholdsMatch
    directionDeltaThreshold = $directionDeltaThreshold
    radialVelocityThreshold = $radialVelocityThreshold
    maxAllowedConsecutiveDirectionDeltas = $maxAllowedConsecutiveDirectionDeltas
    maxAllowedDirectionDeltaPercent = $maxAllowedDirectionDeltaPercent
    constraintSummaryCount = $constraintSummaries.Count
    constraintSummaries = $constraintSummaries
    softCorrectionCount = $softCorrectionCount
    hardCorrectionCount = $hardCorrectionCount
    maxRadialCorrection = [math]::Round($maxRadialCorrection, 4)
    maxVelocityDirectionDelta = [math]::Round($maxVelocityDirectionDelta, 4)
    totalConstraintSamples = $totalConstraintSamples
    averageVelocityDirectionDelta = [math]::Round($aggregateAvgVelocityDirectionDelta, 6)
    velocityDirectionDeltaLargeCount = $velocityDirectionDeltaLargeCount
    velocityDirectionDeltaLargePercent = [math]::Round($aggregateVelocityDirectionDeltaPercent, 4)
    maxConsecutiveVelocityDirectionDelta = $maxConsecutiveVelocityDirectionDelta
    radialVelocityRemovedCount = $radialVelocityRemovedCount
    radialVelocityRemovedPercent = [math]::Round($aggregateRadialVelocityRemovedPercent, 4)
    averageRadialVelocityRemoved = [math]::Round($aggregateAvgRadialVelocityRemoved, 6)
    maxRadialVelocityRemoved = [math]::Round($maxRadialVelocityRemoved, 6)
    radialVelocityLargeCount = $radialVelocityLargeCount
    radialVelocityLargePercent = [math]::Round($aggregateRadialVelocityLargePercent, 4)
    anchorFanEvidencePass = $anchorFanEvidencePass
    momentumSelectedAnchorAttempts = $momentumSelectedAttempts.Count
    divergent45AnchorAttempts = $divergent45Attempts.Count
    divergent90AnchorAttempts = $divergent90Attempts.Count
    divergentAnchorEvidencePass = $divergentAnchorEvidencePass
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
