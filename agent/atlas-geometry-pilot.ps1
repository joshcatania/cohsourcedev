[CmdletBinding()]
param(
    [ValidateSet('Status', 'Install', 'Remove', 'Restore')]
    [string]$Action = 'Status',
    [string]$RepoRoot,
    [string]$GeoPath,
    [string]$ExpectedHash,
    [string]$PackedSource = 'bin\piggs\stage3c.pigg',
    [string]$PackedHash,
    [string]$LooseRelativePath = 'bin\data\object_library\City_Zones\Elements\Hero_Statues\Male_Statue_Atlas\Male_Statue_Atlas.geo',
    [switch]$Json
)

$ErrorActionPreference = 'Stop'

if (-not $RepoRoot) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
} else {
    $RepoRoot = (Resolve-Path $RepoRoot).Path
}

$targetPath = [System.IO.Path]::GetFullPath((Join-Path $RepoRoot $LooseRelativePath))
$dataRoot = [System.IO.Path]::GetFullPath((Join-Path $RepoRoot 'bin\data'))
$dataPrefix = $dataRoot.TrimEnd('\') + '\'
if (-not $targetPath.StartsWith($dataPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Loose geometry target escaped bin/data: $targetPath"
}

function Get-Hash([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) { return $null }
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

function Resolve-Geo([string]$Path) {
    if (-not $Path) { throw 'Install requires -GeoPath.' }
    $resolved = (Resolve-Path -LiteralPath $Path).Path
    if (-not (Test-Path -LiteralPath $resolved -PathType Leaf)) { throw "Geometry file is missing: $resolved" }
    return $resolved
}

function Get-Result([string]$State, [string]$InstalledFrom, [string]$InstalledHashValue, [string]$TargetHash, [string]$PackedHashValue) {
    [ordered]@{
        schema = 'coh.atlas-geometry-pilot.v1'
        action = $Action
        state = $State
        target = $targetPath
        targetRelative = $LooseRelativePath.Replace('\', '/')
        targetHash = $TargetHash
        installedFrom = $InstalledFrom
        installedHash = $InstalledHashValue
        packedSource = (Join-Path $RepoRoot $PackedSource)
        packedHash = $PackedHashValue
        reversible = $true
        stockByDefault = ($State -in @('packed-stock', 'removed'))
    }
}

$packedPath = Join-Path $RepoRoot $PackedSource
$currentTargetHash = Get-Hash $targetPath
$currentPackedHash = Get-Hash $packedPath

switch ($Action) {
    'Status' {
        $result = if ($currentTargetHash) {
            Get-Result 'loose-override' $null $null $currentTargetHash $currentPackedHash
        } else {
            Get-Result 'packed-stock' $null $null $null $currentPackedHash
        }
    }

    'Install' {
        $sourcePath = Resolve-Geo $GeoPath
        $sourceHash = Get-Hash $sourcePath
        if ($ExpectedHash -and $sourceHash -ne $ExpectedHash.ToUpperInvariant()) {
            throw "Source hash mismatch: expected $ExpectedHash, got $sourceHash"
        }
        if ($currentTargetHash -and $currentTargetHash -ne $sourceHash) {
            throw "Refusing to overwrite unrelated loose geometry: $targetPath (hash $currentTargetHash)"
        }
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $targetPath) | Out-Null
        [System.IO.File]::Copy($sourcePath, $targetPath, $true)
        $installedTargetHash = Get-Hash $targetPath
        if ($installedTargetHash -ne $sourceHash) {
            throw "Installed geometry hash changed during copy: $installedTargetHash"
        }
        $result = Get-Result 'loose-override' $sourcePath $sourceHash $installedTargetHash $currentPackedHash
    }

    'Remove' {
        if ($currentTargetHash) {
            if (-not $ExpectedHash) {
                throw 'Remove requires -ExpectedHash so an unrelated loose file cannot be deleted.'
            }
            if ($currentTargetHash -ne $ExpectedHash.ToUpperInvariant()) {
                throw "Refusing to remove unrelated loose geometry: $targetPath (hash $currentTargetHash)"
            }
            Remove-Item -LiteralPath $targetPath -Force
        }
        $result = Get-Result 'removed' $null $ExpectedHash $null $currentPackedHash
    }

    'Restore' {
        if ($currentTargetHash) {
            if (-not $ExpectedHash) {
                throw 'Restore requires -ExpectedHash so an unrelated loose file cannot be deleted.'
            }
            if ($currentTargetHash -ne $ExpectedHash.ToUpperInvariant()) {
                throw "Refusing to remove unrelated loose geometry: $targetPath (hash $currentTargetHash)"
            }
            Remove-Item -LiteralPath $targetPath -Force
        }
        if ($PackedHash -and $currentPackedHash -ne $PackedHash.ToUpperInvariant()) {
            throw "Packed geometry source changed: expected $PackedHash, got $currentPackedHash"
        }
        $result = Get-Result 'packed-stock' $null $ExpectedHash $null $currentPackedHash
    }
}

if ($Json) {
    $result | ConvertTo-Json -Depth 5
} else {
    $result | Format-List
}
