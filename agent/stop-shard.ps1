[CmdletBinding()]
param(
    [switch]$ForceProcessStop,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$names = @('ServerMonitor','AuthServer','DbServer','Launcher','MapServer','AccountServer','ChatServer','AuctionServer','MissionServer','TurnstileServer','QueueServer','StatServer','ArenaServer','RaidServer','LogServer','BeaconServer','BeaconClient')
$running = @()
foreach ($name in $names) {
    $running += @(Get-Process -Name $name -ErrorAction SilentlyContinue)
}

if ($running.Count -eq 0) {
    $result = [pscustomobject]@{ stopped = $true; alreadyStopped = $true; forced = $false; processes = @() }
    if ($Json) { $result | ConvertTo-Json -Depth 4 } else { Write-Host 'No known shard processes are running.' }
    exit 0
}

if (-not $ForceProcessStop) {
    $result = [pscustomobject]@{
        stopped = $false
        alreadyStopped = $false
        forced = $false
        processes = @($running | ForEach-Object { [pscustomobject]@{ name=$_.ProcessName; pid=$_.Id } })
        note = 'A verified graceful ServerMonitor shutdown interface has not yet been established. Re-run with -ForceProcessStop only on a disposable local development shard.'
    }
    if ($Json) { $result | ConvertTo-Json -Depth 4 } else {
        Write-Host 'Refusing to kill shard processes by default.'
        Write-Host 'The repo-verified graceful shutdown path has not been established yet.'
        Write-Host 'For a disposable local dev shard only, re-run: .\agent\stop-shard.ps1 -ForceProcessStop'
    }
    exit 2
}

$stopped = @()
foreach ($proc in ($running | Sort-Object { if ($_.ProcessName -eq 'ServerMonitor') { 1 } else { 0 } })) {
    try {
        if ($proc.MainWindowHandle -ne 0) {
            [void]$proc.CloseMainWindow()
            if ($proc.WaitForExit(3000)) {
                $stopped += [pscustomobject]@{ name=$proc.ProcessName; pid=$proc.Id; method='CloseMainWindow' }
                continue
            }
        }
        Stop-Process -Id $proc.Id -Force -ErrorAction Stop
        $stopped += [pscustomobject]@{ name=$proc.ProcessName; pid=$proc.Id; method='Force' }
    } catch {
        if ($_.Exception.Message -match 'Cannot find a process with the process identifier') {
            # A child can exit between the initial snapshot and this call.
            # Treat that race as success; the post-pass below catches any
            # process that was spawned while ServerMonitor was shutting down.
            $stopped += [pscustomobject]@{ name=$proc.ProcessName; pid=$proc.Id; method='AlreadyExited' }
        } else {
            $stopped += [pscustomobject]@{ name=$proc.ProcessName; pid=$proc.Id; method='Failed'; error=$_.Exception.Message }
        }
    }
}

# ServerMonitor can launch a final child while it is receiving its shutdown
# request. Re-scan the known shard process names for a bounded period and
# handle those late children as part of the same stop operation.
$processedKeys = @{}
foreach ($item in $stopped) { $processedKeys["$($item.name):$($item.pid)"] = $true }
$remaining = @()
$settleDeadline = [DateTime]::UtcNow.AddSeconds(10)
while ([DateTime]::UtcNow -lt $settleDeadline) {
    $late = @()
    foreach ($name in $names) { $late += @(Get-Process -Name $name -ErrorAction SilentlyContinue) }
    $late = @($late | Sort-Object Id -Unique)
    if ($late.Count -eq 0) {
        $remaining = @()
        break
    }

    foreach ($proc in $late) {
        $key = "$($proc.ProcessName):$($proc.Id)"
        if ($processedKeys.ContainsKey($key)) { continue }
        $processedKeys[$key] = $true
        try {
            if ($proc.MainWindowHandle -ne 0) {
                [void]$proc.CloseMainWindow()
                if ($proc.WaitForExit(1000)) {
                    $stopped += [pscustomobject]@{ name=$proc.ProcessName; pid=$proc.Id; method='CloseMainWindow' }
                    continue
                }
            }
            Stop-Process -Id $proc.Id -Force -ErrorAction Stop
            $stopped += [pscustomobject]@{ name=$proc.ProcessName; pid=$proc.Id; method='Force' }
        } catch {
            if ($_.Exception.Message -match 'Cannot find a process with the process identifier') {
                $stopped += [pscustomobject]@{ name=$proc.ProcessName; pid=$proc.Id; method='AlreadyExited' }
            } else {
                $stopped += [pscustomobject]@{ name=$proc.ProcessName; pid=$proc.Id; method='Failed'; error=$_.Exception.Message }
            }
        }
    }
    Start-Sleep -Milliseconds 250
    $remaining = @($names | ForEach-Object { @(Get-Process -Name $_ -ErrorAction SilentlyContinue) } | Sort-Object Id -Unique)
    if ($remaining.Count -eq 0) { break }
}

if ($remaining.Count -gt 0) {
    foreach ($proc in $remaining) {
        $stopped += [pscustomobject]@{ name=$proc.ProcessName; pid=$proc.Id; method='Failed'; error='Process remained after bounded shutdown settle period.' }
    }
}

$failed = @($stopped | Where-Object method -eq 'Failed')
$result = [pscustomobject]@{ stopped=($failed.Count -eq 0 -and $remaining.Count -eq 0); alreadyStopped=$false; forced=$true; processes=$stopped; remainingProcesses=@($remaining | ForEach-Object { [pscustomobject]@{ name=$_.ProcessName; pid=$_.Id } }) }
if ($Json) { $result | ConvertTo-Json -Depth 5 } else {
    $stopped | Format-Table -AutoSize
    if ($failed.Count -eq 0) { Write-Host 'Known shard processes stopped.' } else { Write-Host "$($failed.Count) process(es) could not be stopped." }
}
if ($failed.Count -gt 0) { exit 1 }
exit 0
