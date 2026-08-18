[CmdletBinding()]
param(
    [ValidateSet('Generate', 'Install', 'Remove', 'Restore')]
    [string]$Action = 'Generate',
    [string]$ManifestPath,
    [string]$StockRoot,
    [string]$StockBase,
    [string]$StockNormalGloss,
    [string]$OutputRoot = (Join-Path ([System.IO.Path]::GetTempPath()) 'coh-issue20-texture-pilot'),
    [string]$RepoRoot = (Split-Path -Parent $PSScriptRoot),
    [string]$GetTexPath = (Join-Path (Split-Path -Parent $PSScriptRoot) 'Utilities\GetTex\bin\x86\Release\GetTex.exe')
)

$python = Get-Command python.exe -ErrorAction SilentlyContinue
if (-not $python) {
    throw 'Python 3 with Pillow and NumPy is required for the two-file pilot wrapper.'
}

$scriptPath = Join-Path $PSScriptRoot 'texture_pilot.py'
$arguments = @(
    $scriptPath,
    $Action.ToLowerInvariant(),
    '--output-root', $OutputRoot,
    '--repo-root', $RepoRoot
)

if ($ManifestPath) {
    $arguments += @('--manifest', $ManifestPath)
    if ($StockRoot) {
        $arguments += @('--stock-root', $StockRoot)
    }
}

if ($Action -eq 'Generate' -and -not $ManifestPath) {
    if (-not $StockBase -or -not $StockNormalGloss) {
        throw 'Generate requires -StockBase and -StockNormalGloss pointing to locally extracted stock .texture files.'
    }
    $arguments += @('--stock-base', $StockBase, '--stock-normal-gloss', $StockNormalGloss, '--gettex', $GetTexPath)
}

if ($Action -eq 'Generate' -and $ManifestPath -and -not $StockRoot) {
    throw 'Manifest Generate requires -StockRoot pointing to locally extracted stock .texture files.'
}

if ($Action -eq 'Generate' -and $ManifestPath) {
    $arguments += @('--gettex', $GetTexPath)
}

& $python.Source @arguments
exit $LASTEXITCODE
