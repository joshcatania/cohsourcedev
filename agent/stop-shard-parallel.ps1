# Stop ONLY this worktree's parallel shard (started by start-shard-parallel.ps1).
# It matches processes by executable path under this worktree's bin directory,
# so another agent's shard in a different worktree is never touched.
[CmdletBinding()]
param(
    [switch]$ForceProcessStop
)
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$binRoot = Join-Path $repoRoot 'bin'

$names = @('ServerMonitor','DbServer','Launcher','MapServer','ChatServer','LogServer','BeaconServer','BeaconClient','AccountServer','AuctionServer','ArenaServer','MissionServer','TurnstileServer','QueueServer','StatServer','RaidServer')
$mine = @()
foreach ($n in $names) {
    $mine += Get-CimInstance Win32_Process -Filter "Name='$n.exe'" -ErrorAction SilentlyContinue |
        Where-Object { $_.ExecutablePath -like "$binRoot*" }
}

if (-not $mine) {
    Write-Host 'No shard processes from this worktree are running.'
    return
}

$mine | ForEach-Object { Write-Host "This worktree: $($_.Name) PID $($_.ProcessId)" }

if (-not $ForceProcessStop) {
    Write-Host 'No verified graceful ServerMonitor shutdown exists; refusing to terminate.'
    Write-Host 'Re-run with -ForceProcessStop for the disposable parallel shard (it is isolated: own DB and ports).'
    return
}

foreach ($p in ($mine | Sort-Object ProcessId -Descending)) {
    Stop-Process -Id $p.ProcessId -Force -ErrorAction SilentlyContinue
}
Start-Sleep -Seconds 2

# Bounded rescan for late-spawned children / exit races
$left = @()
foreach ($n in $names) {
    $left += Get-CimInstance Win32_Process -Filter "Name='$n.exe'" -ErrorAction SilentlyContinue |
        Where-Object { $_.ExecutablePath -like "$binRoot*" }
}
if ($left) {
    $left | ForEach-Object { Write-Host "STILL RUNNING: $($_.Name) PID $($_.ProcessId); stopping again" ; Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
    Start-Sleep -Seconds 1
}
Write-Host 'Parallel shard from this worktree stopped.'
