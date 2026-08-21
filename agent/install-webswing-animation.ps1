[CmdletBinding()]
param(
    [ValidateSet('Status', 'Install', 'Remove')]
    [string]$Action = 'Status',
    [string]$PlayerSourcePath,
    [string]$RepositoryRoot = (Split-Path -Parent $PSScriptRoot)
)

$ErrorActionPreference = 'Stop'

$root = (Resolve-Path -LiteralPath $RepositoryRoot).Path
$animationRoot = Join-Path $root 'agent\webswing-animation'
$trackedInclude = Join-Path $animationRoot 'webswing.inc'
$trackedStateBits = Join-Path $animationRoot 'webswing.statebits'
$runtimeRoot = Join-Path $root 'bin\data\sequencers'
$runtimeInclude = Join-Path $runtimeRoot 'cohsourcedev_webswing.inc'
$runtimeStateBits = Join-Path $runtimeRoot 'cohsourcedev_webswing.statebits'
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

function Get-Status {
    $marker = Test-Path -LiteralPath $runtimePlayer -PathType Leaf
    $content = if ($marker) { [System.IO.File]::ReadAllText($runtimePlayer) } else { '' }
    $playerSourceFormat = 'missing'
    if ($marker) {
        try {
            $materialized = Convert-PlayerDumpToNativeSource -Text $content -SourcePath $runtimePlayer
            $playerSourceFormat = if ($materialized.Changed) { 'runtime-dump' } else { 'native' }
        }
        catch {
            $playerSourceFormat = 'invalid'
        }
    }
    [pscustomobject]@{
        installed = ($content.Contains($sentinelBegin) -and $content.Contains($includeLine) -and $content.Contains($sentinelEnd))
        playerPath = $runtimePlayer
        playerPresent = (Test-Path -LiteralPath $runtimePlayer -PathType Leaf)
        playerSourceFormat = $playerSourceFormat
        rawRecordLinesRemoved = if ($marker -and $playerSourceFormat -eq 'runtime-dump') {
            try { (Convert-PlayerDumpToNativeSource -Text $content -SourcePath $runtimePlayer).RawRecordLinesRemoved } catch { 0 }
        } else { 0 }
        backupPresent = (Test-Path -LiteralPath $backupPlayer -PathType Leaf)
        includePresent = (Test-Path -LiteralPath $runtimeInclude -PathType Leaf)
        stateBitsPresent = (Test-Path -LiteralPath $runtimeStateBits -PathType Leaf)
        includeSha256 = Get-Sha256 $runtimeInclude
        stateBitsSha256 = Get-Sha256 $runtimeStateBits
        trackedIncludeSha256 = Get-Sha256 $trackedInclude
        trackedStateBitsSha256 = Get-Sha256 $trackedStateBits
    } | ConvertTo-Json -Depth 3
}

if ($Action -eq 'Status') {
    Get-Status
    exit 0
}

if (-not (Test-Path -LiteralPath $trackedInclude -PathType Leaf) -or
    -not (Test-Path -LiteralPath $trackedStateBits -PathType Leaf)) {
    throw 'Tracked Web Swing animation data is incomplete.'
}

if (-not (Test-Path -LiteralPath $runtimeRoot -PathType Container)) {
    New-Item -ItemType Directory -Path $runtimeRoot -Force | Out-Null
}

