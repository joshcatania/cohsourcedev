[CmdletBinding()]
param(
    [int]$TailLines = 40
)

$ErrorActionPreference = 'SilentlyContinue'
$repoRoot = Split-Path -Parent $PSScriptRoot
$binDir = Join-Path $repoRoot 'bin'
$configPath = Join-Path $binDir 'etc\config.txt'
$logRoot = Join-Path $binDir 'logs'

Write-Host 'COH AUTHSERVER DIAGNOSTICS'
Write-Host ''

$auth = @(Get-Process -Name AuthServer -ErrorAction SilentlyContinue)
if ($auth.Count -eq 0) {
    Write-Host '[FAIL] AuthServer process is not running.'
    exit 1
}

foreach ($p in $auth) {
    Write-Host ("AuthServer PID: {0}" -f $p.Id)
    Write-Host ("Started: {0}" -f $p.StartTime)
    try {
        $cim = Get-CimInstance Win32_Process -Filter "ProcessId=$($p.Id)"
        if ($cim) {
            Write-Host ("Executable: {0}" -f $cim.ExecutablePath)
            Write-Host ("Command line: {0}" -f $cim.CommandLine)
        }
    } catch {}

    Write-Host ''
    Write-Host 'TCP listeners owned by AuthServer:'
    $listeners = @(Get-NetTCPConnection -OwningProcess $p.Id -State Listen -ErrorAction SilentlyContinue | Sort-Object LocalPort)
    if ($listeners.Count -eq 0) {
        Write-Host '  <none>'
    } else {
        foreach ($l in $listeners) {
            Write-Host ("  {0}:{1}" -f $l.LocalAddress, $l.LocalPort)
        }
    }
}

Write-Host ''
Write-Host 'Expected Auth ports from repo configuration/code:'
if (Test-Path $configPath) {
    $cfg = Get-Content $configPath
    foreach ($pattern in @('serverPort\s*=','serverExPort\s*=','serverIntPort\s*=','connectionString\s*=')) {
        $matches = @($cfg | Where-Object { $_ -match $pattern })
        foreach ($m in $matches) { Write-Host ("  {0}" -f $m.Trim()) }
    }
} else {
    Write-Host ("  Config missing: {0}" -f $configPath)
}

Write-Host ''
foreach ($port in @(2104,2106,2108)) {
    $open = $false
    try {
        $client = New-Object System.Net.Sockets.TcpClient
        $iar = $client.BeginConnect('127.0.0.1', $port, $null, $null)
        $open = $iar.AsyncWaitHandle.WaitOne(800, $false)
        if ($open) {
            try { $client.EndConnect($iar) } catch { $open = $false }
        }
        $client.Close()
    } catch { $open = $false }
    Write-Host ("127.0.0.1:{0} accepting TCP: {1}" -f $port, $open)
}

Write-Host ''
Write-Host 'Newest runtime log files:'
$logs = @()
if (Test-Path $logRoot) {
    $logs = @(Get-ChildItem $logRoot -Recurse -File -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 12)
}
if ($logs.Count -eq 0) {
    Write-Host '  <no files under bin\logs>'
} else {
    foreach ($f in $logs) {
        Write-Host ("  {0}  {1}  {2}" -f $f.LastWriteTime.ToString('yyyy-MM-dd HH:mm:ss'), $f.Length, $f.FullName)
    }
}

$interesting = @($logs | Where-Object { $_.Name -match 'auth|server|error|log' } | Select-Object -First 5)
if ($interesting.Count -gt 0) {
    Write-Host ''
    Write-Host 'Recent useful log tails:'
    foreach ($f in $interesting) {
        Write-Host ''
        Write-Host ("--- {0} ---" -f $f.FullName)
        $tail = @(Get-Content $f.FullName -Tail $TailLines -ErrorAction SilentlyContinue)
        if ($tail.Count -eq 0) {
            Write-Host '<empty/unreadable>'
        } else {
            $tail | ForEach-Object { Write-Host $_ }
        }
    }
}

Write-Host ''
Write-Host 'TIP: If AuthServer owns no listening ports, the log tail above is the primary clue. Do not treat process presence as readiness.'
