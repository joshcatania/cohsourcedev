[CmdletBinding()]
param(
    [switch]$Json
)

# Process presence is diagnostic only. Launcher is important because DbServer
# can use it to start MapServers on demand; a stopped MapServer is therefore
# not, by itself, evidence that a freshly started shard is unhealthy.
$processNames = @(
    'ServerMonitor',
    'AuthServer',
    'DbServer',
    'Launcher',
    'MapServer',
    'AccountServer',
    'ChatServer',
    'AuctionServer',
    'MissionServer',
    'TurnstileServer',
    'QueueServer',
    'StatServer'
)

$rows = foreach ($name in $processNames) {
    $procs = @(Get-Process -Name $name -ErrorAction SilentlyContinue)
    [pscustomobject]@{
        name = $name
        running = ($procs.Count -gt 0)
        count = $procs.Count
        pids = @($procs | ForEach-Object Id)
    }
}

$monitorRunning = [bool]($rows | Where-Object { $_.name -eq 'ServerMonitor' -and $_.running })
$serverCount = @($rows | Where-Object { $_.name -ne 'ServerMonitor' -and $_.running }).Count

$result = [pscustomobject]@{
    serverMonitorRunning = $monitorRunning
    observedServerProcessTypes = $serverCount
    processes = $rows
    readiness = 'unknown'
    note = 'Process presence is diagnostic only. MapServer may be started on demand through Launcher. Full readiness requires an application-level check such as agent/smoke.ps1.'
}

if ($Json) {
    $result | ConvertTo-Json -Depth 5
} else {
    Write-Host 'COH SHARD PROCESS STATUS'
    Write-Host ''
    foreach ($row in $rows) {
        $state = if ($row.running) { 'RUNNING' } else { 'STOPPED' }
        $extra = if ($row.running) { " ($($row.count)) PID(s): $($row.pids -join ', ')" } else { '' }
        Write-Host ('{0,-20} {1}{2}' -f $row.name, $state, $extra)
    }
    Write-Host ''
    Write-Host 'READINESS: UNKNOWN (run .\agent\smoke.ps1 for an application-level login/DB check)'
}

if (-not $monitorRunning) { exit 1 }
exit 0
