[CmdletBinding()]
param(
    [ValidateSet('Status', 'Install', 'Remove')]
    [string]$Action = 'Status',
    [string]$PlayerSourcePath,
    [string]$RepositoryRoot,
    [switch]$IncludeCanary,
    [string]$CanaryAnimationPath
)

$ErrorActionPreference = 'Stop'

$root = if ($RepositoryRoot) {
    (Resolve-Path -LiteralPath $RepositoryRoot).Path
} else {
    (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
}
$animationRoot = Join-Path $root 'agent\webswing-animation'
$trackedInclude = Join-Path $animationRoot 'webswing.inc'
$trackedOverlay = Join-Path $animationRoot 'webswing.txt'
$trackedStateBits = Join-Path $animationRoot 'webswing.statebits'
$trackedCanaryOverlay = Join-Path $animationRoot 'webswing-canary.txt'
$trackedCanaryStateBits = Join-Path $animationRoot 'webswing-canary.statebits'
$trackedCanaryInclude = Join-Path $root 'agent\animation\canary-sequencer.inc'
$trackedCanaryAnimation = Join-Path $root 'agent\animation\runtime\player_library\animations\male\COHSOURCEDEV_RETARGET_SWING_FULL.anim'
$expectedCanaryAnimation = $trackedCanaryAnimation
$trackedAnimationRoot = Join-Path $root 'agent\animation\runtime\player_library\animations'
$trackedAnimationManifest = Join-Path $root 'agent\animation\runtime\webswing-animations.json'
$runtimeRoot = Join-Path $root 'bin\data\sequencers'
$runtimeDataRoot = Join-Path $root 'bin\data'
$runtimeAnimationRoot = Join-Path $runtimeDataRoot 'player_library\animations'
$runtimeInclude = Join-Path $runtimeRoot 'cohsourcedev_webswing.inc'
$runtimeOverlay = Join-Path $runtimeRoot 'cohsourcedev_webswing.txt'
$runtimeCanaryInclude = Join-Path $runtimeRoot 'cohsourcedev_canary.inc'
$runtimeStateBits = Join-Path $runtimeDataRoot 'cohsourcedev_webswing.statebits'
$runtimePlayer = Join-Path $runtimeRoot 'player.txt'
$backupPlayer = Join-Path $runtimeRoot 'player.txt.cohsourcedev-webswing.bak'
$sentinelBegin = '// BEGIN COHSOURCEDEV WEBSWING ANIMATION'
$sentinelEnd = '// END COHSOURCEDEV WEBSWING ANIMATION'
$includeLine = 'include sequencers/cohsourcedev_webswing.inc'
$legacyIncludeLine = 'include cohsourcedev_webswing.inc'

function Get-Sha256([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) { return $null }
    $sha256 = [System.Security.Cryptography.SHA256]::Create()
    try {
        return ([System.BitConverter]::ToString($sha256.ComputeHash([System.IO.File]::ReadAllBytes($Path)))).Replace('-', '').ToLowerInvariant()
    }
    finally {
        $sha256.Dispose()
    }
}

function Write-Utf8NoBom([string]$Path, [string]$Text) {
    $encoding = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Text, $encoding)
}

function Get-AnimationManifest {
    if (-not (Test-Path -LiteralPath $trackedAnimationManifest -PathType Leaf)) {
        throw "Tracked Web Swing animation manifest is missing: $trackedAnimationManifest"
    }
    try {
        $manifest = Get-Content -Raw -LiteralPath $trackedAnimationManifest | ConvertFrom-Json
    }
    catch {
        throw "Tracked Web Swing animation manifest is invalid: $trackedAnimationManifest ($($_.Exception.Message))"
    }
    $entries = @($manifest.animations)
    if ($entries.Count -ne 14) {
        throw "Expected exactly 14 tracked Web Swing animation entries, found $($entries.Count)."
    }
    return $entries
}

