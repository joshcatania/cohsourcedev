[CmdletBinding()]
param(
    # Comma-separated capture labels. Each must exist in the game's capture
    # shot table (Game/src/game.c, s_captureShots); unknown labels capture the
    # default shot but still produce artifacts under their own name.
    # Closeup_01 is excluded by default: at camdist 10 the player's idle
    # animation phase differs between runs, which is future engine work.
    [string]$Targets = 'AtlasPlaza_CityHall_03,AtlasPlaza_East_01,AtlasPlaza_North_01,AtlasPlaza_West_01',
    [string]$AccountName = 'Dummy00018',
    [string]$Password = '11111111',
    [int]$TimeoutSeconds = 180,
    # Baseline directory. Missing baselines are a failure in formal/default
    # mode. Use -AdoptMissingBaseline only for an intentional baseline run.
    [string]$BaselineDir = '',
    # Extra client arguments forwarded verbatim to capture.ps1 for every shot.
    # Omit -glslPilot for the default hybrid suite; use '-glslPilot 0' for an
    # explicit legacy ARB/Cg control suite.
    [string]$ExtraClientArgs = '',
    [int]$PixelTolerance = 12,
    [double]$MaxChangedPercent = 6.0,
    [double]$MaxMeanDelta = 3.0,
    [int]$CompareWidth = 320,
    [switch]$AdoptMissingBaseline,
    [switch]$Json
)
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$captureScript = Join-Path $PSScriptRoot 'capture.ps1'
$compareScript = Join-Path $PSScriptRoot 'compare-captures.ps1'
$captureDir = Join-Path $repoRoot 'agent\captures'
if (-not $BaselineDir) { $BaselineDir = Join-Path $repoRoot 'agent\baselines' }
$logRoot = Join-Path $repoRoot 'agent\logs'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$summaryPath = Join-Path $logRoot "regression-$stamp.json"
New-Item -ItemType Directory -Force -Path $BaselineDir, $captureDir, $logRoot | Out-Null

$startedAt = Get-Date
$results = @()
$labels = @($Targets -split ',' | ForEach-Object { $_.Trim() } | Where-Object { $_ })

# splatted only when non-empty: an empty -ExtraClientArgs would be dropped by
# the child invocation and reported as a missing parameter
$extraClientArgList = @()
if ($ExtraClientArgs) { $extraClientArgList = @('-ExtraClientArgs', $ExtraClientArgs) }

# Warmup capture: the first client to run on a fresh mapserver generation is
# also the one that freezes the world clock, and the sky/sun systems keep
# interpolating toward the frozen state for a while afterwards. Run one
# throwaway capture so every baseline and comparison sees a settled server.
if ($labels.Count -gt 0) {
    $null = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $captureScript -Target $labels[0] -AccountName $AccountName -Password $Password -TimeoutSeconds $TimeoutSeconds @extraClientArgList -Json 2>&1
}

