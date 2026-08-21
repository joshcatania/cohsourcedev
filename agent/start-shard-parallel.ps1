# Start a parallel local shard from this worktree on shifted ports so it can
# run side by side with another agent's shard in a different worktree.
#
# All server/client ports defined in Common/comm_backend.h shift by
# $PortOffset via the COH_PORT_OFFSET environment variable (inherited by
# spawned server processes and by capture.ps1/smoke.ps1 when launched from a
# shell that has the variable set). This script refuses to run without the
# variable isolation and records what it started.
#
# Usage:
#   .\agent\start-shard-parallel.ps1                 # offset 1000 => ports 7971-8001
#   .\agent\start-shard-parallel.ps1 -PortOffset 500
#   .\agent\start-shard-parallel.ps1 -StartupWaitSeconds 60
[CmdletBinding()]
param(
    [ValidateRange(1, 65535)]
    [int]$PortOffset = 1000,
    [int]$StartupWaitSeconds = 45
)
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$binRoot = Join-Path $repoRoot 'bin'
$workTag = Join-Path $repoRoot 'agent\work\parallel-shard'

New-Item -ItemType Directory -Force -Path $workTag | Out-Null

# Guard: validate the EFFECTIVE port band, not just the raw offset. The base
# band is 6971-7000 and public mapserver ports continue upward from
# 7001(+offset) via an unbounded bind scan, so the offset must leave room
# below the 65535 TCP/UDP port ceiling (same bound as COH_MAX_PORT_OFFSET in
# Common/comm_backend.h, which clamps defensively).
$bandStart = 6971 + $PortOffset
$bandEnd = 7001 + $PortOffset
$mapPortHeadroom = 512
$maxOffset = 65535 - 7001 - $mapPortHeadroom
if ($PortOffset -gt $maxOffset) {
    throw "PortOffset $PortOffset puts the effective band at $bandStart-$($bandEnd + $mapPortHeadroom) (mapserver scan headroom included) above the 65535 port ceiling. Use -PortOffset <= $maxOffset."
}

# Guard: the target band (offset applied) must be free. Another shard may own
# the baseline band; we only refuse if something already listens in ours.
$listeners = Get-NetTCPConnection -State Listen -ErrorAction SilentlyContinue |
    Where-Object { $_.LocalPort -ge $bandStart -and $_.LocalPort -le $bandEnd }
if ($listeners) {
    $ports = ($listeners | Select-Object -ExpandProperty LocalPort -Unique | Sort-Object) -join ','
    throw "Target port band $bandStart-$bandEnd already has listeners: $ports. Choose another -PortOffset."
}

# Guard: this worktree must not already have a shard running.
$existing = Get-CimInstance Win32_Process -Filter "Name='DbServer.exe'" |
    Where-Object { $_.ExecutablePath -like "$binRoot*" }
if ($existing) {
    throw "This worktree already has a DbServer running (PID $($existing.ProcessId))."
}

$env:COH_PORT_OFFSET = [string]$PortOffset

# NOTE: ServerMonitor.exe is deliberately NOT used here. Its process monitor
# matches DbServer/Launcher by exe name system-wide, so another agent's shard
# in a different worktree satisfies it and it would never spawn ours. Instead
# spawn the same pair it would have spawned (processMonitor.c table), which
# then brings up the MapServers via the launcher protocol.
$dbLog = Join-Path $workTag 'dbserver.console.log'
$launcherLog = Join-Path $workTag 'launcher.console.log'
$db = Start-Process -FilePath (Join-Path $binRoot 'DbServer.exe') `
    -ArgumentList '-startall' -WorkingDirectory $binRoot -PassThru `
    -RedirectStandardOutput $dbLog -RedirectStandardError (Join-Path $workTag 'dbserver.console.err.log')
$launcher = Start-Process -FilePath (Join-Path $binRoot 'Launcher.exe') `
    -ArgumentList '-noversioncheck' -WorkingDirectory $binRoot -PassThru `
    -RedirectStandardOutput $launcherLog -RedirectStandardError (Join-Path $workTag 'launcher.console.err.log')
"dbPid=$($db.Id) launcherPid=$($launcher.Id) portOffset=$PortOffset db=cohgfx started=$(Get-Date -Format o)" |
    Set-Content -LiteralPath (Join-Path $workTag 'state.txt') -NoNewline

Write-Host "DbServer (PID $($db.Id)) and Launcher (PID $($launcher.Id)) started with COH_PORT_OFFSET=$PortOffset (ports $bandStart-$bandEnd)."
Write-Host 'Polling for DbServer and Launcher readiness...'

$deadline = (Get-Date).AddSeconds($StartupWaitSeconds)
$seenDb = $false; $seenLauncher = $false
while ((Get-Date) -lt $deadline) {
    Start-Sleep -Milliseconds 500
    $mine = Get-CimInstance Win32_Process -Filter "Name='DbServer.exe' or Name='Launcher.exe'" |
        Where-Object { $_.ExecutablePath -like "$binRoot*" }
    if ($mine | Where-Object Name -eq 'DbServer.exe') { $seenDb = $true }
    if ($mine | Where-Object Name -eq 'Launcher.exe') { $seenLauncher = $true }
    if ($seenDb -and $seenLauncher) { break }
}

if (-not ($seenDb -and $seenLauncher)) {
    Write-Host "PARALLEL SHARD START INCOMPLETE (db=$seenDb launcher=$seenLauncher after ${StartupWaitSeconds}s)."
    Write-Host 'DbServer may still be creating the cohgfx database on first run; check bin/logs/dbserver.'
} else {
    Write-Host 'PARALLEL SHARD UP: DbServer and Launcher observed from this worktree.'
}
Write-Host 'Remember: launch clients (capture.ps1/smoke.ps1/Ouroboros) from a shell with $env:COH_PORT_OFFSET set to the same offset.'