function Get-AnimationAssetState {
    $entries = @(Get-AnimationManifest)
    $details = @()
    foreach ($entry in $entries) {
        if ([string]::IsNullOrWhiteSpace($entry.file) -or
            [string]::IsNullOrWhiteSpace($entry.type) -or
            [string]::IsNullOrWhiteSpace($entry.logical) -or
            [string]::IsNullOrWhiteSpace($entry.sha256)) {
            throw 'Tracked Web Swing animation manifest contains an incomplete entry.'
        }
        $relative = [string]$entry.file
        if ([System.IO.Path]::IsPathRooted($relative) -or $relative.Contains('..')) {
            throw "Tracked Web Swing animation manifest contains an unsafe path: $relative"
        }
        $source = Join-Path $trackedAnimationRoot $relative
        $runtime = Join-Path $runtimeAnimationRoot $relative
        $sourceHash = Get-Sha256 $source
        $runtimeHash = Get-Sha256 $runtime
        $expectedHash = ([string]$entry.sha256).ToLowerInvariant()
        $details += [pscustomobject]@{
            type = [string]$entry.type
            logical = [string]$entry.logical
            frames = [int]$entry.frames
            file = $relative
            sourcePath = $source
            runtimePath = $runtime
            expectedSha256 = $expectedHash
            sourceSha256 = $sourceHash
            runtimeSha256 = $runtimeHash
            sourceValid = [bool]($sourceHash -and $sourceHash -eq $expectedHash)
            runtimeValid = [bool]($runtimeHash -and $runtimeHash -eq $expectedHash)
        }
    }
    [pscustomobject]@{
        entries = @($details)
        sourceValid = (@($details | Where-Object { -not $_.sourceValid }).Count -eq 0)
        runtimeValid = (@($details | Where-Object { -not $_.runtimeValid }).Count -eq 0)
    }
}

function Remove-CanaryRuntimeInclude {
    if (-not (Test-Path -LiteralPath $runtimeCanaryInclude -PathType Leaf)) { return }
    $currentHash = Get-Sha256 $runtimeCanaryInclude
    $trackedHash = Get-Sha256 $trackedCanaryInclude
    if ($currentHash -ne $trackedHash) {
        throw "Refusing to remove modified runtime canary include: $runtimeCanaryInclude"
    }
    Remove-Item -LiteralPath $runtimeCanaryInclude -Force
}

function Copy-AnimationAssets {
    $assetState = Get-AnimationAssetState
    if (-not $assetState.sourceValid) {
        $bad = @($assetState.entries | Where-Object { -not $_.sourceValid } | ForEach-Object { $_.file }) -join ', '
        throw "Tracked Web Swing animation assets are missing or corrupt: $bad"
    }
    foreach ($entry in $assetState.entries) {
        $destinationDirectory = Split-Path -Parent $entry.runtimePath
        if (-not (Test-Path -LiteralPath $destinationDirectory -PathType Container)) {
            New-Item -ItemType Directory -Path $destinationDirectory -Force | Out-Null
        }
        Copy-Item -LiteralPath $entry.sourcePath -Destination $entry.runtimePath -Force
    }
    $installed = Get-AnimationAssetState
    if (-not $installed.runtimeValid) {
        $bad = @($installed.entries | Where-Object { -not $_.runtimeValid } | ForEach-Object { $_.file }) -join ', '
        throw "Runtime Web Swing animation assets failed hash validation: $bad"
    }
    return $installed
}

function Remove-AnimationAssets {
    $assetState = Get-AnimationAssetState
    foreach ($entry in $assetState.entries) {
        if (-not (Test-Path -LiteralPath $entry.runtimePath -PathType Leaf)) { continue }
        if (-not $entry.runtimeValid) {
            throw "Refusing to remove modified runtime Web Swing animation: $($entry.runtimePath)"
        }
        Remove-Item -LiteralPath $entry.runtimePath -Force
    }
}

