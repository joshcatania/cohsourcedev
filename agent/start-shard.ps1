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
Start-Sleep -Seconds $StartupWaitSeconds
$stillRunning = Get-Process -Id $process.Id -ErrorAction SilentlyContinue

$result = [pscustomobject]@{
    started = [bool]$stillRunning
    alreadyRunning = $false
    processId = $process.Id
    executable = $monitor
    note = 'This wrapper currently proves ServerMonitor stayed alive after launch. Service-level shard readiness still needs a validated health signal.'
}

if ($Json) {
    $result | ConvertTo-Json -Depth 3
} else {
    if ($stillRunning) {
        Write-Host "ServerMonitor started (PID $($process.Id))."
        Write-Host 'NOTE: process-alive is not yet equivalent to full shard readiness. Run .\agent\status.ps1.'
    } else {
        Write-Host 'ServerMonitor exited during startup.'
    }
}

if (-not $stillRunning) { exit 1 }
exit 0
