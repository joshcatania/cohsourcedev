[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$Name,
    [string]$Target = 'AtlasHero_CityHall_01',
    [string]$AccountName = 'Dummy00009',
    [int]$TimeoutSeconds = 180,
    [int]$ShadowMode = 0,
    [int]$ShadowShader = 0,
    [int]$ShadowSize = 0,
    [int]$ShadowDistance = -2,
    [int]$AmbientStrength = 0,
    [int]$AmbientResolution = 2,
    [int]$AmbientBlur = 5,
    [switch]$MeasureFps,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$captureScript = Join-Path $PSScriptRoot 'capture.ps1'
$registryRoot = Join-Path $repoRoot 'bin/registry-keys/hkey_current_user/software/cryptic/coh'
$evidenceRoot = Join-Path $repoRoot 'docs/evidence/issue25-depth-pilot'
$safeName = $Name -replace '[^A-Za-z0-9_-]', '_'
$safeTarget = $Target -replace '[^A-Za-z0-9_-]', '_'
$outputDirectory = Join-Path $evidenceRoot $safeName
$runnerOutput = Join-Path $outputDirectory "$safeTarget.runner.stdout.log"
$runnerError = Join-Path $outputDirectory "$safeTarget.runner.stderr.log"
$captureOutputPath = Join-Path $outputDirectory "$safeTarget.capture.json"
$settingsOutputPath = Join-Path $outputDirectory "$safeTarget.settings.json"
$imageOutputPath = Join-Path $outputDirectory "$safeTarget.jpg"
$presentMonOutput = Join-Path $outputDirectory "$safeTarget.presentmon.csv"
$presentMonStdout = Join-Path $outputDirectory "$safeTarget.presentmon.stdout.log"
$presentMonStderr = Join-Path $outputDirectory "$safeTarget.presentmon.stderr.log"

New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null

$settings = [ordered]@{
    shadowmode = $ShadowMode
    shadowmapshader = $ShadowShader
    shadowmapsize = $ShadowSize
    shadowmapdistance = $ShadowDistance
    ambientstrength = $AmbientStrength
    ambientresolution = $AmbientResolution
    ambientblur = $AmbientBlur
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

try {
    foreach ($key in $settings.Keys) {
        Set-Content -LiteralPath (Join-Path $registryRoot $key) -Value ([string]$settings[$key]) -NoNewline
    }

    $extraArgs = '-glslPilot 1 -modernMaterials 1 -modernPresentation 0 -modernBloom 0'
    if ($MeasureFps) {
        $extraArgs += ' -showfps 2'
    }

    $extraArgValue = if ($MeasureFps) { '"' + ($extraArgs -replace '"', '\\"') + '"' } else { $extraArgs }
    $captureArgs = @(
        '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $captureScript,
        '-Target', $Target, '-AccountName', $AccountName, '-TimeoutSeconds', $TimeoutSeconds,
        '-ExtraClientArgs', $extraArgValue, '-Json'
    )
    $fpsSamples = [System.Collections.Generic.List[double]]::new()
    $cpuSamples = [System.Collections.Generic.List[double]]::new()
    $gpuSamples = [System.Collections.Generic.List[double]]::new()
    $lastCpuMs = $null
    $lastSampleAt = $null
    $lastGpuSampleAt = [datetime]::MinValue
    $presentSummary = $null
    $captureExitCode = $null
    $captureText = ''

    if ($MeasureFps) {
        $presentMon = $null
        $targetProcess = $null
        $presentMonPath = 'C:\Program Files\NVIDIA Corporation\FrameViewSDK\bin\PresentMon_x64.exe'
        $runner = Start-Process -FilePath 'powershell.exe' -ArgumentList $captureArgs -WorkingDirectory $repoRoot -PassThru -RedirectStandardOutput $runnerOutput -RedirectStandardError $runnerError
        if (Test-Path -LiteralPath $presentMonPath) {
            $presentDeadline = (Get-Date).AddSeconds(15)
            while (-not $targetProcess -and (Get-Date) -lt $presentDeadline) {
                $targetProcess = Get-Process -Name 'Ouroboros' -ErrorAction SilentlyContinue | Select-Object -First 1
                Start-Sleep -Milliseconds 250
            }
            try {
                $presentArgs = '--process_id ' + $targetProcess.Id + ' --output_file "' + $presentMonOutput + '" --no_console_stats --timed 10 --terminate_after_timed --stop_existing_session --session_name Issue25_' + $safeName
                $presentMon = Start-Process -FilePath $presentMonPath -ArgumentList $presentArgs -PassThru -WindowStyle Hidden -RedirectStandardOutput $presentMonStdout -RedirectStandardError $presentMonStderr
                Start-Sleep -Milliseconds 500
            } catch {
                $presentMon = $null
            }
        }
        $deadline = (Get-Date).AddSeconds($TimeoutSeconds + 30)
        while (-not $runner.HasExited -and (Get-Date) -lt $deadline) {
            $ouroboros = Get-Process -Name 'Ouroboros' -ErrorAction SilentlyContinue | Select-Object -First 1
            if ($ouroboros) {
                $ouroboros.Refresh()
                $now = Get-Date
                $cpuMs = $ouroboros.TotalProcessorTime.TotalMilliseconds
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
                $title = $ouroboros.MainWindowTitle
                $match = [regex]::Match($title, 'FPS:\s*([0-9]+(?:\.[0-9]+)?)')
                if ($match.Success) {
                    $fpsSamples.Add([double]$match.Groups[1].Value)
                }
            }
            Start-Sleep -Milliseconds 250
        }
        if (-not $runner.HasExited) {
            Stop-Process -Id $runner.Id -Force -ErrorAction SilentlyContinue
            throw "Capture wrapper exceeded its timeout for $Target"
        }
        $runner.WaitForExit()
        $captureExitCode = $runner.ExitCode
        $captureText = if (Test-Path -LiteralPath $runnerOutput) { [string](Get-Content -Raw -LiteralPath $runnerOutput) } else { '' }
        if ($presentMon) {
            $presentDeadline = (Get-Date).AddSeconds(10)
            while (-not $presentMon.HasExited -and (Get-Date) -lt $presentDeadline) {
                Start-Sleep -Milliseconds 250
            }
            if (-not $presentMon.HasExited) {
                Stop-Process -Id $presentMon.Id -Force -ErrorAction SilentlyContinue
            }
        }
        if (Test-Path -LiteralPath $presentMonOutput) {
            $frameTimes = [System.Collections.Generic.List[double]]::new()
            foreach ($row in @(Import-Csv -LiteralPath $presentMonOutput)) {
                $ms = 0.0
                if ([double]::TryParse([string]$row.MsBetweenPresents, [Globalization.NumberStyles]::Float, [Globalization.CultureInfo]::InvariantCulture, [ref]$ms) -and $ms -gt 0.5 -and $ms -lt 100.0) {
                    $frameTimes.Add($ms)
                }
            }
            if ($frameTimes.Count -gt 0) {
                $sortedFrameTimes = @($frameTimes | Sort-Object)
                $medianIndex = [int][math]::Floor(($sortedFrameTimes.Count - 1) * 0.50)
                $p95Index = [int][math]::Floor(($sortedFrameTimes.Count - 1) * 0.95)
                $medianMs = [double]$sortedFrameTimes[$medianIndex]
                $p95Ms = [double]$sortedFrameTimes[$p95Index]
                $presentSummary = [ordered]@{
                    sampleCount = $frameTimes.Count
                    medianMsBetweenPresents = [math]::Round($medianMs, 3)
                    p95MsBetweenPresents = [math]::Round($p95Ms, 3)
                    medianFps = [math]::Round(1000.0 / $medianMs, 2)
                    p95FrameTimeFps = [math]::Round(1000.0 / $p95Ms, 2)
                }
            }
        }
    } else {
        $captureText = (& powershell.exe @captureArgs 2>&1 | Out-String)
        $captureExitCode = $LASTEXITCODE
        Set-Content -LiteralPath $runnerOutput -Value $captureText -Encoding UTF8
    }

    $jsonStart = $captureText.IndexOf('{')
    if ($jsonStart -lt 0) {
        throw "Capture wrapper did not return JSON for $Target"
    }
    $capture = $captureText.Substring($jsonStart) | ConvertFrom-Json
    $capture | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $captureOutputPath -Encoding UTF8
    if ($capture.screenshot -and (Test-Path -LiteralPath $capture.screenshot)) {
        Copy-Item -LiteralPath $capture.screenshot -Destination $imageOutputPath -Force
    }

    $fpsSummary = $null
    if ($fpsSamples.Count -gt 0) {
        $orderedFps = @($fpsSamples | Sort-Object)
        $fpsSummary = [ordered]@{
            sampleCount = $fpsSamples.Count
            min = [math]::Round(($orderedFps | Select-Object -First 1), 2)
            median = [math]::Round($orderedFps[[int][math]::Floor($orderedFps.Count / 2)], 2)
            max = [math]::Round(($orderedFps | Select-Object -Last 1), 2)
        }
    }

    $performanceSummary = [ordered]@{
        cpuSampleCount = $cpuSamples.Count
        gpuSampleCount = $gpuSamples.Count
        cpuMeanPercentOfMachine = if ($cpuSamples.Count) { [math]::Round((($cpuSamples | Measure-Object -Average).Average), 2) } else { $null }
        cpuMaxPercentOfMachine = if ($cpuSamples.Count) { [math]::Round((($cpuSamples | Measure-Object -Maximum).Maximum), 2) } else { $null }
        gpuMeanPercent = if ($gpuSamples.Count) { [math]::Round((($gpuSamples | Measure-Object -Average).Average), 2) } else { $null }
        gpuMaxPercent = if ($gpuSamples.Count) { [math]::Round((($gpuSamples | Measure-Object -Maximum).Maximum), 2) } else { $null }
        presentMon = $presentSummary
    }

    $result = [ordered]@{
        name = $Name
        target = $Target
        requestedSettings = $settings
        capturePassed = ($captureExitCode -eq 0 -and $capture.passed)
        captureExitCode = $captureExitCode
        screenshot = $imageOutputPath
        captureResult = $captureOutputPath
        fps = $fpsSummary
        presentMon = $presentSummary
        presentMonStdout = $presentMonStdout
        presentMonStderr = $presentMonStderr
        performance = $performanceSummary
        fpsSamples = @($fpsSamples)
        runnerStdout = $runnerOutput
        runnerStderr = $runnerError
    }
    $result | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $settingsOutputPath -Encoding UTF8
    if ($Json) { $result | ConvertTo-Json -Depth 8 } else { Write-Host "DEPTH PILOT PASS - $imageOutputPath" }
    if (-not $result.capturePassed) { exit 1 }
} finally {
    Restore-Registry
}