function Convert-PlayerDumpToNativeSource {
    param(
        [Parameter(Mandatory = $true)][string]$Text,
        [Parameter(Mandatory = $true)][string]$SourcePath
    )

    # The runtime extraction used for this branch is ParserWriteText output.
    # ParseMoveP writes its unnamed SeqMoveRaw fields after the authored Flags
    # field and before MEnd.  Those records are numeric-only lines; they are
    # derived bin-time data, not native player source.  Remove only that exact
    # region.  Any other content in the region is an input-format failure.
    $integerLine = '^\s*-?\d+(?:\s*,\s*-?\d+)*\s*$'
    # PowerShell uses 0, not -1, for an unlimited split count.  A -1 count
    # leaves the CRLF dump as one line and silently bypasses all normalization.
    $lines = $Text -split "`r?`n", 0
    $output = New-Object 'System.Collections.Generic.List[string]'
    $inMove = $false
    $rawSection = $false
    $moveName = $null
    $rawRecordCount = 0
    $derivedMoveFlagCount = 0

    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]

        if ($line -match '^\s*Move\s+(.+?)\s*$') {
            if ($inMove) {
                throw "Unexpected nested Move while materializing player source $SourcePath (move '$moveName', line $($i + 1))."
            }
            $inMove = $true
            $rawSection = $false
            $moveName = $Matches[1]
            $output.Add($line)
            continue
        }

        if ($inMove -and $line -match '^\s*MEnd\s*$') {
            $output.Add($line)
            $inMove = $false
            $rawSection = $false
            $moveName = $null
            continue
        }

        if ($inMove -and $line -match '^(\s*Flags\s+)(.*)$') {
            # ParserWriteText preserves derived move flags in the authored
            # Flags line.  They must not be fed back into the source parser:
            # SEQMOVE_COMPLEXCYCLE (16) is recomputed from CycleMove entries,
            # and SEQMOVE_PREDICTABLE (2097152) is recomputed from Requires
            # state bits.  The raw cycle records are removed below, so keeping
            # either value would describe runtime data that is no longer
            # present and can make seqStep divide by a zero cycle count.
            $flagPrefix = $Matches[1]
            $flagText = $Matches[2]
            $flagTokens = @($flagText -split '\s*,\s*' | Where-Object { $_ -and $_ -notmatch '^\s*(?:16|2097152)\s*$' })
            $removedFlags = @($flagText -split '\s*,\s*' | Where-Object { $_ -match '^\s*(?:16|2097152)\s*$' })
            if ($removedFlags.Count -gt 0) {
                $derivedMoveFlagCount += $removedFlags.Count
                $line = $flagPrefix + ($flagTokens -join ', ')
            }
        }

        if ($inMove -and $rawSection) {
            if ([string]::IsNullOrWhiteSpace($line)) {
                $output.Add($line)
                continue
            }
            if ($line -notmatch $integerLine) {
                throw "Unexpected non-numeric content in the ParseMoveP bin-time region of Move '$moveName' while materializing player source $SourcePath (line $($i + 1)). Expected only numeric records before MEnd."
            }
            $rawRecordCount++
            continue
        }

        # ParserWriteText omits default-valued unnamed fields.  Consequently
        # the raw region can begin after Flags, or after the last authored
        # field when Flags itself is omitted.
        if ($inMove -and $line -match $integerLine) {
            $rawSection = $true
            $rawRecordCount++
            continue
        }

        $output.Add($line)
    }

    if ($inMove) {
        throw "Player source ended inside Move '$moveName' without MEnd: $SourcePath"
    }

    [pscustomobject]@{
        Text = ($output -join "`r`n")
        Changed = ($rawRecordCount -gt 0 -or $derivedMoveFlagCount -gt 0)
        RawRecordLinesRemoved = $rawRecordCount
        DerivedMoveFlagsRemoved = $derivedMoveFlagCount
    }
}