if ($Action -eq 'Install') {
    $playerSource = Find-RawPlayerSource -ExplicitPath $PlayerSourcePath
    $sourceText = [System.IO.File]::ReadAllText($playerSource)
    $materializedSource = Convert-PlayerDumpToNativeSource -Text $sourceText -SourcePath $playerSource
    $hasSentinel = $sourceText.Contains($sentinelBegin) -and $sourceText.Contains($sentinelEnd)
    $alreadyInstalled = $hasSentinel -and ($sourceText.Contains($includeLine) -or $sourceText.Contains($legacyIncludeLine))

    if ($alreadyInstalled -and -not $sourceText.Contains($includeLine)) {
        $sourceText = $sourceText.Replace($legacyIncludeLine, $includeLine)
        $materializedSource = Convert-PlayerDumpToNativeSource -Text $sourceText -SourcePath $playerSource
        $sourceText = $materializedSource.Text
        Write-Utf8NoBom $runtimePlayer $sourceText
    }
    elseif ($alreadyInstalled -and $materializedSource.Changed) {
        Write-Utf8NoBom $runtimePlayer $materializedSource.Text
    }
    elseif (-not $alreadyInstalled) {
        $sourceText = $materializedSource.Text
        if ($sourceText -notmatch '(?m)^\s*SeqEnd\s*$') {
            throw "The resolved player source has no SeqEnd marker: $playerSource"
        }
        if ((Test-Path -LiteralPath $runtimePlayer -PathType Leaf) -and
            ((Resolve-Path -LiteralPath $playerSource).Path -ne (Resolve-Path -LiteralPath $runtimePlayer).Path)) {
            throw "Loose player source already exists without the Web Swing sentinel; refusing to overwrite it: $runtimePlayer"
        }
        $sourceText = [regex]::Replace($sourceText, '(?m)^\s*SeqEnd\s*$', "$sentinelBegin`r`n$includeLine`r`n$sentinelEnd`r`nSeqEnd", 1)
        # Keep the exact extracted source as the protected restore copy.  On
        # removal it is materialized only for comparison, then restored intact.
        Copy-Item -LiteralPath $playerSource -Destination $backupPlayer -Force
        Write-Utf8NoBom $runtimePlayer $sourceText
    }
    elseif (-not (Test-Path -LiteralPath $backupPlayer -PathType Leaf)) {
        throw "The Web Swing sentinel is present but its protected player backup is missing: $backupPlayer"
    }

    Copy-Item -LiteralPath $trackedInclude -Destination $runtimeInclude -Force
    Copy-Item -LiteralPath $trackedStateBits -Destination $runtimeStateBits -Force
    Get-Status
    exit 0
}

if ($Action -eq 'Remove') {
    $currentIncludeHash = Get-Sha256 $runtimeInclude
    $currentStateBitsHash = Get-Sha256 $runtimeStateBits
    if ($currentIncludeHash -and $currentIncludeHash -ne (Get-Sha256 $trackedInclude)) {
        throw "Refusing to remove modified runtime include: $runtimeInclude"
    }
    if ($currentStateBitsHash -and $currentStateBitsHash -ne (Get-Sha256 $trackedStateBits)) {
        throw "Refusing to remove modified runtime state bits: $runtimeStateBits"
    }

    if (Test-Path -LiteralPath $runtimePlayer -PathType Leaf) {
        $currentText = [System.IO.File]::ReadAllText($runtimePlayer)
        if ($currentText.Contains($sentinelBegin) -or $currentText.Contains($sentinelEnd) -or $currentText.Contains($includeLine)) {
            if (-not (Test-Path -LiteralPath $backupPlayer -PathType Leaf)) {
                throw "Refusing to remove an unprotected Web Swing player override: $runtimePlayer"
            }
            $backupText = [System.IO.File]::ReadAllText($backupPlayer)
            $materializedBackup = Convert-PlayerDumpToNativeSource -Text $backupText -SourcePath $backupPlayer
            $expectedInstalled = [regex]::Replace($materializedBackup.Text, '(?m)^\s*SeqEnd\s*$', "$sentinelBegin`r`n$includeLine`r`n$sentinelEnd`r`nSeqEnd", 1)
            if ($currentText -cne $expectedInstalled) {
                throw "The loose player source changed after install; refusing to restore over edits: $runtimePlayer"
            }
            Copy-Item -LiteralPath $backupPlayer -Destination $runtimePlayer -Force
        }
    }

    foreach ($path in @($runtimeInclude, $runtimeStateBits, $backupPlayer)) {
        if (Test-Path -LiteralPath $path -PathType Leaf) {
            Remove-Item -LiteralPath $path -Force
        }
    }
    Get-Status
}
