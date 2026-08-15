[CmdletBinding()]
param(
    [int]$StartupWaitSeconds = 8,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$binDir = Join-Path $repoRoot 'bin'
$monitor = Join-Path $binDir 'ServerMonitor.exe'

if (-not (Test-Path $monitor)) {
    Write-Error "ServerMonitor.exe not found at $monitor. Run .\agent\build.ps1 first."
    exit 2
}

$existing = Get-Process -Name ServerMonitor -ErrorAction SilentlyContinue
if ($existing) {
    $result = [pscustomobject]@{
        started = $false
        alreadyRunning = $true
        processId = @($existing.Id)
        executable = $monitor
    }
    if ($Json) { $result | ConvertTo-Json -Depth 3 } else { Write-Host "ServerMonitor already running (PID $($existing.Id -join ', '))." }
    exit 0
}

$process = Start-Process -FilePath $monitor -ArgumentList '-connect' -WorkingDirectory $binDir -PassThru

# ServerMonitor starts DbServer and Launcher asynchronously. Treat the
# existing parameter as a bounded readiness timeout, and poll for the
# processes that the direct-DbServer workflow actually needs. This is still
# only process-level readiness; agent/smoke.ps1 remains the application-level
# check.
$deadline = [DateTime]::UtcNow.AddSeconds([math]::Max(1, $StartupWaitSeconds))
$requiredNames = @('ServerMonitor', 'DbServer', 'Launcher')
$missing = @($requiredNames)
$readyAtSeconds = $null
while ([DateTime]::UtcNow -lt $deadline) {
    $missing = @($requiredNames | Where-Object {
        -not (Get-Process -Name $_ -ErrorAction SilentlyContinue)
    })
    if ($missing.Count -eq 0) {
        $readyAtSeconds = [math]::Round(([DateTime]::UtcNow - $process.StartTime.ToUniversalTime()).TotalSeconds, 2)
        break
    }
    Start-Sleep -Milliseconds 250
}

$stillRunning = Get-Process -Id $process.Id -ErrorAction SilentlyContinue
$processReady = ($null -ne $stillRunning) -and ($missing.Count -eq 0)

$result = [pscustomobject]@{
    started = [bool]$stillRunning
    alreadyRunning = $false
    processId = $process.Id
    executable = $monitor
    processReady = $processReady
    requiredProcesses = $requiredNames
    missingProcesses = $missing
    readiness = if ($processReady) { 'process-ready' } else { 'startup-timeout' }
    readyAtSeconds = $readyAtSeconds
    note = 'Process-ready means ServerMonitor, DbServer, and Launcher were observed. Run agent/smoke.ps1 for the direct-DB application-level check.'
}

if ($Json) {
    $result | ConvertTo-Json -Depth 3
} else {
    if ($stillRunning) {
        Write-Host "ServerMonitor started (PID $($process.Id))."
        if ($processReady) {
            Write-Host ("Required startup processes observed after {0:N1}s." -f $readyAtSeconds)
        } else {
            Write-Host ("Startup timeout; still waiting for: {0}" -f ($missing -join ', '))
        }
        Write-Host 'NOTE: process-ready is not equivalent to full application readiness. Run .\agent\smoke.ps1.'
    } else {
        Write-Host 'ServerMonitor exited during startup.'
    }
}

if (-not $stillRunning -or -not $processReady) { exit 1 }
exit 0