function Find-RawPlayerSource {
    param([string]$ExplicitPath)

    if ($ExplicitPath) {
        $resolved = (Resolve-Path -LiteralPath $ExplicitPath -ErrorAction Stop).Path
        if (-not (Test-Path -LiteralPath $resolved -PathType Leaf)) {
            throw "Player source does not exist: $resolved"
        }
        return $resolved
    }

    $looseCandidates = @(
        (Join-Path $root 'bin\data\sequencers\player.txt'),
        (Join-Path $root 'bin\sequencers\player.txt')
    )
    foreach ($candidate in $looseCandidates) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    $pigExeCandidates = @(
        (Join-Path $root 'Utilities\pig\bin\x86\Release\pig.exe'),
        (Join-Path $root 'bin\pig.exe')
    )
    $pigExe = $pigExeCandidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
    $pigg = Join-Path $root 'bin\piggs\bin.pigg'
    if ($pigExe -and (Test-Path -LiteralPath $pigg -PathType Leaf)) {
        $temp = Join-Path ([System.IO.Path]::GetTempPath()) ("coh-webswing-player-{0}" -f $PID)
        New-Item -ItemType Directory -Path $temp -Force | Out-Null
        try {
            Push-Location $temp
            try {
                & $pigExe xy $pigg | Out-Null
            }
            finally {
                Pop-Location
            }
            $extracted = Get-ChildItem -LiteralPath $temp -Recurse -File -Filter 'player.txt' -ErrorAction SilentlyContinue | Select-Object -First 1
            if ($extracted) {
                $stableCopy = Join-Path ([System.IO.Path]::GetTempPath()) ("coh-webswing-player-source-{0}.txt" -f $PID)
                Copy-Item -LiteralPath $extracted.FullName -Destination $stableCopy -Force
                return $stableCopy
            }
        }
        finally {
            if (Test-Path -LiteralPath $temp -PathType Container) {
                Remove-Item -LiteralPath $temp -Recurse -Force
            }
        }
    }

    throw 'No raw sequencers/player.txt was found in the loose data or local piggs. The checked-in piggs contain compiled sequencers.bin only; refusing to synthesize or replace the player sequencer. Supply -PlayerSourcePath with an exact runtime dump/source when available.'
}

function Remove-LegacyPlayerOverride {
    if (-not (Test-Path -LiteralPath $runtimePlayer -PathType Leaf)) { return }

    $currentText = [System.IO.File]::ReadAllText($runtimePlayer)
    $hasLegacy = $currentText.Contains($sentinelBegin) -or
        $currentText.Contains($sentinelEnd) -or
        $currentText.Contains($includeLine) -or
        $currentText.Contains($legacyIncludeLine)
    if (-not $hasLegacy) { return }
    if (-not (Test-Path -LiteralPath $backupPlayer -PathType Leaf)) {
        throw "Refusing to overwrite an unprotected legacy Web Swing player override: $runtimePlayer"
    }

    $backupText = [System.IO.File]::ReadAllText($backupPlayer)
    $materializedBackup = Convert-PlayerDumpToNativeSource -Text $backupText -SourcePath $backupPlayer
    $expectedInstalled = [regex]::Replace($materializedBackup.Text, '(?m)^\s*SeqEnd\s*$', "$sentinelBegin`r`n$includeLine`r`n$sentinelEnd`r`nSeqEnd", 1)
    $legacyMatches = $currentText -ceq $expectedInstalled -or
        $currentText -ceq $expectedInstalled.Replace($includeLine, $legacyIncludeLine)
    if (-not $legacyMatches) {
        throw "The legacy Web Swing player source changed after install; refusing to restore over edits: $runtimePlayer"
    }

    Copy-Item -LiteralPath $backupPlayer -Destination $runtimePlayer -Force
    Remove-Item -LiteralPath $backupPlayer -Force
}

