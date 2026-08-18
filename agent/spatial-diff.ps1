param(
    [Parameter(Mandatory=$true)][string]$Baseline,
    [Parameter(Mandatory=$true)][string]$Current,
    [int]$Bands = 4
)
Add-Type -AssemblyName System.Drawing
function Load([string]$Path) {
    $bmp = [System.Drawing.Bitmap]::FromFile($Path)
    $w = 320
    $h = [int]([Math]::Max(1, [Math]::Round($bmp.Height * ($w / [double]$bmp.Width))))
    $scaled = New-Object System.Drawing.Bitmap($w, $h)
    $g = [System.Drawing.Graphics]::FromImage($scaled)
    $g.DrawImage($bmp, 0, 0, $w, $h)
    $g.Dispose(); $bmp.Dispose()
    return ,$scaled
}
$a = Load $Baseline; $b = Load $Current
$w = $a.Width; $h = $a.Height
$bandH = [int]($h / $Bands)
for ($band = 0; $band -lt $Bands; $band++) {
    $y0 = $band * $bandH
    $y1 = if ($band -eq $Bands - 1) { $h } else { $y0 + $bandH }
    $sumR=0.0; $sumG=0.0; $sumB=0.0; $n=0
    for ($y = $y0; $y -lt $y1; $y += 2) {
        for ($x = 0; $x -lt $w; $x += 2) {
            $ca = $a.GetPixel($x, $y); $cb = $b.GetPixel($x, $y)
            $sumR += ($cb.R - $ca.R); $sumG += ($cb.G - $ca.G); $sumB += ($cb.B - $ca.B); $n++
        }
    }
    "{0}: rows {1,3}-{2,3}  dR {3,7:N2}  dG {4,7:N2}  dB {5,7:N2}" -f $Baseline.Split('\')[-1], $y0, $y1, ($sumR/$n), ($sumG/$n), ($sumB/$n)
}
$a.Dispose(); $b.Dispose()
