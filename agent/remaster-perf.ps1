# Paired GPU-utilization A/B for stock default vs remaster profile captures.
# Interleaves stock/remaster captures in the same time window (to control for
# concurrent GPU load from other agents) and samples nvidia-smi at 1s.
[CmdletBinding()]
param(
    [int]$Pairs = 3,
    [string]$Target = 'AtlasPlaza_CityHall_03',
    [int]$TimeoutSeconds = 240
)
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$captureScript = Join-Path $PSScriptRoot 'capture.ps1'
$registryRoot = Join-Path $repoRoot 'bin/registry-keys/hkey_current_user/software/cryptic/coh'
$outRoot = Join-Path $repoRoot 'agent\work\remaster-perf'
New-Item -ItemType Directory -Force -Path $outRoot | Out-Null

# Remaster profile registry values (as saved by the preset) and the previous
# stock-default values for the toggled keys.
$remaster = @{
    shadowmode = '4'; shadowmapshader = '3'; shadowmapsize = '1'; shadowmapdistance = '1'
    ambientstrength = '3'; ambientresolution = '3'; ambientblur = '5'; usewater = '3'
}
$stock = @{
    shadowmode = '2'; shadowmapshader = '1'; shadowmapsize = '1'; shadowmapdistance = '1'
    ambientstrength = '0'; ambientresolution = '1'; ambientblur = '3'; usewater = '2'
}

function Set-Profile($profile) {
    foreach ($k in $profile.Keys) {
        Set-Content -LiteralPath (Join-Path $registryRoot $k) -Value $profile[$k] -NoNewline
    }
}

function Invoke-SampledCapture($label, $extraArgs) {
    # The intermittent map-entry entity race (entrecv/tricks, pre-existing)
    # fails some captures; retry so perf samples only successful runs.
    for ($attempt = 1; $attempt -le 4; $attempt++) {
        $samples = @()
        $resultPath = $null
        $procArgs = @('-NoProfile','-ExecutionPolicy','Bypass','-File', $captureScript,
                      '-Target', $Target, '-AccountName','Dummy00010',
                      '-TimeoutSeconds', $TimeoutSeconds.ToString())
        if ($extraArgs) { $procArgs += @('-ExtraClientArgs', ('"{0}"' -f $extraArgs)) }
        $p = Start-Process powershell.exe -ArgumentList ($procArgs -join ' ') -PassThru -WindowStyle Hidden
        while (-not $p.HasExited) {
            $s = & nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader 2>$null
            if ($s -match '^\s*(\d+)') { $samples += [int]$Matches[1] }
            Start-Sleep -Milliseconds 1000
        }
        $p.WaitForExit()
        if ($p.ExitCode -eq 0) {
            $m = ($samples | Measure-Object -Average -Maximum)
            '{0}: attempt={1} n={2} meanGPU={3:F1} maxGPU={4}' -f $label, $attempt, $samples.Count, $m.Average, $m.Maximum
            return [pscustomobject]@{ label = $label; n = $samples.Count; mean = [math]::Round($m.Average,1); max = $m.Maximum }
        }
        Write-Host "$label attempt $attempt failed (exit $($p.ExitCode)); retrying"
        # A failed attempt can leave a hung client holding this worktree's
        # bin; capture.ps1's guard would then reject every later attempt.
        Get-CimInstance Win32_Process -Filter "Name='Ouroboros.exe'" -ErrorAction SilentlyContinue |
            Where-Object { $_.ExecutablePath -like "$repoRoot\bin*" } |
            ForEach-Object { Write-Host "  cleaning hung client PID $($_.ProcessId)"; Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
        Start-Sleep -Seconds 2
    }
    Write-Host "${label}: all attempts failed"
    return $null
}

$results = @()
for ($i = 1; $i -le $Pairs; $i++) {
    Write-Host "=== pair $i/$Pairs ==="
    Set-Profile $stock
    $s = Invoke-SampledCapture "stock-p$i"   '-modernPresentation 0 -modernBloom 0'
    Set-Profile $remaster
    $r = Invoke-SampledCapture "remaster-p$i" ''
    # Keep pairs intact: both sides must have produced samples.
    if ($s -and $r) { $results += $s; $results += $r }
}
$results | ConvertTo-Json | Set-Content (Join-Path $outRoot 'perf.json') -Encoding UTF8
$stockRuns = @($results | Where-Object label -like 'stock-*')
$remRuns = @($results | Where-Object label -like 'remaster-*')
if ($stockRuns.Count -and $remRuns.Count) {
    $stockMean = ($stockRuns | Measure-Object mean -Average).Average
    $remMean = ($remRuns | Measure-Object mean -Average).Average
    'OVERALL stock meanGPU={0:F1} remaster meanGPU={1:F1} delta={2:F1}pp relative={3:F2}x' -f `
        $stockMean, $remMean, ($remMean - $stockMean), ($remMean / [math]::Max($stockMean, 0.1))
} else {
    'OVERALL: no complete pairs captured'
}