function Get-Status {
    $marker = Test-Path -LiteralPath $runtimePlayer -PathType Leaf
    $content = if ($marker) { [System.IO.File]::ReadAllText($runtimePlayer) } else { '' }
    $playerSourceFormat = 'not-used'
    if ($marker) {
        try {
            $materialized = Convert-PlayerDumpToNativeSource -Text $content -SourcePath $runtimePlayer
            $playerSourceFormat = if ($materialized.Changed) { 'runtime-dump' } else { 'native' }
        }
        catch {
            $playerSourceFormat = 'invalid'
        }
    }
    $animationState = Get-AnimationAssetState
    $normalModeInstalled = ((Test-Path -LiteralPath $runtimeOverlay -PathType Leaf) -and
        (Test-Path -LiteralPath $runtimeInclude -PathType Leaf) -and
        (Test-Path -LiteralPath $runtimeStateBits -PathType Leaf) -and
        (Get-Sha256 $runtimeInclude) -eq (Get-Sha256 $trackedInclude) -and
        (Get-Sha256 $runtimeOverlay) -eq (Get-Sha256 $trackedOverlay) -and
        (Get-Sha256 $runtimeStateBits) -eq (Get-Sha256 $trackedStateBits) -and
        $animationState.runtimeValid)
    $canaryAssetPath = Join-Path $runtimeAnimationRoot 'male\COHSOURCEDEV_RETARGET_SWING_FULL.anim'
    $canaryModeInstalled = ((Test-Path -LiteralPath $runtimeOverlay -PathType Leaf) -and
        (Test-Path -LiteralPath $runtimeInclude -PathType Leaf) -and
        (Test-Path -LiteralPath $runtimeCanaryInclude -PathType Leaf) -and
        (Test-Path -LiteralPath $runtimeStateBits -PathType Leaf) -and
        (Get-Sha256 $runtimeInclude) -eq (Get-Sha256 $trackedInclude) -and
        (Get-Sha256 $runtimeOverlay) -eq (Get-Sha256 $trackedCanaryOverlay) -and
        (Get-Sha256 $runtimeStateBits) -eq (Get-Sha256 $trackedCanaryStateBits) -and
        (Get-Sha256 $runtimeCanaryInclude) -eq (Get-Sha256 $trackedCanaryInclude) -and
        $animationState.runtimeValid -and
        (Test-Path -LiteralPath $canaryAssetPath -PathType Leaf) -and
        (Get-Sha256 $canaryAssetPath) -eq (Get-Sha256 $expectedCanaryAnimation))
    [pscustomobject]@{
        installed = if ($IncludeCanary) { $canaryModeInstalled } else { $normalModeInstalled }
        normalModeInstalled = $normalModeInstalled
        canaryModeInstalled = $canaryModeInstalled
        includeCanary = [bool]$IncludeCanary
        legacyPlayerOverride = ($content.Contains($sentinelBegin) -or $content.Contains($sentinelEnd) -or $content.Contains($includeLine) -or $content.Contains($legacyIncludeLine))
        playerPath = $runtimePlayer
        playerPresent = (Test-Path -LiteralPath $runtimePlayer -PathType Leaf)
        playerSourceFormat = $playerSourceFormat
        rawRecordLinesRemoved = if ($marker -and $playerSourceFormat -eq 'runtime-dump') {
            try { (Convert-PlayerDumpToNativeSource -Text $content -SourcePath $runtimePlayer).RawRecordLinesRemoved } catch { 0 }
        } else { 0 }
        backupPresent = (Test-Path -LiteralPath $backupPlayer -PathType Leaf)
        overlayPath = $runtimeOverlay
        overlayPresent = (Test-Path -LiteralPath $runtimeOverlay -PathType Leaf)
        includePresent = (Test-Path -LiteralPath $runtimeInclude -PathType Leaf)
        stateBitsPresent = (Test-Path -LiteralPath $runtimeStateBits -PathType Leaf)
        canaryIncludePresent = (Test-Path -LiteralPath $runtimeCanaryInclude -PathType Leaf)
        canaryAssetPath = $canaryAssetPath
        canaryAssetPresent = (Test-Path -LiteralPath $canaryAssetPath -PathType Leaf)
        canaryAssetSha256 = Get-Sha256 $canaryAssetPath
        trackedCanaryAssetSha256 = Get-Sha256 $expectedCanaryAnimation
        includeSha256 = Get-Sha256 $runtimeInclude
        overlaySha256 = Get-Sha256 $runtimeOverlay
        stateBitsSha256 = Get-Sha256 $runtimeStateBits
        canaryIncludeSha256 = Get-Sha256 $runtimeCanaryInclude
        trackedIncludeSha256 = Get-Sha256 $trackedInclude
        trackedOverlaySha256 = Get-Sha256 $trackedOverlay
        trackedStateBitsSha256 = Get-Sha256 $trackedStateBits
        trackedCanaryIncludeSha256 = Get-Sha256 $trackedCanaryInclude
        trackedCanaryOverlaySha256 = Get-Sha256 $trackedCanaryOverlay
        trackedCanaryStateBitsSha256 = Get-Sha256 $trackedCanaryStateBits
        animationAssets = $animationState.entries
        animationAssetsSourceValid = $animationState.sourceValid
        animationAssetsRuntimeValid = $animationState.runtimeValid
    } | ConvertTo-Json -Depth 5
}

if ($Action -eq 'Status') {
    Get-Status
    exit 0
}

