param(
    [string]$EvidenceRoot = (Join-Path $PSScriptRoot '..\docs\evidence\issue25-depth-pilot'),
    [string]$OutputRoot = (Join-Path $PSScriptRoot '..\docs\evidence')
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

function Get-JpegEncoder {
    [System.Drawing.Imaging.ImageCodecInfo]::GetImageEncoders() |
        Where-Object { $_.MimeType -eq 'image/jpeg' } |
        Select-Object -First 1
}

function New-ContactSheet {
    param(
        [string]$OutputPath,
        [object[]]$Tiles,
        [int]$TileWidth = 360,
        [int]$TileHeight = 202,
        [int]$Columns = 4
    )

    $margin = 16
    $labelHeight = 28
    $rows = [math]::Ceiling($Tiles.Count / $Columns)
    $sheet = New-Object System.Drawing.Bitmap(
        ($margin + ($TileWidth + $margin) * $Columns),
        ($margin + ($TileHeight + $labelHeight + $margin) * $rows)
    )
    $graphics = [System.Drawing.Graphics]::FromImage($sheet)
    $graphics.Clear([System.Drawing.Color]::FromArgb(28, 30, 34))
    $graphics.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAlias
    $font = New-Object System.Drawing.Font('Segoe UI', 10, [System.Drawing.FontStyle]::Regular)
    $brush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::White)
    $encoder = Get-JpegEncoder
    $encoderParams = New-Object System.Drawing.Imaging.EncoderParameters(1)
    $encoderParams.Param[0] = New-Object System.Drawing.Imaging.EncoderParameter(
        [System.Drawing.Imaging.Encoder]::Quality, [long]88
    )

    try {
        for ($index = 0; $index -lt $Tiles.Count; $index++) {
            $tile = $Tiles[$index]
            $column = $index % $Columns
            $row = [math]::Floor($index / $Columns)
            $x = $margin + $column * ($TileWidth + $margin)
            $y = $margin + $row * ($TileHeight + $labelHeight + $margin)
            $image = [System.Drawing.Image]::FromFile($tile.Path)
            try {
                $graphics.DrawImage($image, (New-Object System.Drawing.Rectangle($x, $y, $TileWidth, $TileHeight)))
            } finally {
                $image.Dispose()
            }
            $graphics.DrawString($tile.Label, $font, $brush, $x, $y + $TileHeight + 5)
        }
        $sheet.Save($OutputPath, $encoder, $encoderParams)
    } finally {
        $encoderParams.Dispose()
        $brush.Dispose()
        $font.Dispose()
        $graphics.Dispose()
        $sheet.Dispose()
    }
}

New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null

$matrixNames = @(
    @{ Label = 'Current'; Path = Join-Path $EvidenceRoot 'matrix-current\AtlasHero_CityHall_01.jpg' },
    @{ Label = 'Shadows only — high / 1024 / middle'; Path = Join-Path $EvidenceRoot 'shadows-high-middle-1024\AtlasHero_CityHall_01.jpg' },
    @{ Label = 'AO only — high / HQ / bilateral depth'; Path = Join-Path $EvidenceRoot 'ao-high-quality-bilateral-depth\AtlasHero_CityHall_01.jpg' },
    @{ Label = 'Combined winner'; Path = Join-Path $EvidenceRoot 'combined-winner-cityhall\AtlasHero_CityHall_01.jpg' }
)

$viewPairs = @(
    @{ View = 'City Hall'; Reference = 'reference-cityhall'; Winner = 'combined-winner-cityhall'; File = 'AtlasHero_CityHall_01.jpg' },
    @{ View = 'East'; Reference = 'reference-east'; Winner = 'combined-winner-east-final'; File = 'AtlasHero_East_01.jpg' },
    @{ View = 'North'; Reference = 'reference-north-clean'; Winner = 'combined-winner-north-final'; File = 'AtlasHero_North_01.jpg' },
    @{ View = 'West'; Reference = 'reference-west'; Winner = 'combined-winner-west'; File = 'AtlasHero_West_01.jpg' }
)

$ultraPairs = @(
    @{ View = 'City Hall'; Ultra = 'stock-ultra-cityhall'; Winner = 'winner-cityhall-rerun'; File = 'AtlasHero_CityHall_01.jpg' },
    @{ View = 'East'; Ultra = 'stock-ultra-east'; Winner = 'winner-east-rerun'; File = 'AtlasHero_East_01.jpg' },
    @{ View = 'North'; Ultra = 'stock-ultra-north'; Winner = 'winner-north-rerun'; File = 'AtlasHero_North_01.jpg' },
    @{ View = 'West'; Ultra = 'stock-ultra-west'; Winner = 'winner-west-rerun'; File = 'AtlasHero_West_01.jpg' }
)

$matrixTiles = $matrixNames | ForEach-Object {
    if (-not (Test-Path $_.Path)) { throw "Missing matrix image: $($_.Path)" }
    [pscustomobject]@{ Label = $_.Label; Path = (Resolve-Path $_.Path).Path }
}
$viewTiles = foreach ($pair in $viewPairs) {
    $referencePath = Join-Path $EvidenceRoot "$($pair.Reference)\$($pair.File)"
    $winnerPath = Join-Path $EvidenceRoot "$($pair.Winner)\$($pair.File)"
    if (-not (Test-Path $referencePath)) { throw "Missing reference image: $referencePath" }
    if (-not (Test-Path $winnerPath)) { throw "Missing winner image: $winnerPath" }
    [pscustomobject]@{ Label = "$($pair.View) — current"; Path = (Resolve-Path $referencePath).Path }
    [pscustomobject]@{ Label = "$($pair.View) — combined winner"; Path = (Resolve-Path $winnerPath).Path }
}
$ultraTiles = foreach ($pair in $ultraPairs) {
    $ultraPath = Join-Path $EvidenceRoot "$($pair.Ultra)\$($pair.File)"
    $winnerPath = Join-Path $EvidenceRoot "$($pair.Winner)\$($pair.File)"
    if (-not (Test-Path $ultraPath)) { throw "Missing Stock Ultra image: $ultraPath" }
    if (-not (Test-Path $winnerPath)) { throw "Missing winner image: $winnerPath" }
    [pscustomobject]@{ Label = "$($pair.View) — Stock Ultra"; Path = (Resolve-Path $ultraPath).Path }
    [pscustomobject]@{ Label = "$($pair.View) — #25 winner"; Path = (Resolve-Path $winnerPath).Path }
}

New-ContactSheet -OutputPath (Join-Path $OutputRoot 'issue25-atlas-depth-matrix.jpg') -Tiles $matrixTiles
New-ContactSheet -OutputPath (Join-Path $OutputRoot 'issue25-atlas-depth-contact-sheet.jpg') -Tiles $viewTiles -Columns 4
New-ContactSheet -OutputPath (Join-Path $OutputRoot 'issue25-atlas-depth-ultra-contact-sheet.jpg') -Tiles $ultraTiles -Columns 4

Write-Output "Wrote issue25-atlas-depth-matrix.jpg, issue25-atlas-depth-contact-sheet.jpg, and issue25-atlas-depth-ultra-contact-sheet.jpg"
