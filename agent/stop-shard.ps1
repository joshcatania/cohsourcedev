[CmdletBinding()]
param(
    [switch]$ForceProcessStop,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$names = @('ServerMonitor','AuthServer','DbServer','Launcher','MapServer','AccountServer','ChatServer','AuctionServer','MissionServer','TurnstileServer','QueueServer','StatServer')
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
        $stopped += [pscustomobject]@{ name=$proc.ProcessName; pid=$proc.Id; method='Failed'; error=$_.Exception.Message }
    }
}

$failed = @($stopped | Where-Object method -eq 'Failed')
$result = [pscustomobject]@{ stopped=($failed.Count -eq 0); alreadyStopped=$false; forced=$true; processes=$stopped }
if ($Json) { $result | ConvertTo-Json -Depth 5 } else {
    $stopped | Format-Table -AutoSize
    if ($failed.Count -eq 0) { Write-Host 'Known shard processes stopped.' } else { Write-Host "$($failed.Count) process(es) could not be stopped." }
}
if ($failed.Count -gt 0) { exit 1 }
exit 0
