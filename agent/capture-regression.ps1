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
    # Baseline directory. Missing baselines are adopted automatically unless
    # -NoAdopt is given; adopted shots are reported, never counted as passes.
    [string]$BaselineDir = '',
    [switch]$NoAdopt,
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

# Warmup capture: the first client to run on a fresh mapserver generation is
# also the one that freezes the world clock, and the sky/sun systems keep
# interpolating toward the frozen state for a while afterwards. Run one
# throwaway capture so every baseline and comparison sees a settled server.
if ($labels.Count -gt 0) {
    $null = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $captureScript -Target $labels[0] -AccountName $AccountName -Password $Password -TimeoutSeconds $TimeoutSeconds -Json 2>&1
}

foreach ($label in $labels) {
    $safeLabel = $label -replace '[^A-Za-z0-9_-]', '_'
    $artifact = Join-Path $captureDir "$safeLabel.jpg"
    $baseline = Join-Path $BaselineDir "$safeLabel.jpg"
    $entry = [ordered]@{ target=$label; artifact=$artifact; baseline=$baseline }

    $captureOutput = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $captureScript -Target $label -AccountName $AccountName -Password $Password -TimeoutSeconds $TimeoutSeconds -Json 2>&1 | Out-String
    $captureExit = $LASTEXITCODE
    $entry.captureExitCode = $captureExit
    $entry.capturePassed = ($captureExit -eq 0)

    if (-not $entry.capturePassed) {
        $entry.verdict = 'CAPTURE_FAILED'
        $entry.detail = ($captureOutput -split "`r?`n" | Where-Object { $_ -match '"passed"|"reason"' }) -join ' | '
        $results += [pscustomobject]$entry
        continue
    }

    if (-not (Test-Path -LiteralPath $baseline)) {
        if ($NoAdopt) {
            $entry.verdict = 'NO_BASELINE'
        } else {
            Copy-Item -LiteralPath $artifact -Destination $baseline -Force
            $entry.verdict = 'BASELINE_ADOPTED'
        }
        $results += [pscustomobject]$entry
        continue
    }

    $compareOutput = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $compareScript -Baseline $baseline -Current $artifact -Json 2>&1 | Out-String
    $compareExit = $LASTEXITCODE
    $entry.compareExitCode = $compareExit
    try {
        $entry.compare = ($compareOutput | ConvertFrom-Json)
    } catch {
        $entry.compare = $null
    }
    if ($compareExit -eq 0) {
        $entry.verdict = 'PASS'
    } else {
        $entry.verdict = 'REGRESSED'
    }
    $results += [pscustomobject]$entry
}

$passedCount  = @($results | Where-Object { $_.verdict -eq 'PASS' }).Count
$regressed    = @($results | Where-Object { $_.verdict -eq 'REGRESSED' }).Count
$failedCaps   = @($results | Where-Object { $_.verdict -eq 'CAPTURE_FAILED' }).Count
$adoptedCount = @($results | Where-Object { $_.verdict -eq 'BASELINE_ADOPTED' }).Count
$noBaseline   = @($results | Where-Object { $_.verdict -eq 'NO_BASELINE' }).Count
# Only regressions (and capture failures once baselines exist) fail the run;
# adoptions are informational.
$overallPassed = ($regressed -eq 0) -and ($failedCaps -eq 0) -and (($passedCount + $regressed) -gt 0 -or $adoptedCount -gt 0)

$summary = [ordered]@{
    passed     = $overallPassed
    startedAt  = $startedAt.ToString('o')
    finishedAt = (Get-Date).ToString('o')
    totals     = [ordered]@{
        targets=$labels.Count; passed=$passedCount; regressed=$regressed
        captureFailed=$failedCaps; baselineAdopted=$adoptedCount; noBaseline=$noBaseline
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
