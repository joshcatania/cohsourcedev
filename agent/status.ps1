[CmdletBinding()]
param(
    [switch]$Json
)

$processNames = @(
    'ServerMonitor',
    'AuthServer',
    'DbServer',
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
    note = 'Process presence is diagnostic only. Full shard readiness requires a validated application-level health signal or TestClient smoke test.'
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
    Write-Host 'READINESS: UNKNOWN (process presence alone is not treated as healthy)'
}

if (-not $monitorRunning) { exit 1 }
exit 0
