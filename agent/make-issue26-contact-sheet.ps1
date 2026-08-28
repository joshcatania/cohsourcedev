[CmdletBinding()]
param(
    [string]$EvidenceRoot = (Join-Path $PSScriptRoot '..\docs\evidence\issue26-atlas-remaster'),
    [string]$OutputPath = (Join-Path $PSScriptRoot '..\docs\evidence\issue26-atlas-remaster-v1\contact-sheet.jpg'),
    [string]$CapturePrefix = 'atlas-final'
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
    $columns = 2
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
    @{ Label = 'City Hall'; File = 'AtlasHero_CityHall_01.jpg'; Slug = 'AtlasHero_CityHall' },
    @{ Label = 'East'; File = 'AtlasHero_East_01.jpg'; Slug = 'AtlasHero_East' },
    @{ Label = 'North'; File = 'AtlasHero_North_01.jpg'; Slug = 'AtlasHero_North' },
    @{ Label = 'West'; File = 'AtlasHero_West_01.jpg'; Slug = 'AtlasHero_West' }
)
$tiles = foreach ($view in $views) {
    $stock = Join-Path $EvidenceRoot ("{0}-{1}-stock-ultra\{2}" -f $CapturePrefix, $view.Slug.Replace('AtlasHero_', ''), $view.File)
    $modern = Join-Path $EvidenceRoot ("{0}-{1}-modern-lighting-v1\{2}" -f $CapturePrefix, $view.Slug.Replace('AtlasHero_', ''), $view.File)
    foreach ($item in @(
        @{ Label = "$($view.Label) — Stock Ultra"; Path = $stock },
        @{ Label = "$($view.Label) — Modern Lighting v1"; Path = $modern }
    )) {
        if (-not (Test-Path -LiteralPath $item.Path)) { throw "Missing comparison image: $($item.Path)" }
        [pscustomobject]@{ Label = $item.Label; Path = (Resolve-Path -LiteralPath $item.Path).Path }
    }
}

$outputDirectory = Split-Path -Parent $OutputPath
New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
New-ContactSheet -OutputFile $OutputPath -Tiles $tiles
Write-Host "Wrote $OutputPath"
