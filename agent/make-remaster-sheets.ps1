# Build stock-vs-remaster contact sheets for human review.
[CmdletBinding()]
param(
    [string]$Root = 'D:\github\coh-graphics\agent\captures\remaster-ab',
    [string[]]$Labels = @('stock','remaster-final'),
    [string[]]$Shots = @('AtlasPlaza_CityHall_03','AtlasPlaza_East_01','AtlasPlaza_NightEast_01','FoundersCanal_01'),
    [string]$Out = 'D:\github\coh-graphics\agent\captures\remaster-ab'
)
Add-Type -AssemblyName System.Drawing
$labelW = 220
$thumbW = 620
$thumbH = 349
$pad = 8
$headerH = 28
$sheetW = $labelW + $thumbW * $Labels.Count + $pad * ($Labels.Count + 1)
$sheetH = $headerH + ($thumbH + $pad) * $Shots.Count + $pad
$bmp = New-Object System.Drawing.Bitmap $sheetW, $sheetH
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.Clear([System.Drawing.Color]::Black)
$titleFont = New-Object System.Drawing.Font 'Consolas', 14, ([System.Drawing.FontStyle]::Bold)
$rowFont = New-Object System.Drawing.Font 'Consolas', 12
$labelBrush = [System.Drawing.Brushes]::White
$labelColors = @([System.Drawing.Color]::FromArgb(180,60,60), [System.Drawing.Color]::FromArgb(60,140,220))
$g.DrawString('Remaster Profile v1  A/B', $titleFont, $labelBrush, $pad, 4)
$x = $labelW + $pad
for ($i = 0; $i -lt $Labels.Count; $i++) {
    $g.FillRectangle((New-Object System.Drawing.SolidBrush $labelColors[$i]), $x, 4, $thumbW, 20)
    $g.DrawString($Labels[$i], $rowFont, $labelBrush, $x + 8, 5)
    $x += $thumbW + $pad
}
for ($r = 0; $r -lt $Shots.Count; $r++) {
    $y = $headerH + ($thumbH + $pad) * $r
    $g.DrawString($Shots[$r], $rowFont, $labelBrush, $pad, $y + $thumbH / 2 - 10)
    $x = $labelW + $pad
    for ($i = 0; $i -lt $Labels.Count; $i++) {
        $p = Join-Path $Root "$($Labels[$i])\$($Shots[$r]).jpg"
        if (Test-Path $p) {
            $img = [System.Drawing.Image]::FromFile($p)
            $g.DrawImage($img, $x, $y, $thumbW, $thumbH)
            $img.Dispose()
        }
        $x += $thumbW + $pad
    }
}
$g.Dispose()
$outPath = Join-Path $Out 'remaster-ab-contact-sheet.jpg'
$bmp.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Jpeg)
$bmp.Dispose()
Write-Host "sheet: $outPath"
