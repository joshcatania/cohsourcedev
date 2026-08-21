# Remaster Profile v1 A/B capture harness.
# Writes file-backed graphics registry keys, runs agent/capture.ps1 for each
# requested scene, stores results under agent/captures/remaster-ab/<label>/,
# and restores the previous registry state in finally.
#
# Usage examples:
#   .\agent\remaster-ab.ps1 -Label stock -Targets AtlasPlaza_CityHall_03,AtlasPlaza_East_01,AtlasPlaza_NightEast_01,FoundersCanal_01
#   .\agent\remaster-ab.ps1 -Label remaster -Targets ... -ShadowMode 4 -AmbientStrength 3 -AmbientResolution 3 -AmbientBlur 5 -ModernPresentation -ModernBloom
#   .\agent\remaster-ab.ps1 -Label ultra -Targets ... -Ultra
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Label,
    [string[]]$Targets = @('AtlasPlaza_CityHall_03','AtlasPlaza_East_01','AtlasPlaza_NightEast_01','FoundersCanal_01'),
    [string]$AccountName = 'Dummy00010',
    [int]$TimeoutSeconds = 240,
    # Depth configuration (issue-25 semantics)
    [int]$ShadowMode = -1,          # -1 = leave registry as-is
    [int]$ShadowShader = -1,
    [int]$ShadowSize = -1,
    [int]$ShadowDistance = -2,      # -2 = leave as-is (0/1/2 = close/middle/far)
    [int]$AmbientStrength = -1,
    [int]$AmbientResolution = -1,
    [int]$AmbientBlur = -1,
    [int]$CubemapMode = -1,
    [int]$WaterMode = -1,
    # Presentation flags appended as client args
    [switch]$ModernPresentation,
    [switch]$ModernBloom,
    [switch]$NoModern,              # explicitly pass -modernPresentation 0 -modernBloom 0
    [switch]$GlslPilot0,            # legacy ARB control
    # Preset shortcuts
    [switch]$Ultra,                 # stock Ultra composition per gfxGetUltraAdvancedSettings
    [switch]$Json
)
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$registryRoot = Join-Path $repoRoot 'bin/registry-keys/hkey_current_user/software/cryptic/coh'
$captureScript = Join-Path $PSScriptRoot 'capture.ps1'
$outRoot = Join-Path $repoRoot "agent\captures\remaster-ab\$Label"
New-Item -ItemType Directory -Force -Path $outRoot | Out-Null

if ($Ultra) {
    # gfxGetUltraAdvancedSettings + gfxUpdateShadowMapAdvanced/gfxUpdateAmbientAdvanced
    $ShadowMode = 4      # SHADOW_SHADOWMAP_HIGH
    $ShadowShader = 3    # SHADOWSHADER_HIGHQ
    $ShadowSize = 1      # 1024
    $ShadowDistance = 2  # far (4 cascades)
    $AmbientStrength = 3 # AMBIENT_HIGH
    $AmbientResolution = 2 # AMBIENT_RES_QUALITY
    $AmbientBlur = 6     # AMBIENT_TRILATERAL
    $CubemapMode = 3     # CUBEMAP_HIGHQUALITY
    $WaterMode = 4       # WATER_ULTRA
}

$settings = [ordered]@{}
if ($ShadowMode -ge 0)      { $settings['shadowmode'] = $ShadowMode }
if ($ShadowShader -ge 0)    { $settings['shadowmapshader'] = $ShadowShader }
if ($ShadowSize -ge 0)      { $settings['shadowmapsize'] = $ShadowSize }
if ($ShadowDistance -ge 0)  { $settings['shadowmapdistance'] = $ShadowDistance }
if ($AmbientStrength -ge 0) { $settings['ambientstrength'] = $AmbientStrength }
if ($AmbientResolution -ge 0){ $settings['ambientresolution'] = $AmbientResolution }
if ($AmbientBlur -ge 0)     { $settings['ambientblur'] = $AmbientBlur }
if ($CubemapMode -ge 0)     { $settings['cubemapmode'] = $CubemapMode }
if ($WaterMode -ge 0)       { $settings['usewater'] = $WaterMode }

$extraArgs = ''
if ($ModernPresentation) { $extraArgs += ' -modernPresentation 1' }
if ($ModernBloom)        { $extraArgs += ' -modernBloom 1' }
if ($NoModern)           { $extraArgs += ' -modernPresentation 0 -modernBloom 0' }
if ($GlslPilot0)         { $extraArgs += ' -glslPilot 0' }

$original = @{}
foreach ($key in $settings.Keys) {
    $path = Join-Path $registryRoot $key
    if (Test-Path -LiteralPath $path) {
        $original[$key] = [pscustomobject]@{ exists = $true; value = Get-Content -Raw -LiteralPath $path }
    } else {
        $original[$key] = [pscustomobject]@{ exists = $false; value = $null }
    }
}

$summary = [ordered]@{
    label = $Label; startedAt = (Get-Date).ToString('o'); targets = $Targets
    settings = $settings; extraArgs = $extraArgs.Trim(); results = @()
}

try {
    foreach ($key in $settings.Keys) {
        Set-Content -LiteralPath (Join-Path $registryRoot $key) -Value ([string]$settings[$key]) -NoNewline
    }
    foreach ($target in $Targets) {
        Write-Host "[$Label] capturing $target ..."
        $capArgs = @('-NoProfile','-ExecutionPolicy','Bypass','-File', $captureScript,
                     '-Target', $target, '-AccountName', $AccountName,
                     '-TimeoutSeconds', $TimeoutSeconds.ToString(), '-Json')
        if ($extraArgs -match '\S') { $capArgs += @('-ExtraClientArgs', ('"{0}"' -f $extraArgs.Trim())) }
        $cap = & powershell.exe @capArgs | ConvertFrom-Json
        $dest = Join-Path $outRoot "$target.jpg"
        if ($cap.passed) { Copy-Item -LiteralPath $cap.screenshot -Destination $dest -Force }
        $summary.results += [ordered]@{
            target = $target; passed = $cap.passed; reason = $cap.reason; exitCode = $cap.exitCode
            screenshot = if ($cap.passed) { $dest } else { $null }
            screenshotOrig = $cap.screenshot
        }
        $cap | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $outRoot "$target.capture.json") -Encoding UTF8
    }
} finally {
    foreach ($key in $settings.Keys) {
        $path = Join-Path $registryRoot $key
        $saved = $original[$key]
        if ($saved.exists) {
            Set-Content -LiteralPath $path -Value $saved.value -NoNewline
        } elseif (Test-Path -LiteralPath $path) {
            Remove-Item -LiteralPath $path -Force
        }
    }
}

$summary.finishedAt = (Get-Date).ToString('o')
$summaryPath = Join-Path $outRoot 'summary.json'
$summary | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $summaryPath -Encoding UTF8
if ($Json) { $summary | ConvertTo-Json -Depth 6 }
else {
    $fail = @($summary.results | Where-Object { -not $_.passed })
    if ($fail.Count) { Write-Host "AB CAPTURE INCOMPLETE ($($fail.Count) failed): $(([string[]]($fail | ForEach-Object target)) -join ', ')" }
    else { Write-Host "AB CAPTURE COMPLETE [$Label]: $($summary.results.Count) shots under $outRoot" }
}