foreach ($label in $labels) {
    $safeLabel = $label -replace '[^A-Za-z0-9_-]', '_'
    $artifact = Join-Path $captureDir "$safeLabel.jpg"
    $baseline = Join-Path $BaselineDir "$safeLabel.jpg"
    $entry = [ordered]@{ target=$label; artifact=$artifact; baseline=$baseline }

    $captureOutput = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $captureScript -Target $label -AccountName $AccountName -Password $Password -TimeoutSeconds $TimeoutSeconds @extraClientArgList -Json 2>&1 | Out-String
    $captureExit = $LASTEXITCODE
    $entry.captureExitCode = $captureExit
    $entry.capturePassed = ($captureExit -eq 0)

    if (-not $entry.capturePassed) {
        $entry.verdict = 'CAPTURE_FAILED'
        $entry.failureReasons = @($captureOutput -split "`r?`n" | Where-Object { $_ -match '"passed"|"reason"|"error"' })
        $entry.detail = if ($entry.failureReasons.Count -gt 0) { $entry.failureReasons -join ' | ' } else { 'capture.ps1 exited with a non-zero status' }
        $results += [pscustomobject]$entry
        continue
    }

    if (-not (Test-Path -LiteralPath $baseline)) {
        if ($AdoptMissingBaseline) {
            Copy-Item -LiteralPath $artifact -Destination $baseline -Force
            $entry.verdict = 'BASELINE_ADOPTED'
            $entry.detail = 'Missing baseline adopted by explicit -AdoptMissingBaseline; this is not a parity PASS.'
        } else {
            $entry.verdict = 'NO_BASELINE'
            $entry.detail = 'Missing baseline; formal/default mode never adopts automatically. Re-run with -AdoptMissingBaseline only for intentional baseline creation.'
        }
        $results += [pscustomobject]$entry
        continue
    }

    $compareOutput = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $compareScript -Baseline $baseline -Current $artifact -PixelTolerance $PixelTolerance -MaxChangedPercent $MaxChangedPercent -MaxMeanDelta $MaxMeanDelta -CompareWidth $CompareWidth -Json 2>&1 | Out-String
    $compareExit = $LASTEXITCODE
    $entry.compareExitCode = $compareExit
    try {
        $entry.compare = ($compareOutput | ConvertFrom-Json)
    } catch {
        $entry.compare = $null
    }
    if ($compareExit -eq 0 -and $entry.compare -and $entry.compare.passed) {
        $entry.verdict = 'PASS'
    } else {
        $entry.verdict = 'REGRESSED'
        if ($entry.compare -and $entry.compare.failureReasons) {
            $entry.failureReasons = @($entry.compare.failureReasons)
            $entry.detail = $entry.failureReasons -join '; '
        } elseif ($entry.compare -and $entry.compare.error) {
            $entry.failureReasons = @("comparison error: $($entry.compare.error)")
            $entry.detail = $entry.failureReasons -join '; '
        } else {
            $entry.failureReasons = @('comparison did not produce a passing result')
            $entry.detail = $entry.failureReasons -join '; '
        }
    }
    if ($entry.compare -and $entry.compare.advisories) {
        $entry.advisories = @($entry.compare.advisories)
    }
    $results += [pscustomobject]$entry
}

$passedCount  = @($results | Where-Object { $_.verdict -eq 'PASS' }).Count
$regressed    = @($results | Where-Object { $_.verdict -eq 'REGRESSED' }).Count
$failedCaps   = @($results | Where-Object { $_.verdict -eq 'CAPTURE_FAILED' }).Count
$adoptedCount = @($results | Where-Object { $_.verdict -eq 'BASELINE_ADOPTED' }).Count
$noBaseline   = @($results | Where-Object { $_.verdict -eq 'NO_BASELINE' }).Count
$overallPassed = ($labels.Count -gt 0) -and ($passedCount -eq $labels.Count)

$summary = [ordered]@{
    passed     = $overallPassed
    startedAt  = $startedAt.ToString('o')
    finishedAt = (Get-Date).ToString('o')
    totals     = [ordered]@{
        targets=$labels.Count; passed=$passedCount; regressed=$regressed
        captureFailed=$failedCaps; baselineAdopted=$adoptedCount; noBaseline=$noBaseline
    }
    policy = [ordered]@{
        baselineMode = if ($AdoptMissingBaseline) { 'explicit-adoption' } else { 'formal-no-adoption' }
        missingBaselineVerdict = 'NO_BASELINE'
        adoptionVerdict = 'BASELINE_ADOPTED'
        baselineAdoptionCountsAsPass = $false
        comparison = [ordered]@{
            hardCriterion = 'changedPercent'
            advisoryCriterion = 'meanDelta'
            meanDeltaAction = 'report-only'
            thresholds = [ordered]@{
                pixelTolerance=$PixelTolerance; maxChangedPercent=$MaxChangedPercent
                maxMeanDelta=$MaxMeanDelta; compareWidth=$CompareWidth
            }
        }
    }
    results    = $results
    summaryPath = $summaryPath
}
$summary | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $summaryPath -Encoding UTF8
if ($Json) { $summary | ConvertTo-Json -Depth 6 }
else {
    foreach ($r in $results) {
        Write-Host ("{0,-24} {1}" -f $r.target, $r.verdict)
    }
    Write-Host "Summary: $summaryPath"
}
if ($overallPassed) { exit 0 } else { exit 1 }
