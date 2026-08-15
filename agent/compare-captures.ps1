[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$Baseline,
    [Parameter(Mandatory=$true)][string]$Current,
    # A pixel counts as changed when any channel differs by more than this (0-255)
    [int]$PixelTolerance = 12,
    # Percent of sampled pixels allowed to change before the verdict fails
    [double]$MaxChangedPercent = 2.0,
    # Average per-channel delta allowed before the verdict fails
    [double]$MaxMeanDelta = 2.0,
    # Images are compared downsampled to this width for speed; JPEG noise
    # averages out and genuine scene changes do not.
    [int]$CompareWidth = 320,
    [switch]$Json
)
$ErrorActionPreference = 'Stop'

function Get-ImageBytes {
    param([string]$Path, [int]$TargetWidth)
    $bmp = [System.Drawing.Bitmap]::FromFile($Path)
    try {
        $w = $TargetWidth
        $h = [int]([Math]::Max(1, [Math]::Round($bmp.Height * ($TargetWidth / [double]$bmp.Width))))
        $scaled = New-Object System.Drawing.Bitmap($w, $h)
        try {
            $g = [System.Drawing.Graphics]::FromImage($scaled)
            try {
                $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::Bilinear
                $g.DrawImage($bmp, 0, 0, $w, $h)
            } finally { $g.Dispose() }
            $rect = New-Object System.Drawing.Rectangle(0, 0, $w, $h)
            $data = $scaled.LockBits($rect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly, [System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
            try {
                $bytes = New-Object byte[] ($data.Stride * $h)
                [System.Runtime.InteropServices.Marshal]::Copy($data.Scan0, $bytes, 0, $bytes.Length)
                return @{ bytes=$bytes; stride=$data.Stride; width=$w; height=$h }
            } finally { $scaled.UnlockBits($data) }
        } finally { $scaled.Dispose() }
    } finally { $bmp.Dispose() }
}

try {
    foreach ($p in @($Baseline, $Current)) {
        if (-not (Test-Path -LiteralPath $p)) { throw "Image not found: $p" }
    }
    Add-Type -AssemblyName System.Drawing

    $a = Get-ImageBytes -Path $Baseline -TargetWidth $CompareWidth
    $b = Get-ImageBytes -Path $Current -TargetWidth $CompareWidth
    if ($a.width -ne $b.width -or $a.height -ne $b.height) {
        throw "Downsampled dimensions differ: $($a.width)x$($a.height) vs $($b.width)x$($b.height)"
    }

    $totalPixels = 0
    $changedPixels = 0
    $deltaSum = 0.0
    $maxDelta = 0
    for ($y = 0; $y -lt $a.height; $y++) {
        $rowA = $y * $a.stride
        $rowB = $y * $b.stride
        for ($x = 0; $x -lt $a.width; $x++) {
            $pixA = $rowA + 3 * $x
            $pixB = $rowB + 3 * $x
            $pixDelta = 0
            for ($c = 0; $c -lt 3; $c++) {
                $d = [Math]::Abs($a.bytes[$pixA + $c] - $b.bytes[$pixB + $c])
                if ($d -gt $pixDelta) { $pixDelta = $d }
            }
            $totalPixels++
            $deltaSum += $pixDelta
            if ($pixDelta -gt $maxDelta) { $maxDelta = $pixDelta }
            if ($pixDelta -gt $PixelTolerance) { $changedPixels++ }
        }
    }

    $changedPercent = if ($totalPixels -gt 0) { 100.0 * $changedPixels / $totalPixels } else { 0.0 }
    $meanDelta = if ($totalPixels -gt 0) { $deltaSum / $totalPixels } else { 0.0 }
    $passed = ($changedPercent -le $MaxChangedPercent) -and ($meanDelta -le $MaxMeanDelta)

    $result = [ordered]@{
        passed   = $passed
        baseline = (Resolve-Path -LiteralPath $Baseline).Path
        current  = (Resolve-Path -LiteralPath $Current).Path
        metrics  = [ordered]@{
            compareWidth    = $CompareWidth
            sampledPixels   = $totalPixels
            changedPixels   = $changedPixels
            changedPercent  = [Math]::Round($changedPercent, 4)
            meanDelta       = [Math]::Round($meanDelta, 4)
            maxDelta        = $maxDelta
        }
        thresholds = [ordered]@{
            pixelTolerance    = $PixelTolerance
            maxChangedPercent = $MaxChangedPercent
            maxMeanDelta      = $MaxMeanDelta
        }
    }
} catch {
    $result = [ordered]@{
        passed = $false
        error  = $_.Exception.Message
        baseline = $Baseline
        current  = $Current
    }
}

if ($Json) {
    $result | ConvertTo-Json -Depth 4
} else {
    if ($result.error) {
        Write-Host "COMPARE ERROR - $($result.error)"
    } elseif ($result.passed) {
        Write-Host ("COMPARE PASS - changed {0}% mean {1} max {2}" -f $result.metrics.changedPercent, $result.metrics.meanDelta, $result.metrics.maxDelta)
    } else {
        Write-Host ("COMPARE FAIL - changed {0}% mean {1} max {2} (limits {3}% / {4})" -f $result.metrics.changedPercent, $result.metrics.meanDelta, $result.metrics.maxDelta, $MaxChangedPercent, $MaxMeanDelta)
    }
}
if ($result.passed) { exit 0 } else { exit 1 }
