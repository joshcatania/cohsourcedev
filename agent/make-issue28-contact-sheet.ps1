[CmdletBinding()]
param(
    [string]$EvidenceRoot = (Join-Path $PSScriptRoot '..\docs\evidence\issue28-atlas-remaster-atmosphere-v1'),
    [string]$OutputPath = (Join-Path $PSScriptRoot '..\docs\evidence\issue28-atlas-remaster-atmosphere-v1\contact-sheet.jpg')
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

function Get-JpegEncoder {
    [System.Drawing.Imaging.ImageCodecInfo]::GetImageEncoders() |
        Where-Object { $_.MimeType -eq 'image/jpeg' } |
        Select-Object -First 1
}

function New-ContactSheet {
    param([string]$OutputFile, [object[]]$Tiles)

    $tileWidth = 640
    $tileHeight = 360
    $margin = 18
    $labelHeight = 30
    $columns = 3
    $rows = [math]::Ceiling($Tiles.Count / $columns)
    $sheet = New-Object System.Drawing.Bitmap(
        ($margin + ($tileWidth + $margin) * $columns),
        ($margin + ($tileHeight + $labelHeight + $margin) * $rows)
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
        for ($index = 0; $index -lt $Tiles.Count; $index++) {
            $tile = $Tiles[$index]
            $column = $index % $columns
            $row = [math]::Floor($index / $columns)
            $x = $margin + $column * ($tileWidth + $margin)
            $y = $margin + $row * ($tileHeight + $labelHeight + $margin)
            $image = [System.Drawing.Image]::FromFile($tile.Path)
            try {
                $graphics.DrawImage($image, (New-Object System.Drawing.Rectangle($x, $y, $tileWidth, $tileHeight)))
            } finally {
                $image.Dispose()
            }
            $graphics.DrawString($tile.Label, $font, $brush, $x, $y + $tileHeight + 5)
        }
        $sheet.Save($OutputFile, $encoder, $encoderParams)
    } finally {
        $encoderParams.Dispose()
        $brush.Dispose()
        $font.Dispose()
        $graphics.Dispose()
        $sheet.Dispose()
    }
}

$views = @(
    @{ Label = 'City Hall'; File = 'AtlasHero_CityHall_01.jpg' },
    @{ Label = 'East'; File = 'AtlasHero_East_01.jpg' },
    @{ Label = 'North'; File = 'AtlasHero_North_01.jpg' },
    @{ Label = 'West'; File = 'AtlasHero_West_01.jpg' }
)
$states = @(
    @{ Label = 'Stock Ultra'; Directory = 'final-stock-ultra' },
    @{ Label = 'Current Remaster (#26)'; Directory = 'final-current-remaster' },
    @{ Label = '#26 + Modern Atmosphere'; Directory = 'final-modern-atmosphere' }
)

$tiles = foreach ($view in $views) {
    foreach ($state in $states) {
        $path = Join-Path $EvidenceRoot (Join-Path $state.Directory $view.File)
        if (-not (Test-Path -LiteralPath $path)) { throw "Missing comparison image: $path" }
        [pscustomobject]@{
            Label = "$($view.Label) — $($state.Label)"
            Path = (Resolve-Path -LiteralPath $path).Path
        }
    }
}

$outputDirectory = Split-Path -Parent $OutputPath
New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
New-ContactSheet -OutputFile $OutputPath -Tiles $tiles
Write-Host "Wrote $OutputPath"
