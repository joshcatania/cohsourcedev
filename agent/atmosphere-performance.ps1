[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$Name,
    [string]$Target = 'AtlasHero_CityHall_01',
    [string]$AccountName = 'Dummy00009',
    [int]$TimeoutSeconds = 180,
    [switch]$ModernLighting,
    [switch]$ModernAtmosphere,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$pilotScript = Join-Path $PSScriptRoot 'atmosphere-pilot.ps1'
$safeName = $Name -replace '[^A-Za-z0-9_-]', '_'
$outputDirectory = Join-Path $repoRoot "docs/evidence/issue28-atlas-remaster-atmosphere-v1/$safeName"
$runnerOutput = Join-Path $outputDirectory "$safeName.runner.stdout.log"
$runnerError = Join-Path $outputDirectory "$safeName.runner.stderr.log"
$settingsOutputPath = Join-Path $outputDirectory "$safeName.performance.json"
New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null

$modernLightingArg = if ($ModernLighting) { '-ModernLighting' } else { '' }
$modernAtmosphereArg = if ($ModernAtmosphere) { '-ModernAtmosphere' } else { '' }
$pilotArgs = @(
    '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $pilotScript,
    '-Name', $Name, '-Target', $Target, '-AccountName', $AccountName,
    '-TimeoutSeconds', $TimeoutSeconds, '-Json'
)
if ($modernLightingArg) { $pilotArgs += $modernLightingArg }
if ($modernAtmosphereArg) { $pilotArgs += $modernAtmosphereArg }

$cpuSamples = [System.Collections.Generic.List[double]]::new()
$gpuSamples = [System.Collections.Generic.List[double]]::new()
$lastCpuMs = $null
$lastSampleAt = $null
$lastGpuSampleAt = [datetime]::MinValue
$runner = Start-Process -FilePath 'powershell.exe' -ArgumentList $pilotArgs -WorkingDirectory $repoRoot -PassThru -RedirectStandardOutput $runnerOutput -RedirectStandardError $runnerError
$deadline = (Get-Date).AddSeconds($TimeoutSeconds + 45)
while (-not $runner.HasExited -and (Get-Date) -lt $deadline) {
    $client = Get-Process -Name 'Ouroboros' -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($client) {
        $client.Refresh()
        $now = Get-Date
        $cpuMs = $client.TotalProcessorTime.TotalMilliseconds
        if ($null -ne $lastCpuMs -and $null -ne $lastSampleAt) {
            $wallMs = ($now - $lastSampleAt).TotalMilliseconds
            if ($wallMs -gt 0) {
                $cpuSamples.Add(100.0 * ($cpuMs - $lastCpuMs) / ($wallMs * [environment]::ProcessorCount))
            }
        }
        $lastCpuMs = $cpuMs
        $lastSampleAt = $now
        if (($now - $lastGpuSampleAt).TotalSeconds -ge 1.0) {
            try {
                $gpuText = (& nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader,nounits 2>$null | Select-Object -First 1).Trim()
                $gpuValue = 0.0
                if ([double]::TryParse($gpuText, [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref]$gpuValue)) {
                    $gpuSamples.Add($gpuValue)
                }
            } catch {}
            $lastGpuSampleAt = $now
        }
    }
    Start-Sleep -Milliseconds 250
}
if (-not $runner.HasExited) {
    Stop-Process -Id $runner.Id -Force -ErrorAction SilentlyContinue
    throw "Performance capture exceeded its timeout for $Target"
}
$runner.WaitForExit()
$captureText = if (Test-Path -LiteralPath $runnerOutput) { [string](Get-Content -Raw -LiteralPath $runnerOutput) } else { '' }
$jsonStart = $captureText.IndexOf('{')
$capture = if ($jsonStart -ge 0) { $captureText.Substring($jsonStart) | ConvertFrom-Json } else { $null }

$result = [ordered]@{
    name = $Name
    target = $Target
    modernLighting = [bool]$ModernLighting
    modernAtmosphere = [bool]$ModernAtmosphere
    capturePassed = [bool]($capture -and $capture.capturePassed)
    cpuSampleCount = $cpuSamples.Count
    gpuSampleCount = $gpuSamples.Count
    cpuMeanPercentOfMachine = if ($cpuSamples.Count) { [math]::Round((($cpuSamples | Measure-Object -Average).Average), 2) } else { $null }
    cpuMaxPercentOfMachine = if ($cpuSamples.Count) { [math]::Round((($cpuSamples | Measure-Object -Maximum).Maximum), 2) } else { $null }
    gpuMeanPercent = if ($gpuSamples.Count) { [math]::Round((($gpuSamples | Measure-Object -Average).Average), 2) } else { $null }
    gpuMaxPercent = if ($gpuSamples.Count) { [math]::Round((($gpuSamples | Measure-Object -Maximum).Maximum), 2) } else { $null }
    captureResult = if ($capture) { $capture.captureResult } else { '' }
    runnerStdout = $runnerOutput
    runnerStderr = $runnerError
}
$result | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $settingsOutputPath -Encoding UTF8
if ($Json) { $result | ConvertTo-Json -Depth 8 } else { $result | Format-List }
if (-not $result.capturePassed) { exit 1 }
