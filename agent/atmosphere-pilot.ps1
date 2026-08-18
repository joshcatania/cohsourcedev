[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$Name,
    [string]$Target = 'AtlasHero_CityHall_01',
    [string]$AccountName = 'Dummy00009',
    [int]$TimeoutSeconds = 180,
    [ValidateSet(0, 1)]
    [int]$GlslPilot = 1,
    [switch]$ModernLighting,
    [switch]$ModernAtmosphere,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$captureScript = Join-Path $PSScriptRoot 'capture.ps1'
$registryRoot = Join-Path $repoRoot 'bin/registry-keys/hkey_current_user/software/cryptic/coh'
$evidenceRoot = Join-Path $repoRoot 'docs/evidence/issue28-atlas-remaster-atmosphere-v1'
$safeName = $Name -replace '[^A-Za-z0-9_-]', '_'
$safeTarget = $Target -replace '[^A-Za-z0-9_-]', '_'
$outputDirectory = Join-Path $evidenceRoot $safeName
$captureOutputPath = Join-Path $outputDirectory "$safeTarget.capture.json"
$settingsOutputPath = Join-Path $outputDirectory "$safeTarget.settings.json"
$imageOutputPath = Join-Path $outputDirectory "$safeTarget.jpg"
$runnerOutput = Join-Path $outputDirectory "$safeTarget.runner.stdout.log"
$runnerError = Join-Path $outputDirectory "$safeTarget.runner.stderr.log"

New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null

$settings = [ordered]@{
    shadowmode = 4
    shadowmapshader = 3
    shadowmapsize = 1
    shadowmapdistance = 2
    ambientstrength = 3
    ambientresolution = 2
    ambientblur = 6
    graphicsquality = '1.000000'
    shaderdetail = 3
    usewater = 2
}

$original = @{}
foreach ($key in $settings.Keys) {
    $path = Join-Path $registryRoot $key
    if (Test-Path -LiteralPath $path) {
        $original[$key] = [pscustomobject]@{ exists = $true; value = Get-Content -Raw -LiteralPath $path }
    } else {
        $original[$key] = [pscustomobject]@{ exists = $false; value = $null }
    }
}

function Restore-Registry {
    foreach ($key in $settings.Keys) {
        $path = Join-Path $registryRoot $key
        $saved = $original[$key]
        if ($saved.exists) {
            Set-Content -LiteralPath $path -Value $saved.value -NoNewline
        } elseif (Test-Path -LiteralPath $path) {
            Remove-Item -LiteralPath $path -Force
        }
    }
}

$captureExitCode = 1
$capture = $null
$reason = $null
try {
    foreach ($key in $settings.Keys) {
        Set-Content -LiteralPath (Join-Path $registryRoot $key) -Value ([string]$settings[$key]) -NoNewline
    }

    $lightingValue = if ($ModernLighting) { 1 } else { 0 }
    $atmosphereValue = if ($ModernAtmosphere) { 1 } else { 0 }
    $extraArgs = "-glslPilot $GlslPilot -modernMaterials 1 -modernPresentation 0 -modernBloom 0 -modernLighting $lightingValue -modernAtmosphere $atmosphereValue"
    $captureArgs = @(
        '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $captureScript,
        '-Target', $Target, '-AccountName', $AccountName, '-TimeoutSeconds', $TimeoutSeconds,
        '-ExtraClientArgs', $extraArgs, '-Json'
    )
    $captureText = (& powershell.exe @captureArgs 2>&1 | Out-String)
    Set-Content -LiteralPath $runnerOutput -Value $captureText -Encoding UTF8
    $captureExitCode = $LASTEXITCODE

    $jsonStart = $captureText.IndexOf('{')
    if ($jsonStart -lt 0) {
        throw "Capture wrapper did not return JSON for $Target"
    }
    $capture = $captureText.Substring($jsonStart) | ConvertFrom-Json
    $capture | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $captureOutputPath -Encoding UTF8
    if ($capture.screenshot -and (Test-Path -LiteralPath $capture.screenshot)) {
        Copy-Item -LiteralPath $capture.screenshot -Destination $imageOutputPath -Force
    }
} catch {
    $reason = $_.Exception.Message
} finally {
    Restore-Registry
}

$result = [ordered]@{
    name = $Name
    target = $Target
    modernLighting = [bool]$ModernLighting
    modernAtmosphere = [bool]$ModernAtmosphere
    requestedSettings = $settings
    capturePassed = ($captureExitCode -eq 0 -and $capture -and $capture.passed)
    captureExitCode = $captureExitCode
    screenshot = if (Test-Path -LiteralPath $imageOutputPath) { $imageOutputPath } else { '' }
    captureResult = if (Test-Path -LiteralPath $captureOutputPath) { $captureOutputPath } else { '' }
    runnerStdout = $runnerOutput
    runnerStderr = $runnerError
    reason = $reason
}
$result | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $settingsOutputPath -Encoding UTF8
if ($Json) {
    $result | ConvertTo-Json -Depth 8
} elseif ($result.capturePassed) {
    Write-Host "ATMOSPHERE PILOT PASS - $imageOutputPath"
} else {
    Write-Host "ATMOSPHERE PILOT FAIL - $($result.reason)"
}
if (-not $result.capturePassed) { exit 1 }