if (-not (Test-Path -LiteralPath $trackedOverlay -PathType Leaf) -or
    -not (Test-Path -LiteralPath $trackedInclude -PathType Leaf) -or
    -not (Test-Path -LiteralPath $trackedStateBits -PathType Leaf) -or
    -not (Test-Path -LiteralPath $trackedCanaryOverlay -PathType Leaf) -or
    -not (Test-Path -LiteralPath $trackedCanaryStateBits -PathType Leaf) -or
    -not (Test-Path -LiteralPath $trackedCanaryInclude -PathType Leaf) -or
    -not (Test-Path -LiteralPath $trackedCanaryAnimation -PathType Leaf)) {
    throw 'Tracked Web Swing animation data is incomplete.'
}

if ($IncludeCanary) {
    $canaryRuntimeAsset = Join-Path $runtimeAnimationRoot 'male\COHSOURCEDEV_RETARGET_SWING_FULL.anim'
    $resolvedCanary = if ($CanaryAnimationPath) {
        (Resolve-Path -LiteralPath $CanaryAnimationPath -ErrorAction Stop).Path
    } else {
        $trackedCanaryAnimation
    }
    if (-not (Test-Path -LiteralPath $resolvedCanary -PathType Leaf)) {
        throw "Canary animation asset does not exist: $resolvedCanary"
    }
    $expectedCanaryAnimation = $resolvedCanary
}

if (-not (Test-Path -LiteralPath $runtimeRoot -PathType Container)) {
    New-Item -ItemType Directory -Path $runtimeRoot -Force | Out-Null
}
if (-not (Test-Path -LiteralPath $runtimeDataRoot -PathType Container)) {
    New-Item -ItemType Directory -Path $runtimeDataRoot -Force | Out-Null
}

if ($Action -eq 'Install') {
    Remove-LegacyPlayerOverride
    Copy-Item -LiteralPath $trackedOverlay -Destination $runtimeOverlay -Force
    Copy-Item -LiteralPath $trackedInclude -Destination $runtimeInclude -Force
    Copy-Item -LiteralPath $trackedStateBits -Destination $runtimeStateBits -Force
    if ($IncludeCanary) {
        Copy-Item -LiteralPath $trackedCanaryOverlay -Destination $runtimeOverlay -Force
        Copy-Item -LiteralPath $trackedCanaryStateBits -Destination $runtimeStateBits -Force
        Copy-Item -LiteralPath $trackedCanaryInclude -Destination $runtimeCanaryInclude -Force
    } else {
        Remove-CanaryRuntimeInclude
    }
    Copy-AnimationAssets | Out-Null
    if ($IncludeCanary -and $CanaryAnimationPath) {
        Copy-Item -LiteralPath $resolvedCanary -Destination $canaryRuntimeAsset -Force
    }
    Get-Status
    exit 0
}

if ($Action -eq 'Remove') {
    $currentIncludeHash = Get-Sha256 $runtimeInclude
    $currentOverlayHash = Get-Sha256 $runtimeOverlay
    $currentCanaryIncludeHash = Get-Sha256 $runtimeCanaryInclude
    $currentStateBitsHash = Get-Sha256 $runtimeStateBits
    if ($currentIncludeHash -and $currentIncludeHash -ne (Get-Sha256 $trackedInclude)) {
        throw "Refusing to remove modified runtime include: $runtimeInclude"
    }
    if ($currentStateBitsHash -and
        $currentStateBitsHash -ne (Get-Sha256 $trackedStateBits) -and
        $currentStateBitsHash -ne (Get-Sha256 $trackedCanaryStateBits)) {
        throw "Refusing to remove modified runtime state bits: $runtimeStateBits"
    }
    if ($currentOverlayHash -and
        $currentOverlayHash -ne (Get-Sha256 $trackedOverlay) -and
        $currentOverlayHash -ne (Get-Sha256 $trackedCanaryOverlay)) {
        throw "Refusing to remove modified runtime overlay: $runtimeOverlay"
    }
    if ($currentCanaryIncludeHash -and $currentCanaryIncludeHash -ne (Get-Sha256 $trackedCanaryInclude)) {
        throw "Refusing to remove modified runtime canary include: $runtimeCanaryInclude"
    }

    Remove-LegacyPlayerOverride
    Remove-AnimationAssets
    foreach ($path in @($runtimeOverlay, $runtimeInclude, $runtimeCanaryInclude, $runtimeStateBits)) {
        if (Test-Path -LiteralPath $path -PathType Leaf) {
            Remove-Item -LiteralPath $path -Force
        }
    }
    Get-Status
}
