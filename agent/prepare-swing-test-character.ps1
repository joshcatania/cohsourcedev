[CmdletBinding()]
param(
    [string]$DbAddress = '127.0.0.1',
    [string]$AccountName = 'Dummy00009',
    [string]$Password = '11111111',
    [string]$CharacterName = 'SwingTest',
    [int]$ReadinessTimeoutSeconds = 300,
    [int]$SmokeTimeoutSeconds = 180,
    [int]$TimeoutSeconds = 120
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$binDir = Join-Path $repoRoot 'bin'
$testClient = Join-Path $binDir 'TestClient.exe'
$directDbScript = Join-Path $PSScriptRoot 'set-directdb-mode.ps1'
$startScript = Join-Path $PSScriptRoot 'start-shard.ps1'
$smokeScript = Join-Path $PSScriptRoot 'smoke.ps1'
$logDir = Join-Path $PSScriptRoot 'logs'
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$stdoutLog = Join-Path $logDir "swing-test-prep-$stamp.out.log"
$stderrLog = Join-Path $logDir "swing-test-prep-$stamp.err.log"
$statusLog = Join-Path $logDir "swing-test-prep-$stamp.status"

if ($AccountName -notmatch '^[A-Za-z][A-Za-z0-9_]{1,31}$') {
    throw "AccountName must contain only letters, digits, and underscores: $AccountName"
}
if ($CharacterName -notmatch '^[A-Za-z][A-Za-z0-9_]{1,31}$') {
    throw "CharacterName must contain only letters, digits, and underscores: $CharacterName"
}
if (-not (Test-Path -LiteralPath $testClient)) {
    throw "TestClient.exe was not found at $testClient. Run .\agent\build.ps1 first."
}

function Get-ShardProcessState {
    $required = @('ServerMonitor', 'DbServer', 'Launcher')
    $missing = @($required | Where-Object { -not (Get-Process -Name $_ -ErrorAction SilentlyContinue) })
    [pscustomobject]@{ Healthy = ($missing.Count -eq 0); Missing = $missing }
}

function Invoke-ExistingScript {
    param([string]$Path, [string[]]$Arguments)
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Path @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$(Split-Path -Leaf $Path) failed with exit code $LASTEXITCODE." }
}

function Read-AgentStatus([string]$Path) {
    $status = @{}
    if (Test-Path -LiteralPath $Path) {
        foreach ($line in (Get-Content -LiteralPath $Path -ErrorAction SilentlyContinue)) {
            $parts = $line -split '=', 2
            if ($parts.Count -eq 2) { $status[$parts[0]] = $parts[1] }
        }
    }
    return $status
}

$sqlcmd = Get-Command sqlcmd.exe -ErrorAction SilentlyContinue
if (-not $sqlcmd) {
    $knownSqlcmd = 'C:\Program Files\Microsoft SQL Server\Client SDK\ODBC\170\Tools\Binn\SQLCMD.EXE'
    if (Test-Path -LiteralPath $knownSqlcmd) { $sqlcmd = Get-Item -LiteralPath $knownSqlcmd }
}
if (-not $sqlcmd) { throw 'sqlcmd.exe is required to persist and verify the local development access level.' }
$sqlcmdPath = if ($sqlcmd.PSObject.Properties['Source']) { $sqlcmd.Source } else { $sqlcmd.FullName }

function Invoke-Sql([string]$Query) {
    $output = & $sqlcmdPath -S localhost -E -d cohdb -W -h -1 -s '|' -Q $Query 2>&1
    if ($LASTEXITCODE -ne 0) { throw "SQL query failed: $($output -join "`n")" }
    return @($output)
}

$sqlAccount = $AccountName.Replace("'", "''")
$sqlCharacter = $CharacterName.Replace("'", "''")

try {
    $beforeMode = Get-ShardProcessState
    $modeOutput = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $directDbScript -Enable 2>&1
    $modeExit = $LASTEXITCODE
    if ($modeExit -ne 0) { throw 'Could not enable direct-DB mode.' }
    $modeChanged = [bool]($modeOutput -match 'Restart ServerMonitor')
    if ($modeChanged -and $beforeMode.Healthy) {
        throw 'Direct-DB mode changed while the shard was already running. Stop and restart the local shard once, then retry.'
    }

    if (-not (Get-ShardProcessState).Healthy) {
        Invoke-ExistingScript -Path $startScript -Arguments @('-StartupWaitSeconds', '30')
    }

    $existingTargetRows = Invoke-Sql "SELECT TOP 1 ContainerId FROM dbo.Ents WHERE Name = N'$sqlCharacter' AND AuthName = N'$sqlAccount';"
    $targetAlreadyExists = @($existingTargetRows | Where-Object { $_ -and $_.ToString().Trim() -and $_.ToString() -notmatch 'rows affected' }).Count -gt 0

    $deadline = [DateTime]::UtcNow.AddSeconds([math]::Max(1, $ReadinessTimeoutSeconds))
    do {
        $smokeOutput = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $smokeScript -DbAddress $DbAddress -AccountName $AccountName -TimeoutSeconds $SmokeTimeoutSeconds -Json 2>&1
        $smokeExit = $LASTEXITCODE
        $smokeResult = $null
        try { $smokeResult = ($smokeOutput -join "`n") | ConvertFrom-Json } catch {}
        if ($smokeExit -eq 0 -and $smokeResult -and $smokeResult.passed) { break }
        if ([DateTime]::UtcNow -ge $deadline) {
            throw "Shard did not become login-ready within $ReadinessTimeoutSeconds seconds."
        }
        Start-Sleep -Seconds 5
    } while ($true)

    if (Get-Process -Name TestClient -ErrorAction SilentlyContinue) {
        throw 'A TestClient process is already running; stop it before preparing SwingTest.'
    }

    $argsList = @(
        '-db', $DbAddress,
        '-authname', $AccountName,
        '-password', $Password,
        '-dontpause',
        '-selfversion',
        '-nosharedmemory',
        '-silent',
        '-swing-test-prep', $CharacterName
    )
    if ($targetAlreadyExists) {
        $argsList += @('-character', $CharacterName)
    }
    $argsList += @(
        '-agent-status', $statusLog
    )

    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $testClient
    $psi.WorkingDirectory = $binDir
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.Arguments = (($argsList | ForEach-Object {
        if ($_ -match '[\s"]') { '"' + ($_ -replace '"','\"') + '"' } else { $_ }
    }) -join ' ')

    Write-Host "Preparing $CharacterName on $AccountName..."
    $proc = New-Object System.Diagnostics.Process
    $proc.StartInfo = $psi
    if (-not $proc.Start()) { throw 'Could not start TestClient.' }
    $stdoutTask = $proc.StandardOutput.ReadToEndAsync()
    $stderrTask = $proc.StandardError.ReadToEndAsync()
    $exited = $proc.WaitForExit([math]::Max(1, $TimeoutSeconds) * 1000)
    if (-not $exited) {
        try { $proc.Kill() } catch {}
        $proc.WaitForExit()
        throw "Swing test prep did not exit within $TimeoutSeconds seconds."
    }
    $stdoutText = $stdoutTask.Result
    $stderrText = $stderrTask.Result
    Set-Content -LiteralPath $stdoutLog -Value $stdoutText -Encoding UTF8
    Set-Content -LiteralPath $stderrLog -Value $stderrText -Encoding UTF8
    if ($proc.ExitCode -ne 0) { throw "TestClient exited with code $($proc.ExitCode)." }

    $status = Read-AgentStatus $statusLog
    if ($status['swing_test_prep_complete'] -ne '1') { throw 'TestClient did not report a completed prep sequence.' }
    if ($status['character'] -ne $CharacterName) { throw "Prepared character was '$($status['character'])', not '$CharacterName'." }
    if ($status['swing_test_prep_level'] -ne '50') { throw "TestClient reported XP-calculated level $($status['swing_test_prep_level']), not 50." }
    if ($status['swing_test_prep_fly_owned'] -ne '1') { throw 'TestClient did not confirm owned Fly from the loaded power dictionary.' }

    # Keep the development character elevated across reconnects. This only
    # changes the requested local test character; it does not alter defaults.
    Invoke-Sql "UPDATE dbo.Ents SET AccessLevel = 9 WHERE Name = N'$sqlCharacter' AND AuthName = N'$sqlAccount';" | Out-Null
    $entRows = Invoke-Sql "SELECT TOP 1 ContainerId,Name,AuthName,AccessLevel,StaticMapId FROM dbo.Ents WHERE Name = N'$sqlCharacter' AND AuthName = N'$sqlAccount';"
    $entRow = @($entRows | Where-Object { $_ -and $_.ToString().Trim() -and $_.ToString() -notmatch 'rows affected' } | Select-Object -First 1)
    if ($entRow.Count -eq 0) { throw 'SwingTest was not found in dbo.Ents after prep.' }
    $entFields = $entRow[0].ToString().Trim() -split '\|'
    if ($entFields.Count -lt 5 -or $entFields[1] -ne $CharacterName -or $entFields[3] -ne '9') {
        throw "Unexpected persisted SwingTest row: $($entRow[0])"
    }

    $powerRows = Invoke-Sql "SELECT COUNT(*) FROM dbo.Powers p INNER JOIN dbo.Ents e ON e.ContainerId = p.ContainerId WHERE e.Name = N'$sqlCharacter' AND e.AuthName = N'$sqlAccount';"
    $powerCountText = @($powerRows | Where-Object { $_ -and $_.ToString().Trim() } | Select-Object -First 1)
    $powerCount = 0
    if ($powerCountText.Count -gt 0) { [int]::TryParse($powerCountText[0].ToString().Trim(), [ref]$powerCount) | Out-Null }
    if ($powerCount -le 0) { throw 'No persisted dbo.Powers rows were found for SwingTest.' }

    Write-Host "SwingTest ready: XP-calculated level 50, real Mission_Maker_Movement.Flight / Fly owned, AccessLevel 9 persisted."
    Write-Host "Prep stdout: $stdoutLog"
    Write-Host "Prep stderr: $stderrLog"
    Write-Host "Prep status: $statusLog"
    exit 0
} catch {
    Write-Host "PREP-SWING-TEST-CHAR ERROR: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "Prep stdout: $stdoutLog"
    Write-Host "Prep stderr: $stderrLog"
    if (Test-Path -LiteralPath $statusLog) { Write-Host "Prep status: $statusLog" }
    exit 1
}
