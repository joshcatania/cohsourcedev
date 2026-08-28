[CmdletBinding()]
param(
    [string]$BlenderPath = 'D:\Blender\blender.exe',
    [string]$SourceBlend = 'agent\work\issue36-rest-basis-full-20260825\COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL.blend',
    [string]$SourceFbx = 'swinginganimations\Swinging.fbx',
    [string]$RigJson = 'agent\work\issue36-forensic-20260823\runtime\skelready2.json',
    [string]$RigSkelx = 'agent\work\issue36-forensic-20260823\runtime\skelready2.SKELX',
    [string]$OutputDir = 'agent\work\issue36-rest-basis-grip-20260827',
    [switch]$Promote
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$asset = 'COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL_GRIP'
$logical = "MALE/$asset"
$baseAsset = 'COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL'
$baseLogical = "MALE/$baseAsset"
$acceptedBaseSha256 = 'acc0fc2d0ce4f382ec1cbf6675dfb3f6f96747080ffe24f6cc1c53866dfec840'
$gripBones = @('F1_L', 'F2_L', 'T1_L', 'T2_L', 'T3_L')

function Resolve-RepoPath([string]$Path, [switch]$AllowMissing) {
    $candidate = if ([System.IO.Path]::IsPathRooted($Path)) { $Path } else { Join-Path $root $Path }
    if ($AllowMissing) { return [System.IO.Path]::GetFullPath($candidate) }
    return (Resolve-Path -LiteralPath $candidate -ErrorAction Stop).Path
}

function Get-Sha256([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
}

function Get-RepoRelative([string]$Path) {
    $full = [System.IO.Path]::GetFullPath($Path)
    return ([System.IO.Path]::GetRelativePath($root, $full)).Replace('\', '/')
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

function Restore-StagedFile([string]$Path, [bool]$Existed, [string]$Backup) {
    if ($Existed) {
        Copy-Item -LiteralPath $Backup -Destination $Path -Force
    }
    elseif (Test-Path -LiteralPath $Path -PathType Leaf) {
        Remove-Item -LiteralPath $Path -Force
    }
}

function Update-Manifest([string]$ManifestPath, [string]$TrackedAssetPath, [string]$SourceFbxPath,
                         [string]$SourceBlendPath, [string]$BaseRuntimeHash, [string]$DerivedHash) {
    $manifest = Get-Content -Raw -LiteralPath $ManifestPath | ConvertFrom-Json
    $entries = @($manifest.animations)
    $existing = @($entries | Where-Object { $_.logical -eq $asset })
    if ($existing.Count -gt 1) { throw "Manifest contains duplicate $asset entries" }

    $source = [pscustomobject]@{
        fbx = Get-RepoRelative $SourceFbxPath
        fbxSha256 = Get-Sha256 $SourceFbxPath
        baseBlend = Get-RepoRelative $SourceBlendPath
        baseBlendSha256 = Get-Sha256 $SourceBlendPath
        baseRuntimeAsset = $baseLogical
        baseRuntimeSha256 = $BaseRuntimeHash
        action = 'Armature|mixamo.com|Layer0'
        sourceFingerMap = [pscustomobject]@{
            F2_L = 'mixamorig:LeftHandRing1'
            F1_L = 'mixamorig:LeftHandRing2'
            T3_L = 'mixamorig:LeftHandThumb1'
            T2_L = 'mixamorig:LeftHandThumb2'
            T1_L = 'mixamorig:LeftHandThumb3'
        }
    }
    $build = [pscustomobject]@{
        script = 'agent/animation/build_issue36_rest_basis_grip.ps1'
        blenderScript = 'agent/animation/create_issue36_rest_basis_grip.py'
        exporter = 'agent/animation/blender_export_animx.py'
        compiler = 'Utilities/GetAnimation2/bin/x86/Release/GetAnimation2.exe'
    }
    $modification = [pscustomobject]@{
        bones = $gripBones
        authoredFrames = '16..30'
        closureProfile = '0,0.30,0.60,0.85,1,1,1,1,1,1,1,0.85,0.60,0.30,0'
        sourceReference = 'left ring/thumb channels from Swinging.fbx, slerped in the accepted runtime-local channel space'
        rootTranslationPolicy = 'fixed bind translations; zero pose-bone location; unit pose-bone scale'
        nonFingerChannels = 'unchanged'
    }
    $value = [pscustomobject]@{
        type = 'MALE'
        logical = $asset
        frames = 60
        fps = 30
        file = "male/$asset.anim"
        bytes = (Get-Item -LiteralPath $TrackedAssetPath).Length
        sha256 = $DerivedHash
        source = $source
        build = $build
        modification = $modification
    }

    if ($existing.Count -eq 1) {
        $index = [array]::IndexOf([object[]]$entries, $existing[0])
        $entries[$index] = $value
    }
    else {
        $items = [System.Collections.ArrayList]@($entries)
        $insertAfter = -1
        for ($i = 0; $i -lt $items.Count; $i++) {
            if ($items[$i].logical -eq $baseAsset) { $insertAfter = $i }
        }
        if ($insertAfter -lt 0) { throw "Manifest is missing the accepted base asset $baseAsset" }
        $items.Insert($insertAfter + 1, $value)
        $entries = @($items)
    }
    $manifest.animations = $entries
    $json = $manifest | ConvertTo-Json -Depth 10
    [System.IO.File]::WriteAllText($ManifestPath, $json + "`n", [System.Text.UTF8Encoding]::new($false))
}

$blender = Resolve-RepoPath $BlenderPath
$sourceBlendPath = Resolve-RepoPath $SourceBlend
$sourceFbxPath = Resolve-RepoPath $SourceFbx
$rigJsonPath = Resolve-RepoPath $RigJson
$rigSkelxPath = Resolve-RepoPath $RigSkelx
$getAnimation = Resolve-RepoPath 'Utilities\GetAnimation2\bin\x86\Release\GetAnimation2.exe'
$output = Resolve-RepoPath $OutputDir -AllowMissing
$runtimeDir = Join-Path $output 'runtime'
$blend = Join-Path $output "$asset.blend"
$animx = Join-Path $output "$asset.ANIMX"
$anim = Join-Path $output "$asset.anim"
$math = Join-Path $output 'rest-basis-grip.math.json'
$baseRuntimePrefix = Join-Path $runtimeDir 'accepted-base'
$baseRuntimeJson = "$baseRuntimePrefix.json"
$gripRuntimePrefix = Join-Path $runtimeDir 'derived-grip'
$gripRuntimeJson = "$gripRuntimePrefix.json"
$proofJson = Join-Path $output 'runtime-grip-proof.json'
$installedBasePrefix = Join-Path $runtimeDir 'installed-base'
$installedBaseJson = "$installedBasePrefix.json"
$installedGripPrefix = Join-Path $runtimeDir 'installed-grip'
$installedGripJson = "$installedGripPrefix.json"
$installedProofJson = Join-Path $output 'runtime-grip-proof-installed.json'
$baseTrackedAsset = Join-Path $root "agent\animation\runtime\player_library\animations\male\$baseAsset.anim"
$baseDataAsset = Join-Path $root "bin\data\player_library\animations\male\$baseAsset.anim"
$gripDataAsset = Join-Path $root "bin\data\player_library\animations\male\$asset.anim"
$trackedAsset = Join-Path $root "agent\animation\runtime\player_library\animations\male\$asset.anim"
$manifestPath = Join-Path $root 'agent\animation\runtime\webswing-animations.json'
$baseBackup = Join-Path $output 'preexisting-base-runtime.anim'
$gripBackup = Join-Path $output 'preexisting-grip-runtime.anim'

New-Item -ItemType Directory -Path $output, $runtimeDir -Force | Out-Null
if (-not (Test-Path -LiteralPath $baseTrackedAsset -PathType Leaf)) {
    throw "Accepted tracked base animation is missing: $baseTrackedAsset"
}
$baseRuntimeHash = Get-Sha256 $baseTrackedAsset
if ($baseRuntimeHash -ne $acceptedBaseSha256) {
    throw "Accepted base runtime hash changed: $baseRuntimeHash (expected $acceptedBaseSha256)"
}

Invoke-Checked 'Generate derived left-hand grip Blender clip' $blender @(
    '--background', '--python', (Join-Path $root 'agent\animation\create_issue36_rest_basis_grip.py'), '--',
    '--blend', $sourceBlendPath,
    '--source-fbx', $sourceFbxPath,
    '--rig-json', $rigJsonPath,
    '--output-dir', $output,
    '--base-runtime-sha256', $baseRuntimeHash
)

Invoke-Checked 'Export derived grip ANIMX' $blender @(
    '--background', '--python', (Join-Path $root 'agent\animation\blender_export_animx.py'), '--',
    '--blend', $blend,
    '--rig-json', $rigJsonPath,
    '--output', $animx,
    '--armature-name', 'CoH_Male_Exact_Export_Rig',
    '--source-name', $logical,
    '--start-frame', '1',
    '--end-frame', '60'
)

Invoke-Checked 'Compile derived grip runtime animation' $getAnimation @(
    '-compile-animx', $animx, $rigSkelxPath, $logical, 'MALE/SKEL_READY2', $anim
)

$baseDataExisted = Test-Path -LiteralPath $baseDataAsset -PathType Leaf
$gripDataExisted = Test-Path -LiteralPath $gripDataAsset -PathType Leaf
if ($baseDataExisted) { Copy-Item -LiteralPath $baseDataAsset -Destination $baseBackup -Force }
if ($gripDataExisted) { Copy-Item -LiteralPath $gripDataAsset -Destination $gripBackup -Force }
New-Item -ItemType Directory -Path (Split-Path -Parent $baseDataAsset) -Force | Out-Null
try {
    # Decode both logical names from the same bin/data tree.  The accepted base
    # bytes are staged only when the runtime tree does not already contain the
    # exact accepted asset, and every pre-existing byte is restored below.
    Copy-Item -LiteralPath $baseTrackedAsset -Destination $baseDataAsset -Force
    Copy-Item -LiteralPath $anim -Destination $gripDataAsset -Force
    Invoke-Checked 'Decode accepted base runtime animation' $getAnimation @(
        '-runtime-rig', $baseLogical, $baseRuntimePrefix
    ) (Join-Path $root 'bin')
    Invoke-Checked 'Decode derived grip runtime animation' $getAnimation @(
        '-runtime-rig', $logical, $gripRuntimePrefix
    ) (Join-Path $root 'bin')
    Invoke-Checked 'Prove decoded finger-only grip delta' 'py' @(
        '-3.14', (Join-Path $root 'agent\animation\verify_issue36_rest_basis_grip.py'),
        '--base-runtime-report', $baseRuntimeJson,
        '--grip-runtime-report', $gripRuntimeJson,
        '--math-report', $math,
        '--output-json', $proofJson
    )
}
finally {
    Restore-StagedFile $baseDataAsset $baseDataExisted $baseBackup
    Restore-StagedFile $gripDataAsset $gripDataExisted $gripBackup
}

if ($Promote) {
    New-Item -ItemType Directory -Path (Split-Path -Parent $trackedAsset) -Force | Out-Null
    Copy-Item -LiteralPath $anim -Destination $trackedAsset -Force
    $derivedHash = Get-Sha256 $trackedAsset
    Update-Manifest $manifestPath $trackedAsset $sourceFbxPath $sourceBlendPath $baseRuntimeHash $derivedHash

    $powerShell = (Get-Process -Id $PID).Path
    Invoke-Checked 'Install promoted private WebSwing animation library' $powerShell @(
        '-NoProfile', '-File', (Join-Path $root 'agent\install-webswing-animation.ps1'),
        '-Action', 'Install'
    )
    Invoke-Checked 'Decode installed accepted base runtime animation' $getAnimation @(
        '-runtime-rig', $baseLogical, $installedBasePrefix
    ) (Join-Path $root 'bin')
    Invoke-Checked 'Decode installed derived grip runtime animation' $getAnimation @(
        '-runtime-rig', $logical, $installedGripPrefix
    ) (Join-Path $root 'bin')
    Invoke-Checked 'Prove installed finger-only grip delta' 'py' @(
        '-3.14', (Join-Path $root 'agent\animation\verify_issue36_rest_basis_grip.py'),
        '--base-runtime-report', $installedBaseJson,
        '--grip-runtime-report', $installedGripJson,
        '--math-report', $math,
        '--output-json', $installedProofJson
    )
}

$finalProofJson = if ($Promote) { $installedProofJson } else { $proofJson }
$proof = Get-Content -Raw -LiteralPath $finalProofJson | ConvertFrom-Json
if (-not $proof.passed) { throw "Grip proof failed: $finalProofJson" }

[pscustomobject]@{
    passed = [bool]$proof.passed
    asset = $logical
    authoredFrameRange = '1..60'
    modifiedFrameRange = '16..30'
    changedBones = $proof.changedBones
    changedFrames = $proof.changedFrames
    maxRotationDeltaDegrees = $proof.maxRotationDeltaDegrees
    maxTranslationDelta = $proof.maxTranslationDelta
    blend = $blend
    animx = $animx
    runtimeAnimation = $anim
    runtimeProof = $finalProofJson
    promoted = [bool]$Promote
    acceptedBaseSha256 = $baseRuntimeHash
    derivedSha256 = if (Test-Path -LiteralPath $trackedAsset -PathType Leaf) { Get-Sha256 $trackedAsset } else { Get-Sha256 $anim }
} | ConvertTo-Json -Depth 6
