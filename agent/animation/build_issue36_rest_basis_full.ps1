[CmdletBinding()]
param(
    [string]$BlenderPath = 'D:\Blender\blender.exe',
    [string]$SourceBlend = 'agent\work\issue36-mixamo-full60-20260822\COHSOURCEDEV_RETARGET_POSE_PROOF.blend',
    [string]$SourceFbx = 'swinginganimations\Swinging.fbx',
    [string]$RigJson = 'agent\work\issue36-forensic-20260823\runtime\skelready2.json',
    [string]$RigSkelx = 'agent\work\issue36-forensic-20260823\runtime\skelready2.SKELX',
    [string]$BottomRuntimeReport = 'agent\work\issue36-rest-basis-bottom-20260824\runtime\bottom.json',
    [string]$OutputDir = 'agent\work\issue36-rest-basis-full-20260825',
    [switch]$Promote
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$asset = 'COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL'
$logical = "MALE/$asset"
$frames = @(1, 6, 12, 17, 18, 20, 22, 27, 33, 39, 45, 52, 60)

function Resolve-RepoPath([string]$Path, [switch]$AllowMissing) {
    $candidate = if ([System.IO.Path]::IsPathRooted($Path)) { $Path } else { Join-Path $root $Path }
    if ($AllowMissing) { return [System.IO.Path]::GetFullPath($candidate) }
    return (Resolve-Path -LiteralPath $candidate -ErrorAction Stop).Path
}

function Invoke-Checked([string]$Label, [string]$Executable, [string[]]$Arguments, [string]$WorkingDirectory = $root) {
    Write-Host "[$Label]"
    Push-Location $WorkingDirectory
    try {
        & $Executable @Arguments
        if ($LASTEXITCODE -ne 0) { throw "$Label failed with exit code $LASTEXITCODE" }
    }
    finally { Pop-Location }
}

$blender = Resolve-RepoPath $BlenderPath
$sourceBlendPath = Resolve-RepoPath $SourceBlend
$sourceFbxPath = Resolve-RepoPath $SourceFbx
$rigJsonPath = Resolve-RepoPath $RigJson
$rigSkelxPath = Resolve-RepoPath $RigSkelx
$bottomRuntimePath = Resolve-RepoPath $BottomRuntimeReport
$getAnimation = Resolve-RepoPath 'Utilities\GetAnimation2\bin\x86\Release\GetAnimation2.exe'
$output = Resolve-RepoPath $OutputDir -AllowMissing
$runtimeDir = Join-Path $output 'runtime'
$renderRoot = Join-Path $output 'contact-frames'
$blend = Join-Path $output "$asset.blend"
$animx = Join-Path $output "$asset.ANIMX"
$anim = Join-Path $output "$asset.anim"
$math = Join-Path $output 'rest-basis-full.math.json'
$runtimePrefix = Join-Path $runtimeDir 'full'
$runtimeJson = "$runtimePrefix.json"
$proofJson = Join-Path $output 'runtime-proof.json'
$installedRuntimePrefix = Join-Path $runtimeDir 'installed-full'
$installedRuntimeJson = "$installedRuntimePrefix.json"
$installedProofJson = Join-Path $output 'runtime-proof-installed.json'
$contactSheet = Join-Path $output 'rest-basis-full-contact-sheet.jpg'
$runtimeAsset = Join-Path $root "bin\data\player_library\animations\male\$asset.anim"
$runtimeBackup = Join-Path $output 'preexisting-runtime-asset.anim'

New-Item -ItemType Directory -Path $output,$runtimeDir,$renderRoot -Force | Out-Null

Invoke-Checked 'Generate corrected full Blender clip' $blender @(
    '--background', '--python', (Join-Path $root 'agent\animation\create_issue36_rest_basis_full.py'), '--',
    '--blend', $sourceBlendPath,
    '--source-fbx', $sourceFbxPath,
    '--rig-json', $rigJsonPath,
    '--output-dir', $output
)

Invoke-Checked 'Export corrected full ANIMX' $blender @(
    '--background', '--python', (Join-Path $root 'agent\animation\blender_export_animx.py'), '--',
    '--blend', $blend,
    '--rig-json', $rigJsonPath,
    '--output', $animx,
    '--armature-name', 'CoH_Male_Exact_Export_Rig',
    '--source-name', $logical,
    '--start-frame', '1',
    '--end-frame', '60'
)

Invoke-Checked 'Compile corrected full runtime animation' $getAnimation @(
    '-compile-animx', $animx, $rigSkelxPath, $logical, 'MALE/SKEL_READY2', $anim
)

# GetAnimation2 resolves logical names through bin/data. Temporarily stage only
# the new distinct private path and restore its exact prior state even when
# decode or proof fails. Promotion happens only after this preliminary proof.
$runtimeAssetExisted = Test-Path -LiteralPath $runtimeAsset -PathType Leaf
if ($runtimeAssetExisted) {
    Copy-Item -LiteralPath $runtimeAsset -Destination $runtimeBackup -Force
}
New-Item -ItemType Directory -Path (Split-Path -Parent $runtimeAsset) -Force | Out-Null
try {
    Copy-Item -LiteralPath $anim -Destination $runtimeAsset -Force
    Invoke-Checked 'Decode temporarily staged corrected full runtime animation' $getAnimation @(
        '-runtime-rig', $logical, $runtimePrefix
    ) (Join-Path $root 'bin')

    Invoke-Checked 'Prove decoded staged runtime correspondence' 'py' @(
        '-3.14', (Join-Path $root 'agent\animation\compare_issue36_rest_basis_full_runtime.py'),
        '--math-report', $math,
        '--runtime-report', $runtimeJson,
        '--stock-report', $rigJsonPath,
        '--bottom-runtime-report', $bottomRuntimePath,
        '--output-json', $proofJson
    )
}
finally {
    if ($runtimeAssetExisted) {
        Copy-Item -LiteralPath $runtimeBackup -Destination $runtimeAsset -Force
    }
    elseif (Test-Path -LiteralPath $runtimeAsset -PathType Leaf) {
        Remove-Item -LiteralPath $runtimeAsset -Force
    }
}

foreach ($frame in $frames) {
    $frameDir = Join-Path $renderRoot ("frame-{0:d2}" -f $frame)
    Invoke-Checked "Render source/target correspondence frame $frame" $blender @(
        '--background', '--python', (Join-Path $root 'agent\animation\render_issue36_production_correspondence.py'), '--',
        '--blend', $blend,
        '--rig-json', $rigJsonPath,
        '--frame', [string]$frame,
        '--output-dir', $frameDir
    )
}
Invoke-Checked 'Assemble corrected full contact sheet' 'py' @(
    '-3.14', (Join-Path $root 'agent\animation\make_issue36_rest_basis_full_contact_sheet.py'),
    '--input-root', $renderRoot,
    '--output', $contactSheet
)

if ($Promote) {
    $trackedAsset = Join-Path $root "agent\animation\runtime\player_library\animations\male\$asset.anim"
    $manifestPath = Join-Path $root 'agent\animation\runtime\webswing-animations.json'
    New-Item -ItemType Directory -Path (Split-Path -Parent $trackedAsset) -Force | Out-Null
    Copy-Item -LiteralPath $anim -Destination $trackedAsset -Force
    $hash = (Get-FileHash -LiteralPath $trackedAsset -Algorithm SHA256).Hash.ToLowerInvariant()
    $manifest = Get-Content -Raw -LiteralPath $manifestPath | ConvertFrom-Json
    $entry = @($manifest.animations | Where-Object { $_.logical -eq $asset })
    if ($entry.Count -gt 1) { throw "Manifest contains duplicate $asset entries" }
    $value = [pscustomobject]@{
        type = 'MALE'; logical = $asset; frames = 60
        file = "male/$asset.anim"; bytes = (Get-Item -LiteralPath $trackedAsset).Length
        sha256 = $hash
    }
    if ($entry.Count -eq 1) {
        $index = [array]::IndexOf([object[]]$manifest.animations, $entry[0])
        $manifest.animations[$index] = $value
    } else {
        $items = [System.Collections.ArrayList]@($manifest.animations)
        $insertAfter = -1
        for ($i = 0; $i -lt $items.Count; $i++) {
            if ($items[$i].logical -eq 'COHSOURCEDEV_RETARGET_RESTBASIS_BOTTOM') { $insertAfter = $i }
        }
        $items.Insert($insertAfter + 1, $value)
        $manifest.animations = $items
    }
    $manifest.version = 4
    $json = $manifest | ConvertTo-Json -Depth 6
    [System.IO.File]::WriteAllText($manifestPath, $json + "`n", [System.Text.UTF8Encoding]::new($false))

    # Use the existing private installer, then decode and prove the bytes at
    # the installed logical path. A successful preliminary proof is therefore
    # necessary but not sufficient for promotion to report PASS.
    $powerShell = (Get-Process -Id $PID).Path
    Invoke-Checked 'Install promoted private WebSwing animation library' $powerShell @(
        '-NoProfile', '-File', (Join-Path $root 'agent\install-webswing-animation.ps1'),
        '-Action', 'Install'
    )
    Invoke-Checked 'Decode installer-installed corrected full runtime animation' $getAnimation @(
        '-runtime-rig', $logical, $installedRuntimePrefix
    ) (Join-Path $root 'bin')
    Invoke-Checked 'Prove installer-installed runtime correspondence' 'py' @(
        '-3.14', (Join-Path $root 'agent\animation\compare_issue36_rest_basis_full_runtime.py'),
        '--math-report', $math,
        '--runtime-report', $installedRuntimeJson,
        '--stock-report', $rigJsonPath,
        '--bottom-runtime-report', $bottomRuntimePath,
        '--output-json', $installedProofJson
    )
}

$finalProofJson = if ($Promote) { $installedProofJson } else { $proofJson }
$proof = Get-Content -Raw -LiteralPath $finalProofJson | ConvertFrom-Json
[pscustomobject]@{
    passed = [bool]$proof.passed
    asset = $logical
    sourceFrameRange = $proof.sourceFrameRange
    authoredFrames = 60
    runtimeBoneCount = $proof.runtimeBoneCount
    maxDecodedRuntimeLocalErrorDegrees = $proof.maxDecodedRuntimeLocalErrorDegrees
    maxBindTranslationError = $proof.maxBindTranslationError
    maxAuthoredTranslationDrift = $proof.maxAuthoredTranslationDrift
    maxAcceptedBottomRotationErrorDegrees = $proof.maxAcceptedBottomRotationErrorDegrees
    blend = $blend
    animx = $animx
    runtimeAnimation = $anim
    runtimeReport = $runtimeJson
    proofReport = $finalProofJson
    contactSheet = $contactSheet
    promoted = [bool]$Promote
} | ConvertTo-Json -Depth 4
