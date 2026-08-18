[CmdletBinding()]
param(
    [string]$EvidenceRoot = (Join-Path $PSScriptRoot '..\docs\evidence\issue28-atlas-remaster-atmosphere-v1'),
    [string]$OutputPath = (Join-Path $PSScriptRoot '..\docs\evidence\issue28-atlas-remaster-atmosphere-v1\skyline-crop.jpg')
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

function Get-JpegEncoder {
    [System.Drawing.Imaging.ImageCodecInfo]::GetImageEncoders() |
        Where-Object { $_.MimeType -eq 'image/jpeg' } |
        Select-Object -First 1
}

$sources = @(
    [pscustomobject]@{
        Label = 'East — Current Remaster (#26)'
        Path = Join-Path $EvidenceRoot 'final-current-remaster\AtlasHero_East_01.jpg'
    }
    [pscustomobject]@{
        Label = 'East — #26 + Modern Atmosphere'
        Path = Join-Path $EvidenceRoot 'final-modern-atmosphere\AtlasHero_East_01.jpg'
    }
)

foreach ($source in $sources) {
    if (-not (Test-Path -LiteralPath $source.Path)) {
        throw "Missing source image: $($source.Path)"
    }
}

$tileWidth = 640
$tileHeight = 210
$labelHeight = 30
$margin = 18
$sheet = New-Object System.Drawing.Bitmap(
    ($margin + ($tileWidth + $margin) * $sources.Count),
    ($margin + $tileHeight + $labelHeight + $margin)
)
$graphics = [System.Drawing.Graphics]::FromImage($sheet)
$graphics.Clear([System.Drawing.Color]::FromArgb(28, 30, 34))
$graphics.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAlias
$font = New-Object System.Drawing.Font('Segoe UI', 11, [System.Drawing.FontStyle]::Regular)
$brush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White)
$encoder = Get-JpegEncoder
$encoderParams = New-Object System.Drawing.Imaging.EncoderParameters(1)
$encoderParams.Param[0] = New-Object System.Drawing.Imaging.EncoderParameter(
    [System.Drawing.Imaging.Encoder]::Quality, [long]92
)

try {
    for ($index = 0; $index -lt $sources.Count; $index++) {
        $source = $sources[$index]
        $image = [System.Drawing.Image]::FromFile($source.Path)
        try {
            $x = $margin + $index * ($tileWidth + $margin)
            $y = $margin
            $destination = New-Object System.Drawing.Rectangle($x, $y, $tileWidth, $tileHeight)
            $sourceRectangle = New-Object System.Drawing.Rectangle(0, 0, $image.Width, [int]($image.Height * 0.58))
            $graphics.DrawImage($image, $destination, $sourceRectangle, [System.Drawing.GraphicsUnit]::Pixel)
            $graphics.DrawString($source.Label, $font, $brush, $x, $y + $tileHeight + 5)
        } finally {
            $image.Dispose()
        }
    }

    $outputDirectory = Split-Path -Parent $OutputPath
    New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
    $sheet.Save($OutputPath, $encoder, $encoderParams)
} finally {
    $encoderParams.Dispose()
    $brush.Dispose()
    $font.Dispose()
    $graphics.Dispose()
    $sheet.Dispose()
}

Write-Host "Wrote $OutputPath"
