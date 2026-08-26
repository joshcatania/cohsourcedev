[CmdletBinding()]
param(
    [string]$DbAddress = '127.0.0.1',
    [string]$AccountName = 'Dummy00009',
    [string]$CharacterName = '',
    [int]$TimeoutSeconds = 420,
    [ValidateSet('RealAnchor', 'SkyAssisted')]
    [string]$Backend = 'RealAnchor',
    [switch]$RequireClientDiagnostics,
    [switch]$RequireAnimationPhases,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$binDir = Join-Path $repoRoot 'bin'
$testClient = Join-Path $binDir 'TestClient.exe'
$logDir = Join-Path $PSScriptRoot 'logs'
$serverWebswingLog = Join-Path $binDir 'logs\mapserver\webswing.log'
$clientWebswingLog = Join-Path $binDir 'logs\TestClient\webswing.log'
$activeServerWebswingLog = Get-ChildItem (Join-Path $binDir 'logs\mapserver') -Filter 'webswing*.log' -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime -Descending | Select-Object -First 1
$activeClientWebswingLog = Get-ChildItem (Join-Path $binDir 'logs\TestClient') -Filter 'webswing*.log' -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime -Descending | Select-Object -First 1
if ($activeServerWebswingLog) { $serverWebswingLog = $activeServerWebswingLog.FullName }
if ($activeClientWebswingLog) { $clientWebswingLog = $activeClientWebswingLog.FullName }
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

function Get-LogNumber([string]$Line, [string]$Name) {
    $match = [regex]::Match($Line, "(?:^| )$([regex]::Escape($Name))=([-+0-9.eE]+)")
    if (-not $match.Success) { return $null }
    $value = 0.0
    if ([double]::TryParse($match.Groups[1].Value, [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref]$value)) {
        return $value
    }
    return $null
}

function Get-LogVector([string]$Line, [string]$Name) {
    $pattern = [regex]::Escape($Name) + '=\(([-+0-9.eE]+) ([-+0-9.eE]+) ([-+0-9.eE]+)\)'
    $match = [regex]::Match($Line, $pattern)
    if (-not $match.Success) { return $null }
    $values = @(
        [double]::Parse($match.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture)
        [double]::Parse($match.Groups[2].Value, [Globalization.CultureInfo]::InvariantCulture)
        [double]::Parse($match.Groups[3].Value, [Globalization.CultureInfo]::InvariantCulture)
    )
    return ,$values
}

function Get-AppendedLogText([string]$Path, [int64]$Offset) {
    if (-not (Test-Path -LiteralPath $Path)) { return '' }
    $stream = [System.IO.File]::Open($Path, [System.IO.FileMode]::Open,
        [System.IO.FileAccess]::Read, [System.IO.FileShare]::ReadWrite)
    try {
        if ($Offset -gt $stream.Length) { $Offset = 0 }
        $stream.Seek($Offset, [System.IO.SeekOrigin]::Begin) | Out-Null
        $reader = New-Object System.IO.StreamReader($stream)
        return $reader.ReadToEnd()
    } finally {
        $stream.Dispose()
    }
}

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
$webswingByteBefore = @{}
foreach ($definition in $webswingLogDefinitions) {
    $webswingByteBefore[$definition.Source] = if (Test-Path -LiteralPath $definition.Source) {
        (Get-Item -LiteralPath $definition.Source).Length
    } else {
        [int64]0
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
if ($Backend -eq 'SkyAssisted') {
    $argsList += '-webswing-sky-assisted'
}
if ($CharacterName) {
    $argsList += @('-character', $CharacterName)
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
        $webswingAppendedText = Get-AppendedLogText $definition.Source $webswingByteBefore[$definition.Source]
        Set-Content -Path $definition.Capture -Value $webswingAppendedText -Encoding UTF8
        $webswingTextParts += $webswingAppendedText
    } else {
        Set-Content -Path $definition.Capture -Value '' -Encoding UTF8
    }
}
$webswingText = $webswingTextParts -join "`n"

$webswingLines = @($webswingText -split "`r?`n" | Where-Object { $_ -match 'WEB_SWING' })
$expectedBackendName = if ($Backend -eq 'SkyAssisted') { 'SKY_ASSISTED' } else { 'REAL_ANCHOR' }
$animationPhaseLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING (CLIENT|SERVER) anim_phase=' })
$animationPhases = @($animationPhaseLines | ForEach-Object {
    $phaseMatch = [regex]::Match($_, 'anim_phase=([A-Z_]+)')
    if ($phaseMatch.Success) { $phaseMatch.Groups[1].Value }
} | Sort-Object -Unique)
$animationPhaseEvidencePass = $animationPhases.Count -ge 3 -and
                              $animationPhases -contains 'AIRBORNE' -and
                              $animationPhases -contains 'DESCEND' -and
                              ($animationPhases -contains 'BOTTOM' -or $animationPhases -contains 'ASCEND')
$clientAttempts = @($webswingLines | Where-Object { $_ -match 'WEB_SWING CLIENT attach_attempt' })
$serverAttempts = @($webswingLines | Where-Object { $_ -match 'WEB_SWING SERVER attach_attempt' })
$selectedClientAttempts = @($clientAttempts | Where-Object { $_ -match 'selected=1' })
$selectedServerAttempts = @($serverAttempts | Where-Object { $_ -match 'selected=1' })
$clientEnabledAttempts = @($clientAttempts | Where-Object { $_ -match 'web_swing_enabled=1' -and $_ -match 'up=[^ ]*[1-9]' })
$attachLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING (CLIENT|SERVER) attach ' })
$detachLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING (CLIENT|SERVER) detach ' })
$assistedTickLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING SERVER assisted_tick ' })
$assistedPhaseLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING SERVER assisted_phase ' })
$assistedCycleLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING SERVER assisted_cycle ' })
$assistedGroundBoostLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING SERVER ground_boost ' })
$assistedAltitudeBandBeginLines = @($webswingLines | Where-Object {
    $_ -match 'WEB_SWING SERVER assisted_altitude_band event=BEGIN '
})
$assistedAltitudeBandRaiseLines = @($webswingLines | Where-Object {
    $_ -match 'WEB_SWING SERVER assisted_altitude_band event=RAISE '
})
$assistedBottomGuardLines = @($webswingLines | Where-Object {
    $_ -match 'WEB_SWING SERVER assisted_bottom_guard '
})
$assistedAltitudeBottomGuardLines = @($assistedBottomGuardLines | Where-Object {
    $_ -match 'reason=ALTITUDE_BAND'
})
$assistedVisualReleaseLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING SERVER visual_tether_release ' })
$assistedVisualRetractedLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING SERVER visual_tether_retracted ' })
$assistedVisualAttachLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING SERVER visual_tether_attach ' })
$assistedVisualExtendedLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING SERVER visual_tether_extended ' })
$assistedVisualShootTimes = @($assistedVisualExtendedLines | ForEach-Object { Get-LogNumber $_ 'shoot_time' } | Where-Object { $null -ne $_ })
$assistedPhases = @($assistedPhaseLines | ForEach-Object {
    $phaseMatch = [regex]::Match($_, ' phase=([A-Z]+) ')
    if ($phaseMatch.Success) { $phaseMatch.Groups[1].Value }
} | Sort-Object -Unique)
$swingLines = if ($Backend -eq 'SkyAssisted') { $assistedTickLines } else {
    @($webswingLines | Where-Object { $_ -match 'WEB_SWING (CLIENT|SERVER) swing ' })
}
$chainHandoffLines = if ($Backend -eq 'SkyAssisted') { $assistedCycleLines } else {
    @($webswingLines | Where-Object { $_ -match 'WEB_SWING (CLIENT|SERVER) chain_handoff ' })
}
$backendChainHandoffLines = if ($Backend -eq 'SkyAssisted') { $assistedCycleLines } else {
    @($chainHandoffLines | Where-Object { $_ -match "backend=$expectedBackendName" })
}
$chainHandoffEvidencePass = $backendChainHandoffLines.Count -ge 1
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
$backendSelectedServerAttempts = @($selectedServerAttempts | Where-Object { $_ -match "backend=$expectedBackendName" })
$backendEvidencePass = $backendSelectedServerAttempts.Count -ge 5
$anchorFanEvidencePass = if ($Backend -eq 'SkyAssisted') {
    $backendEvidencePass -and @($backendSelectedServerAttempts | Where-Object {
        $_ -match 'probes=0' -and $_ -match 'ray_hits=0'
    }).Count -ge 5
} else {
    $backendEvidencePass -and @($backendSelectedServerAttempts | Where-Object {
        $match = [regex]::Match($_, 'probes=(\d+)')
        $match.Success -and [int]$match.Groups[1].Value -ge 15
    }).Count -ge 5
}
$momentumSelectedAttempts = @($selectedServerAttempts | Where-Object { $_ -match 'momentum_basis=1' })
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
$opposingSteeringLines = @($steeringLines | Where-Object {
    $alignment = Get-LogNumber $_ 'tangent_intent_alignment'
    $alignment -ne $null -and $alignment -lt 0.0 -and $_ -match 'intent_source=INPUT' -and $_ -match 'phase_pump_suppressed=1'
})
$opposingSteeringRecoveryCount = 0
$sawOpposingSteering = $false
foreach ($line in $steeringLines) {
    $alignment = Get-LogNumber $line 'tangent_intent_alignment'
    if ($line -match 'intent_source=INPUT' -and $line -match 'phase_pump_suppressed=1' -and $alignment -ne $null -and $alignment -lt 0.0) {
        $sawOpposingSteering = $true
    } elseif ($sawOpposingSteering -and $line -match 'intent_source=INPUT' -and $line -match 'phase_pump_suppressed=0' -and $alignment -ne $null -and $alignment -ge 0.0) {
        ++$opposingSteeringRecoveryCount
        $sawOpposingSteering = $false
    }
}
$opposingMomentumSteeringPass = $opposingSteeringLines.Count -ge 1 -and $opposingSteeringRecoveryCount -ge 1
$fullSequence = $attachLines.Count -ge 2 -and $detachLines.Count -ge 1 -and $swingLines.Count -ge 1
$combinedText = (($stdoutText, $stderrText) -join "`n")
$statusWritten = Test-Path -LiteralPath $statusLog
$statusText = if ($statusWritten) { Get-Content -Raw -LiteralPath $statusLog } else { '' }
$smokeComplete = $statusText -match 'webswing_smoke_complete=1'

function Get-AgentStatusValue([string]$Name) {
    $pattern = '(?m)^(' + [regex]::Escape($Name) + ')=([^\r\n]*)'
    $match = [regex]::Match($statusText, $pattern)
    if ($match.Success) { return $match.Groups[2].Value }
    return $null
}

$groundAttachObserved = 0
$groundAttachTransitions = 0
$groundAttachVerticalSpeed = 0.0
$groundAttachForwardSpeed = 0.0
$groundAltitudeGain = 0.0
$groundForwardDisplacement = 0.0
$forwardAttachObserved = 0
$forwardDisplacement = 0.0
$forwardPeakDisplacement = 0.0
$forwardPeakSpeed = 0.0
$forwardDetachSpeed = 0.0
[int]::TryParse((Get-AgentStatusValue 'webswing_ground_attach_observed'), [ref]$groundAttachObserved) | Out-Null
[int]::TryParse((Get-AgentStatusValue 'webswing_ground_attach_transitions'), [ref]$groundAttachTransitions) | Out-Null
[double]::TryParse((Get-AgentStatusValue 'webswing_ground_attach_vertical_speed'), [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref]$groundAttachVerticalSpeed) | Out-Null
[double]::TryParse((Get-AgentStatusValue 'webswing_ground_attach_forward_speed'), [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref]$groundAttachForwardSpeed) | Out-Null
[double]::TryParse((Get-AgentStatusValue 'webswing_ground_altitude_gain'), [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref]$groundAltitudeGain) | Out-Null
[double]::TryParse((Get-AgentStatusValue 'webswing_ground_forward_displacement'), [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref]$groundForwardDisplacement) | Out-Null
[int]::TryParse((Get-AgentStatusValue 'webswing_forward_attach_observed'), [ref]$forwardAttachObserved) | Out-Null
[double]::TryParse((Get-AgentStatusValue 'webswing_forward_displacement'), [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref]$forwardDisplacement) | Out-Null
[double]::TryParse((Get-AgentStatusValue 'webswing_forward_peak_displacement'), [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref]$forwardPeakDisplacement) | Out-Null
[double]::TryParse((Get-AgentStatusValue 'webswing_forward_peak_speed'), [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref]$forwardPeakSpeed) | Out-Null
[double]::TryParse((Get-AgentStatusValue 'webswing_forward_detach_speed'), [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref]$forwardDetachSpeed) | Out-Null

$attachCatchLines = @($webswingLines | Where-Object { $_ -match 'WEB_SWING (CLIENT|SERVER) attach_catch ' })
$serverAttachCatchLines = @($attachCatchLines | Where-Object { $_ -match 'WEB_SWING SERVER attach_catch ' })
$groundSelectedServerAttempts = @($serverAttempts | Where-Object {
    $_ -match 'grounded=1' -and $_ -match 'falling=0' -and $_ -match 'jumping=0' -and $_ -match 'selected=1' -and $_ -match 'anchor=\('
} | Select-Object -First 1)
$groundAnchorKeys = @($groundSelectedServerAttempts | ForEach-Object {
    $anchorMatch = [regex]::Match($_, 'anchor=\(([-+0-9.eE]+) ([-+0-9.eE]+) ([-+0-9.eE]+)\)')
    if ($anchorMatch.Success) {
        "anchor=($($anchorMatch.Groups[1].Value) $($anchorMatch.Groups[2].Value) $($anchorMatch.Groups[3].Value))"
    }
})
$groundAnchorPattern = if ($groundAnchorKeys.Count -gt 0) {
    '(?:' + (($groundAnchorKeys | ForEach-Object { [regex]::Escape($_) }) -join '|') + ')'
} else {
    '(?!)'
}
$serverGroundAttachCatchLines = @($serverAttachCatchLines | Where-Object {
    $_ -match 'grounded_at_acquisition=1' -and $_ -match 'falling_at_acquisition=0' -and
        $_ -match 'jumping_at_acquisition=0' -and $_ -match $groundAnchorPattern
})
$serverGroundLaunchBeginLines = @($webswingLines | Where-Object {
    $_ -match 'WEB_SWING SERVER ground_launch_begin ' -and $_ -match "backend=$expectedBackendName" -and
        $_ -match 'catch_suppressed=1' -and $_ -match $groundAnchorPattern
})
$serverGroundLaunchEndLines = @($webswingLines | Where-Object {
    $_ -match 'WEB_SWING SERVER ground_launch_end ' -and $_ -match 'reason=CLEARANCE' -and
        $_ -match $groundAnchorPattern
})
$groundCatchVerticalSpeed = 0.0
$groundCatchForwardSpeed = 0.0
$groundServerAltitudeGain = 0.0
$groundServerForwardDisplacement = 0.0
if ($serverGroundAttachCatchLines.Count -gt 0) {
    $groundCatchLine = $serverGroundAttachCatchLines[0]
    $catchMatch = [regex]::Match($groundCatchLine, 'velocity_after=\(([-+0-9.eE]+) ([-+0-9.eE]+) ([-+0-9.eE]+)\).*intent=\(([-+0-9.eE]+) ([-+0-9.eE]+) ([-+0-9.eE]+)\)')
    if ($catchMatch.Success) {
        $afterX = [double]::Parse($catchMatch.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture)
        $groundCatchVerticalSpeed = [double]::Parse($catchMatch.Groups[2].Value, [Globalization.CultureInfo]::InvariantCulture)
        $afterZ = [double]::Parse($catchMatch.Groups[3].Value, [Globalization.CultureInfo]::InvariantCulture)
        $intentX = [double]::Parse($catchMatch.Groups[4].Value, [Globalization.CultureInfo]::InvariantCulture)
        $intentZ = [double]::Parse($catchMatch.Groups[6].Value, [Globalization.CultureInfo]::InvariantCulture)
        $groundCatchForwardSpeed = $afterX * $intentX + $afterZ * $intentZ
    }
}
if ($groundSelectedServerAttempts.Count -gt 0 -and $groundAnchorKeys.Count -gt 0) {
    $groundAttachPosition = Get-LogVector $groundSelectedServerAttempts[0] 'pos'
    $groundIntentVector = Get-LogVector $groundSelectedServerAttempts[0] 'intent'
    if ($null -ne $groundAttachPosition -and $null -ne $groundIntentVector) {
        for ($lineIndex = 0; $lineIndex -lt $webswingLines.Count; ++$lineIndex) {
            $line = $webswingLines[$lineIndex]
            if ($line -match 'WEB_SWING SERVER detach ' -and $line -match $groundAnchorPattern) {
                $groundDetachPosition = Get-LogVector $line 'pos'
                if ($null -ne $groundDetachPosition) {
                    $groundServerAltitudeGain = $groundDetachPosition[1] - $groundAttachPosition[1]
                    $groundServerForwardDisplacement =
                        ($groundDetachPosition[0] - $groundAttachPosition[0]) * $groundIntentVector[0] +
                        ($groundDetachPosition[2] - $groundAttachPosition[2]) * $groundIntentVector[2]
                }
                break
            }
        }
    }
}
$groundOriginAttachEvidencePass = $statusWritten -and
                                  $groundSelectedServerAttempts.Count -ge 1 -and
                                  $serverGroundLaunchBeginLines.Count -ge 1 -and
                                  ($serverGroundLaunchEndLines.Count -ge 1 -or $groundAltitudeGain -ge 0.50) -and
                                  ($groundAltitudeGain -ge 0.50 -or $groundServerAltitudeGain -ge 0.50)

$intentInputAttempts = @($selectedServerAttempts | Where-Object {
    $_ -match 'intent_source=INPUT' -and $_ -match 'meaningful_momentum=1'
})
$intent45Attempts = @($intentInputAttempts | Where-Object {
    $momentumIntentDot = Get-LogNumber $_ 'intent_momentum_alignment'
    $selectedIntent = Get-LogNumber $_ 'selected_intent_alignment'
    $selectedMomentum = Get-LogNumber $_ 'selected_momentum_alignment'
    $null -ne $momentumIntentDot -and $null -ne $selectedIntent -and $null -ne $selectedMomentum -and
        [math]::Abs($momentumIntentDot) -ge 0.45 -and [math]::Abs($momentumIntentDot) -le 0.90 -and
        $selectedIntent -gt $selectedMomentum
})
$intent90Attempts = @($intentInputAttempts | Where-Object {
    $momentumIntentDot = Get-LogNumber $_ 'intent_momentum_alignment'
    $selectedIntent = Get-LogNumber $_ 'selected_intent_alignment'
    $selectedMomentum = Get-LogNumber $_ 'selected_momentum_alignment'
    $null -ne $momentumIntentDot -and $null -ne $selectedIntent -and $null -ne $selectedMomentum -and
        [math]::Abs($momentumIntentDot) -le 0.35 -and $selectedIntent -gt $selectedMomentum
})
$intentAnchorEvidencePass = $intent45Attempts.Count -ge 1 -and $intent90Attempts.Count -ge 1
$divergent45Attempts = $intent45Attempts
$divergent90Attempts = $intent90Attempts
$divergentAnchorEvidencePass = $intentAnchorEvidencePass
$forwardSelectedServerAttempts = @($selectedServerAttempts | Where-Object {
    $position = Get-LogVector $_ 'pos'
    $_ -match 'intent_source=(?:INPUT|FACING)' -and $_ -match 'selected=1' -and $_ -match 'anchor=\(' -and
        $null -ne $position -and $position[1] -ge 115.0 -and $position[1] -le 145.0 -and
        [math]::Abs($position[0] - 100.0) -le 8.0 -and
        [math]::Abs($position[2] + 650.0) -le 20.0
} | Select-Object -First 1)
$forwardAnchorKeys = @($forwardSelectedServerAttempts | ForEach-Object {
    $anchorMatch = [regex]::Match($_, 'anchor=\(([-+0-9.eE]+) ([-+0-9.eE]+) ([-+0-9.eE]+)\)')
    if ($anchorMatch.Success) {
        "anchor=($($anchorMatch.Groups[1].Value) $($anchorMatch.Groups[2].Value) $($anchorMatch.Groups[3].Value))"
    }
})
$forwardAnchorPattern = if ($forwardAnchorKeys.Count -gt 0) {
    '(?:' + (($forwardAnchorKeys | ForEach-Object { [regex]::Escape($_) }) -join '|') + ')'
} else {
    '(?!)'
}
$serverForwardAttachCatchLines = @($serverAttachCatchLines | Where-Object {
    $_ -match 'intent_source=(?:INPUT|FACING)' -and $_ -match $forwardAnchorPattern
})
$forwardCatchVerticalSpeed = 0.0
$forwardCatchForwardSpeed = 0.0
foreach ($catchLine in $serverForwardAttachCatchLines) {
    $catchMatch = [regex]::Match($catchLine, 'velocity_after=\(([-+0-9.eE]+) ([-+0-9.eE]+) ([-+0-9.eE]+)\).*intent=\(([-+0-9.eE]+) ([-+0-9.eE]+) ([-+0-9.eE]+)\)')
    if ($catchMatch.Success) {
        $afterX = [double]::Parse($catchMatch.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture)
        $forwardCatchVerticalSpeed = [double]::Parse($catchMatch.Groups[2].Value, [Globalization.CultureInfo]::InvariantCulture)
        $afterZ = [double]::Parse($catchMatch.Groups[3].Value, [Globalization.CultureInfo]::InvariantCulture)
        $intentX = [double]::Parse($catchMatch.Groups[4].Value, [Globalization.CultureInfo]::InvariantCulture)
        $intentZ = [double]::Parse($catchMatch.Groups[6].Value, [Globalization.CultureInfo]::InvariantCulture)
        $forwardCatchForwardSpeed = $afterX * $intentX + $afterZ * $intentZ
        break
    }
}
$forwardServerPeakSpeed = 0.0
$forwardServerPeakDisplacement = 0.0
$forwardServerDetachSpeed = 0.0
$forwardServerDisplacement = 0.0
if ($forwardAnchorKeys.Count -gt 0) {
    $forwardAttachPosition = if ($forwardSelectedServerAttempts.Count -gt 0) {
        Get-LogVector $forwardSelectedServerAttempts[0] 'pos'
    } else { $null }
    $forwardIntentVector = if ($forwardSelectedServerAttempts.Count -gt 0) {
        Get-LogVector $forwardSelectedServerAttempts[0] 'intent'
    } else { $null }
    $forwardCatchIndex = -1
    for ($lineIndex = 0; $lineIndex -lt $webswingLines.Count; ++$lineIndex) {
        if ($webswingLines[$lineIndex] -match 'WEB_SWING SERVER attach_catch ' -and
            $webswingLines[$lineIndex] -match $forwardAnchorPattern) {
            $forwardCatchIndex = $lineIndex
            break
        }
    }
    if ($forwardCatchIndex -ge 0) {
        for ($lineIndex = $forwardCatchIndex + 1; $lineIndex -lt $webswingLines.Count; ++$lineIndex) {
            $line = $webswingLines[$lineIndex]
            if ($line -match 'WEB_SWING SERVER swing ') {
                $speedMatch = [regex]::Match($line, 'speed=([-+0-9.eE]+)')
                if ($speedMatch.Success) {
                    $forwardServerPeakSpeed = [math]::Max($forwardServerPeakSpeed,
                        [double]::Parse($speedMatch.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture))
                }
                $swingPosition = Get-LogVector $line 'pos'
                if ($null -ne $swingPosition -and $null -ne $forwardAttachPosition -and $null -ne $forwardIntentVector) {
                    $swingDisplacement =
                        ($swingPosition[0] - $forwardAttachPosition[0]) * $forwardIntentVector[0] +
                        ($swingPosition[2] - $forwardAttachPosition[2]) * $forwardIntentVector[2]
                    $forwardServerPeakDisplacement = [math]::Max($forwardServerPeakDisplacement, $swingDisplacement)
                }
            }
            if ($line -match 'WEB_SWING SERVER detach ' -and $line -match $forwardAnchorPattern) {
                $speedMatch = [regex]::Match($line, 'detach speed=([-+0-9.eE]+)')
                if ($speedMatch.Success) {
                    $forwardServerDetachSpeed = [double]::Parse($speedMatch.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture)
                }
                $forwardDetachPosition = Get-LogVector $line 'pos'
                if ($null -ne $forwardAttachPosition -and $null -ne $forwardDetachPosition -and $null -ne $forwardIntentVector) {
                    $forwardServerDisplacement =
                        ($forwardDetachPosition[0] - $forwardAttachPosition[0]) * $forwardIntentVector[0] +
                        ($forwardDetachPosition[2] - $forwardAttachPosition[2]) * $forwardIntentVector[2]
                }
                break
            }
        }
    }
}
$forwardServerPeakSpeed = [math]::Max($forwardServerPeakSpeed, $forwardServerDetachSpeed)
$forwardTravelEvidencePass = $statusWritten -and
                              $forwardSelectedServerAttempts.Count -ge 1 -and
                              $serverForwardAttachCatchLines.Count -eq 1 -and
                              $forwardCatchVerticalSpeed -ge 0.50 -and
                              $forwardCatchForwardSpeed -ge 0.50 -and
                              ($forwardServerDisplacement -ge 1.0 -or $forwardServerPeakDisplacement -ge 1.0 -or $forwardPeakDisplacement -ge 1.0) -and
                              $forwardServerPeakSpeed -gt 0.50 -and
                              $forwardServerDetachSpeed -gt 0.25
$assistedSteeringEvidence = @{}
$assistedBottomSpeeds = @()
$assistedUpperSpeeds = @()
$assistedBottomHorizontalSpeeds = @()
$assistedApexHorizontalSpeeds = @()
$assistedTickEnergies = @()
foreach ($line in $assistedTickLines) {
    $phaseMatch = [regex]::Match($line, 'phase=([A-Z]+)')
    $speedMatch = [regex]::Match($line, ' speed=([-+0-9.eE]+)')
    $horizontalSpeedMatch = [regex]::Match($line, ' horizontal_speed=([-+0-9.eE]+)')
    $energyMatch = [regex]::Match($line, ' energy=([-+0-9.eE]+)')
    if ($phaseMatch.Success -and $speedMatch.Success) {
        $sampleSpeed = [double]::Parse($speedMatch.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture)
        if ($phaseMatch.Groups[1].Value -eq 'BOTTOM') { $assistedBottomSpeeds += $sampleSpeed }
        elseif ($phaseMatch.Groups[1].Value -in @('ASCEND', 'APEX')) { $assistedUpperSpeeds += $sampleSpeed }
    }
    if ($phaseMatch.Success -and $horizontalSpeedMatch.Success) {
        $sampleHorizontalSpeed = [double]::Parse($horizontalSpeedMatch.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture)
        if ($phaseMatch.Groups[1].Value -eq 'BOTTOM') { $assistedBottomHorizontalSpeeds += $sampleHorizontalSpeed }
        elseif ($phaseMatch.Groups[1].Value -eq 'APEX') { $assistedApexHorizontalSpeeds += $sampleHorizontalSpeed }
    }
    if ($energyMatch.Success) {
        $assistedTickEnergies += [double]::Parse($energyMatch.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture)
    }

    $intentMatch = [regex]::Match($line, 'intent=\(([-+0-9.eE]+) ([-+0-9.eE]+) ([-+0-9.eE]+)\) input_magnitude=([-+0-9.eE]+)')
    $velocityMatch = [regex]::Match($line, 'velocity=\(([-+0-9.eE]+) ([-+0-9.eE]+) ([-+0-9.eE]+)\)')
    if (-not $intentMatch.Success -or -not $velocityMatch.Success) { continue }
    $inputMagnitude = [double]::Parse($intentMatch.Groups[4].Value, [Globalization.CultureInfo]::InvariantCulture)
    if ($inputMagnitude -le 0.05) { continue }
    $ix = [double]::Parse($intentMatch.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture)
    $iz = [double]::Parse($intentMatch.Groups[3].Value, [Globalization.CultureInfo]::InvariantCulture)
    $vx = [double]::Parse($velocityMatch.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture)
    $vz = [double]::Parse($velocityMatch.Groups[3].Value, [Globalization.CultureInfo]::InvariantCulture)
    $horizontalSpeed = [math]::Sqrt($vx * $vx + $vz * $vz)
    if ($horizontalSpeed -gt 0.25 -and (($ix * $vx + $iz * $vz) / $horizontalSpeed) -gt 0.30) {
        $intentKey = '{0:F1},{1:F1}' -f $ix, $iz
        $assistedSteeringEvidence[$intentKey] = 1
    }
}
$assistedBottomPeakSpeed = if ($assistedBottomSpeeds.Count) { ($assistedBottomSpeeds | Measure-Object -Maximum).Maximum } else { 0.0 }
$assistedUpperAverageSpeed = if ($assistedUpperSpeeds.Count) { ($assistedUpperSpeeds | Measure-Object -Average).Average } else { 0.0 }
$assistedBottomSpeedPass = $assistedBottomPeakSpeed -gt ($assistedUpperAverageSpeed + 0.15)
$assistedBottomAverageHorizontalSpeed = if ($assistedBottomHorizontalSpeeds.Count) { ($assistedBottomHorizontalSpeeds | Measure-Object -Average).Average } else { 0.0 }
$assistedApexAverageHorizontalSpeed = if ($assistedApexHorizontalSpeeds.Count) { ($assistedApexHorizontalSpeeds | Measure-Object -Average).Average } else { 0.0 }
$assistedPendulumSpeedPass = $assistedBottomHorizontalSpeeds.Count -gt 0 -and
                             $assistedApexHorizontalSpeeds.Count -gt 0 -and
                             $assistedBottomAverageHorizontalSpeed -gt ($assistedApexAverageHorizontalSpeed + 0.50)
$assistedMinEnergy = if ($assistedTickEnergies.Count) { ($assistedTickEnergies | Measure-Object -Minimum).Minimum } else { 0.0 }
$assistedMaxEnergy = if ($assistedTickEnergies.Count) { ($assistedTickEnergies | Measure-Object -Maximum).Maximum } else { 0.0 }
$assistedEnergyGrowth = $assistedMaxEnergy - $assistedMinEnergy
$assistedEnergyGrowthPass = $assistedTickEnergies.Count -gt 0 -and
                            $assistedMaxEnergy -ge 0.70 -and
                            $assistedEnergyGrowth -ge 0.45
$assistedGroundBoostPass = @($assistedGroundBoostLines | Where-Object {
    $upMatch = [regex]::Match($_, 'up_speed=([-+0-9.eE]+)')
    $forwardMatch = [regex]::Match($_, 'forward_speed=([-+0-9.eE]+)')
    $upMatch.Success -and $forwardMatch.Success -and
    [double]::Parse($upMatch.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture) -ge 2.0 -and
    [double]::Parse($forwardMatch.Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture) -ge 1.5
}).Count -ge 1
$assistedVisualCadencePass = $assistedVisualReleaseLines.Count -ge 2 -and
                             $assistedVisualRetractedLines.Count -ge 2 -and
                             $assistedVisualAttachLines.Count -ge 2 -and
                             $assistedVisualExtendedLines.Count -ge 2 -and
                             @($assistedVisualReleaseLines | Where-Object { $_ -match 'physics_continuous=1' }).Count -eq $assistedVisualReleaseLines.Count -and
                             @($assistedVisualAttachLines | Where-Object {
                                 $gapMatch = [regex]::Match($_, 'gap_ticks=([0-9]+)')
                                 $_ -match 'physics_continuous=1' -and $gapMatch.Success -and [int]$gapMatch.Groups[1].Value -ge 18
                             }).Count -eq $assistedVisualAttachLines.Count
$assistedVisualShootTimingPass = $assistedVisualShootTimes.Count -eq $assistedVisualExtendedLines.Count -and
                                  @($assistedVisualExtendedLines | Where-Object {
                                      $shootTime = Get-LogNumber $_ 'shoot_time'
                                      $windupTime = Get-LogNumber $_ 'windup_time'
                                      $null -ne $shootTime -and $null -ne $windupTime -and
                                      [math]::Abs($windupTime - 12.0) -le 0.001 -and
                                      $shootTime -ge 14.5 -and $shootTime -le 18.0
                                  }).Count -eq $assistedVisualExtendedLines.Count
$assistedAheadProbePass = @($assistedTickLines | Where-Object {
    $_ -match 'current_clearance=' -and $_ -match 'ahead_clearance=' -and $_ -match 'lookahead='
}).Count -ge 10
$assistedAltitudeBandBeginPass = @($assistedAltitudeBandBeginLines | Where-Object {
    $attachY = Get-LogNumber $_ 'attach_y'
    $lowPointY = Get-LogNumber $_ 'low_point_y'
    $rope = Get-LogNumber $_ 'rope'
    $null -ne $attachY -and $null -ne $lowPointY -and $null -ne $rope -and
    $attachY -gt $lowPointY -and ($attachY - $lowPointY) -le 15.0 -and
    $rope -ge 8.0 -and $_ -match 'preserve_elevation=1'
}).Count -eq $assistedAltitudeBandBeginLines.Count -and $assistedAltitudeBandBeginLines.Count -ge 5
$assistedAltitudeBandRaisePass = @($assistedAltitudeBandRaiseLines | Where-Object {
    $previous = Get-LogNumber $_ 'previous_low_point_y'
    $candidate = Get-LogNumber $_ 'candidate_low_point_y'
    $lowPoint = Get-LogNumber $_ 'low_point_y'
    $maxRise = Get-LogNumber $_ 'max_rise'
    $gainCeiling = Get-LogNumber $_ 'gain_ceiling_y'
    $null -ne $previous -and $null -ne $candidate -and $null -ne $lowPoint -and
    $null -ne $maxRise -and $null -ne $gainCeiling -and
    $lowPoint -gt $previous -and $lowPoint -le $candidate + 0.01 -and
    ($lowPoint - $previous) -le $maxRise + 0.01 -and
    $lowPoint -le $gainCeiling + 0.01 -and $_ -match 'preserve_elevation=1'
}).Count -eq $assistedAltitudeBandRaiseLines.Count -and $assistedAltitudeBandRaiseLines.Count -ge 1
$assistedAltitudeTelemetryPass = @($assistedTickLines | Where-Object {
    $_ -match 'low_point_y=' -and $_ -match 'altitude_margin='
}).Count -ge 10
$assistedAltitudeBandPass = $assistedAltitudeBandBeginPass -and
                            $assistedAltitudeBandRaisePass -and
                            $assistedAltitudeTelemetryPass -and
                            $assistedAltitudeBottomGuardLines.Count -ge 1
$missingAssistedPhases = @(@('LAUNCH', 'ASCEND', 'APEX', 'DESCEND', 'BOTTOM') | Where-Object {
    $assistedPhases -notcontains $_
})
$assistedPhaseEvidencePass = $missingAssistedPhases.Count -eq 0
$assistedControllerEvidencePass = $assistedTickLines.Count -ge 10 -and
                                  $assistedCycleLines.Count -ge 2 -and
                                  $assistedPhaseEvidencePass -and
                                  $assistedBottomSpeedPass -and
                                  $assistedPendulumSpeedPass -and
                                  $assistedEnergyGrowthPass -and
                                  $assistedGroundBoostPass -and
                                  $assistedVisualCadencePass -and
                                  $assistedVisualShootTimingPass -and
                                  $assistedAheadProbePass -and
                                  $assistedAltitudeBandPass

if ($Backend -eq 'SkyAssisted') {
    # SKY_ASSISTED intentionally has no rope constraint, anchor-quality, or
    # opposing-pendulum diagnostics. Its objective gates are controller
    # cadence, ground/forward travel, multiple redirected input headings,
    # bottom-speed emphasis, release momentum, and geometry-free acquisition.
    $constraintDiagnosticsPass = $hardCorrectionCount -eq 0 -and $totalConstraintSamples -eq 0
    $smoothnessDiagnosticsPass = $constraintDiagnosticsPass
    $steeringEvidencePass = $assistedSteeringEvidence.Count -ge 4
    $missingSteeringEvidence = if ($steeringEvidencePass) { @() } else { @('four assisted redirect headings') }
    $intentAnchorEvidencePass = $backendEvidencePass
    $opposingMomentumSteeringPass = $steeringEvidencePass
    $forwardTravelEvidencePass = $statusWritten -and
                                 $forwardDisplacement -ge 20.0 -and
                                 $groundForwardDisplacement -ge 50.0 -and
                                 $groundAltitudeGain -ge 5.0 -and
                                 $maxDetachSpeed -gt 0.25
}
$clientDiagnosticsAvailable = $selectedClientAttempts.Count -ge 3 -and $clientEnabledAttempts.Count -ge 3
$serverSequencePass = $smokeComplete -and
                      ($selectedServerAttempts.Count -ge 5) -and
                      $fullSequence -and
                      $chainHandoffEvidencePass -and
                      $retainedMomentumDetachPass -and
                      $constraintDiagnosticsPass -and
                      $smoothnessDiagnosticsPass -and
                      $backendEvidencePass -and
                      $anchorFanEvidencePass -and
                      $intentAnchorEvidencePass -and
                      $groundOriginAttachEvidencePass -and
                      $forwardTravelEvidencePass -and
                      $opposingMomentumSteeringPass
if ($Backend -eq 'SkyAssisted') {
    $serverSequencePass = $serverSequencePass -and $assistedControllerEvidencePass
}
$exitCodeValue = $null
if (-not $timedOut) { try { $exitCodeValue = [int]$proc.ExitCode } catch {} }

$passed = (-not $timedOut) -and ($exitCodeValue -eq 0) -and $serverSequencePass -and
          $steeringEvidencePass -and
          (-not $RequireAnimationPhases -or $animationPhaseEvidencePass) -and
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
} elseif (-not $chainHandoffEvidencePass) {
    $reason = "Automatic held-swing handoff evidence was missing for $expectedBackendName."
} elseif ($Backend -eq 'SkyAssisted' -and -not $assistedControllerEvidencePass) {
    $reason = "Assisted cadence evidence was incomplete (ticks=$($assistedTickLines.Count), cycles=$($assistedCycleLines.Count), boosts=$($assistedGroundBoostLines.Count), visual_releases=$($assistedVisualReleaseLines.Count), visual_retracted=$($assistedVisualRetractedLines.Count), visual_attaches=$($assistedVisualAttachLines.Count), visual_extended=$($assistedVisualExtendedLines.Count), shoot_timing=$assistedVisualShootTimingPass, ahead_probe=$assistedAheadProbePass, altitude_band=$assistedAltitudeBandPass, altitude_begins=$($assistedAltitudeBandBeginLines.Count), altitude_raises=$($assistedAltitudeBandRaiseLines.Count), altitude_guards=$($assistedAltitudeBottomGuardLines.Count), missing_phases=$($missingAssistedPhases -join ','), bottom_peak=$([math]::Round($assistedBottomPeakSpeed, 3)), upper_avg=$([math]::Round($assistedUpperAverageSpeed, 3)), bottom_horizontal_avg=$([math]::Round($assistedBottomAverageHorizontalSpeed, 3)), apex_horizontal_avg=$([math]::Round($assistedApexAverageHorizontalSpeed, 3)), energy_growth=$([math]::Round($assistedEnergyGrowth, 3)))."
} elseif (-not $retainedMomentumDetachPass) {
    $reason = "Space-release momentum evidence was incomplete; expected a non-trivial detach speed (max=$([math]::Round($maxDetachSpeed, 3)))."
} elseif (-not $steeringEvidencePass) {
    $reason = "World-space W/A/D steering evidence was incomplete; missing: $($missingSteeringEvidence -join ', ')."
} elseif (-not $constraintDiagnosticsPass) {
    $reason = "Soft constraint diagnostics were incomplete or reported hard corrections (summaries=$($constraintSummaries.Count), hard=$hardCorrectionCount)."
} elseif (-not $smoothnessDiagnosticsPass) {
    $reason = "Repeated smoothness discontinuity evidence exceeded the deterministic gate (avg_direction_delta=$([math]::Round($aggregateAvgVelocityDirectionDelta, 4)), large_direction_delta_pct=$([math]::Round($aggregateVelocityDirectionDeltaPercent, 2)), max_consecutive_large=$maxConsecutiveVelocityDirectionDelta, radial_velocity_removed_pct=$([math]::Round($aggregateRadialVelocityRemovedPercent, 2)))."
} elseif (-not $anchorFanEvidencePass) {
    $reason = if ($Backend -eq 'SkyAssisted') {
        "SKY_ASSISTED geometry-independent acquisition evidence was incomplete; expected five selected attempts with probes=0 and ray_hits=0."
    } else {
        "REAL_ANCHOR broad anchor fan evidence was incomplete; expected five selected attempts with probes>=15."
    }
} elseif (-not $groundOriginAttachEvidencePass) {
    $reason = "Ground-origin Web Swing launch evidence was incomplete (selected=$($groundSelectedServerAttempts.Count) launch_begin=$($serverGroundLaunchBeginLines.Count) launch_end=$($serverGroundLaunchEndLines.Count) replicated_altitude_gain=$([math]::Round($groundAltitudeGain, 3)) server_altitude_gain=$([math]::Round($groundServerAltitudeGain, 3)))."
} elseif (-not $intentAnchorEvidencePass) {
    $reason = "Intent-first anchor evidence was incomplete; expected selected INPUT anchors with meaningful stale momentum at roughly 45 and 90 degrees (45=$($intent45Attempts.Count), 90=$($intent90Attempts.Count))."
} elseif (-not $forwardTravelEvidencePass) {
    $reason = "Held-swing forward-travel evidence was incomplete (selected=$($forwardSelectedServerAttempts.Count) catches=$($serverForwardAttachCatchLines.Count) catch_forward=$([math]::Round($forwardCatchForwardSpeed, 3)) replicated_displacement=$([math]::Round($forwardDisplacement, 3)) server_displacement=$([math]::Round($forwardServerDisplacement, 3)) server_peak_displacement=$([math]::Round($forwardServerPeakDisplacement, 3)) server_peak_speed=$([math]::Round($forwardServerPeakSpeed, 3)) server_detach_speed=$([math]::Round($forwardServerDetachSpeed, 3)))."
} elseif (-not $opposingMomentumSteeringPass) {
    $reason = "Opposing-momentum steering evidence was incomplete (suppressed=$($opposingSteeringLines.Count) recoveries=$opposingSteeringRecoveryCount)."
} elseif ($RequireAnimationPhases -and -not $animationPhaseEvidencePass) {
    $reason = "Animation phase evidence was incomplete; expected AIRBORNE, DESCEND, and BOTTOM or ASCEND (phases=$($animationPhases -join ', '))."
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
    characterName = $CharacterName
    dbAddress = $DbAddress
    backend = $Backend
    expectedBackendName = $expectedBackendName
    backendEvidencePass = $backendEvidencePass
    backendSelectedServerAttempts = $backendSelectedServerAttempts.Count
    statusWritten = $statusWritten
    clientAnchorAttempts = $clientAttempts.Count
    serverAnchorAttempts = $serverAttempts.Count
    clientSelectedAnchors = $selectedClientAttempts.Count
    serverSelectedAnchors = $selectedServerAttempts.Count
    clientEnabledAttempts = $clientEnabledAttempts.Count
    clientDiagnosticsAvailable = $clientDiagnosticsAvailable
    requireClientDiagnostics = [bool]$RequireClientDiagnostics
    requireAnimationPhases = [bool]$RequireAnimationPhases
    animationPhaseLines = $animationPhaseLines.Count
    animationPhases = $animationPhases
    animationPhaseEvidencePass = $animationPhaseEvidencePass
    attachLines = $attachLines.Count
    attachCatchLines = $attachCatchLines.Count
    serverAttachCatchLines = $serverAttachCatchLines.Count
    swingLines = $swingLines.Count
    detachLines = $detachLines.Count
    detachSpeeds = @($detachSpeeds | ForEach-Object { [math]::Round($_, 3) })
    maxDetachSpeed = [math]::Round($maxDetachSpeed, 3)
    retainedMomentumDetachPass = $retainedMomentumDetachPass
    fullSequence = $fullSequence
    chainHandoffLines = $chainHandoffLines.Count
    backendChainHandoffLines = $backendChainHandoffLines.Count
    chainHandoffEvidencePass = $chainHandoffEvidencePass
    assistedTickLines = $assistedTickLines.Count
    assistedPhaseLines = $assistedPhaseLines.Count
    assistedPhases = $assistedPhases
    missingAssistedPhases = $missingAssistedPhases
    assistedCycleLines = $assistedCycleLines.Count
    assistedControllerEvidencePass = $assistedControllerEvidencePass
    assistedSteeringHeadings = @($assistedSteeringEvidence.Keys | Sort-Object)
    assistedSteeringEvidencePass = ($assistedSteeringEvidence.Count -ge 4)
    assistedBottomPeakSpeed = [math]::Round($assistedBottomPeakSpeed, 3)
    assistedUpperAverageSpeed = [math]::Round($assistedUpperAverageSpeed, 3)
    assistedBottomSpeedPass = $assistedBottomSpeedPass
    assistedBottomAverageHorizontalSpeed = [math]::Round($assistedBottomAverageHorizontalSpeed, 3)
    assistedApexAverageHorizontalSpeed = [math]::Round($assistedApexAverageHorizontalSpeed, 3)
    assistedPendulumSpeedPass = $assistedPendulumSpeedPass
    assistedMinEnergy = [math]::Round($assistedMinEnergy, 3)
    assistedMaxEnergy = [math]::Round($assistedMaxEnergy, 3)
    assistedEnergyGrowth = [math]::Round($assistedEnergyGrowth, 3)
    assistedEnergyGrowthPass = $assistedEnergyGrowthPass
    assistedGroundBoostLines = $assistedGroundBoostLines.Count
    assistedGroundBoostPass = $assistedGroundBoostPass
    assistedVisualReleaseLines = $assistedVisualReleaseLines.Count
    assistedVisualRetractedLines = $assistedVisualRetractedLines.Count
    assistedVisualAttachLines = $assistedVisualAttachLines.Count
    assistedVisualExtendedLines = $assistedVisualExtendedLines.Count
    assistedVisualCadencePass = $assistedVisualCadencePass
    assistedVisualShootTimes = @($assistedVisualShootTimes | ForEach-Object { [math]::Round($_, 3) })
    assistedVisualShootTimingPass = $assistedVisualShootTimingPass
    assistedAheadProbePass = $assistedAheadProbePass
    assistedAltitudeBandBeginLines = $assistedAltitudeBandBeginLines.Count
    assistedAltitudeBandRaiseLines = $assistedAltitudeBandRaiseLines.Count
    assistedBottomGuardLines = $assistedBottomGuardLines.Count
    assistedAltitudeBottomGuardLines = $assistedAltitudeBottomGuardLines.Count
    assistedAltitudeBandBeginPass = $assistedAltitudeBandBeginPass
    assistedAltitudeBandRaisePass = $assistedAltitudeBandRaisePass
    assistedAltitudeTelemetryPass = $assistedAltitudeTelemetryPass
    assistedAltitudeBandPass = $assistedAltitudeBandPass
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
    groundOriginAttachEvidencePass = $groundOriginAttachEvidencePass
    groundAttachObserved = $groundAttachObserved
    groundAttachTransitions = $groundAttachTransitions
    groundAttachServerCatchCount = $serverGroundAttachCatchLines.Count
    groundLaunchBeginCount = $serverGroundLaunchBeginLines.Count
    groundLaunchEndCount = $serverGroundLaunchEndLines.Count
    groundSelectedServerAttempts = $groundSelectedServerAttempts.Count
    groundAttachVerticalSpeed = [math]::Round($groundAttachVerticalSpeed, 3)
    groundAttachForwardSpeed = [math]::Round($groundAttachForwardSpeed, 3)
    groundCatchVerticalSpeed = [math]::Round($groundCatchVerticalSpeed, 3)
    groundCatchForwardSpeed = [math]::Round($groundCatchForwardSpeed, 3)
    groundServerAltitudeGain = [math]::Round($groundServerAltitudeGain, 3)
    groundServerForwardDisplacement = [math]::Round($groundServerForwardDisplacement, 3)
    groundAltitudeGain = [math]::Round($groundAltitudeGain, 3)
    groundForwardDisplacement = [math]::Round($groundForwardDisplacement, 3)
    forwardTravelEvidencePass = $forwardTravelEvidencePass
    forwardSelectedServerAttempts = $forwardSelectedServerAttempts.Count
    forwardServerCatchCount = $serverForwardAttachCatchLines.Count
    forwardCatchVerticalSpeed = [math]::Round($forwardCatchVerticalSpeed, 3)
    forwardCatchForwardSpeed = [math]::Round($forwardCatchForwardSpeed, 3)
    forwardServerPeakSpeed = [math]::Round($forwardServerPeakSpeed, 3)
    forwardServerDetachSpeed = [math]::Round($forwardServerDetachSpeed, 3)
    forwardServerDisplacement = [math]::Round($forwardServerDisplacement, 3)
    forwardServerPeakDisplacement = [math]::Round($forwardServerPeakDisplacement, 3)
    forwardPeakDisplacement = [math]::Round($forwardPeakDisplacement, 3)
    forwardAttachObserved = $forwardAttachObserved
    forwardDisplacement = [math]::Round($forwardDisplacement, 3)
    forwardPeakSpeed = [math]::Round($forwardPeakSpeed, 3)
    forwardDetachSpeed = [math]::Round($forwardDetachSpeed, 3)
    momentumSelectedAnchorAttempts = $momentumSelectedAttempts.Count
    intentInputAttempts = $intentInputAttempts.Count
    intent45AnchorAttempts = $intent45Attempts.Count
    intent90AnchorAttempts = $intent90Attempts.Count
    intentAnchorEvidencePass = $intentAnchorEvidencePass
    divergent45AnchorAttempts = $divergent45Attempts.Count
    divergent90AnchorAttempts = $divergent90Attempts.Count
    divergentAnchorEvidencePass = $divergentAnchorEvidencePass
    opposingSteeringLines = $opposingSteeringLines.Count
    opposingSteeringRecoveryCount = $opposingSteeringRecoveryCount
    opposingMomentumSteeringPass = $opposingMomentumSteeringPass
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
