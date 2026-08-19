[CmdletBinding()]
param(
    [string]$AccountName = 'Dummy00018',
    [string]$Password = '11111111',
    [int]$ReadinessTimeoutSeconds = 300,
    [int]$SmokeTimeoutSeconds = 90,
    [int]$RetryDelaySeconds = 5,
    [switch]$SkipReadinessSmoke
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$binDir = Join-Path $repoRoot 'bin'
$ouroboros = Join-Path $binDir 'Ouroboros.exe'
$directDbScript = Join-Path $PSScriptRoot 'set-directdb-mode.ps1'
$startScript = Join-Path $PSScriptRoot 'start-shard.ps1'
$statusScript = Join-Path $PSScriptRoot 'status.ps1'
$smokeScript = Join-Path $PSScriptRoot 'smoke.ps1'

function Invoke-ExistingScript {
    param([string]$Path, [string[]]$Arguments)
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Path @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$(Split-Path -Leaf $Path) failed with exit code $LASTEXITCODE." }
}

function Get-ShardProcessState {
    $required = @('ServerMonitor', 'DbServer', 'Launcher')
    $missing = @($required | Where-Object { -not (Get-Process -Name $_ -ErrorAction SilentlyContinue) })
    [pscustomobject]@{
        Healthy = ($missing.Count -eq 0)
        Missing = $missing
    }
}

function Show-StatusSnapshot {
    if (Test-Path -LiteralPath $statusScript) {
        & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $statusScript
    }
}

try {
    if (-not (Test-Path -LiteralPath $ouroboros)) { throw "Ouroboros.exe was not found at $ouroboros. Build the client first." }

    Write-Host 'Starting local shard...'
    $beforeMode = Get-ShardProcessState
    $modeOutput = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $directDbScript -Enable 2>&1
    $modeExit = $LASTEXITCODE
    $modeOutput | ForEach-Object { if ($_ -match 'Mode:|No change|Restart') { Write-Host $_ } }
    if ($modeExit -ne 0) { throw 'Could not enable direct-DB mode. See the mode configuration error above.' }

    $modeChanged = [bool]($modeOutput -match 'Restart ServerMonitor')
    if ($modeChanged -and $beforeMode.Healthy) {
        throw 'Direct-DB mode was changed while the shard was already running. Stop and restart the local shard once, then run PLAY-COH.cmd again.'
    }

    $state = Get-ShardProcessState
    if (-not $state.Healthy) {
        Invoke-ExistingScript -Path $startScript -Arguments @('-StartupWaitSeconds', '30')
    } else {
        Write-Host 'Local shard is already running; leaving it in place.'
    }

    if (-not $SkipReadinessSmoke) {
        Write-Host 'Waiting for login readiness...'
        $deadline = [DateTime]::UtcNow.AddSeconds([math]::Max(1, $ReadinessTimeoutSeconds))
        $attempt = 0
        $lastReason = 'No readiness attempt completed.'
        do {
            $attempt++
            $state = Get-ShardProcessState
            if (-not $state.Healthy) {
                $lastReason = "Missing shard process(es): $($state.Missing -join ', ')."
            } else {
                $smokeOutput = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $smokeScript -AccountName $AccountName -TimeoutSeconds $SmokeTimeoutSeconds -Json 2>&1
                $smokeExit = $LASTEXITCODE
                $smokeResult = $null
                try { $smokeResult = ($smokeOutput -join "`n") | ConvertFrom-Json } catch {}
                if ($smokeExit -eq 0 -and $smokeResult -and $smokeResult.passed) {
                    Write-Host 'Shard ready.'
                    break
                }
                if ($smokeResult.reason) { $lastReason = $smokeResult.reason } else { $lastReason = 'Direct-DB readiness smoke did not pass.' }
            }
            if ([DateTime]::UtcNow -ge $deadline) {
                Show-StatusSnapshot
                throw "Shard did not become login-ready within $ReadinessTimeoutSeconds seconds. Last check: $lastReason"
            }
            Write-Host ("  Readiness attempt {0} not ready; retrying..." -f $attempt)
            Start-Sleep -Seconds ([math]::Max(1, $RetryDelaySeconds))
        } while ($true)
    } else {
        Write-Host 'Readiness smoke skipped by request.'
    }

    $existingClient = Get-Process -Name Ouroboros -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($existingClient) {
        Write-Host "City of Heroes is already running (PID $($existingClient.Id)); leaving it alone."
        exit 0
    }

    Write-Host 'Launching City of Heroes...'
    $clientArgs = @('-db', '127.0.0.1', '-authname', $AccountName, '-password', $Password, '-noverify', '-quicklogin', '1', '-noversioncheck', '-fullscreen', '0', '-screen', '1280', '720', '-stopinactivedisplay', '0')
    $client = Start-Process -FilePath $ouroboros -ArgumentList $clientArgs -WorkingDirectory $binDir -PassThru
    Start-Sleep -Milliseconds 1500
    if (-not (Get-Process -Id $client.Id -ErrorAction SilentlyContinue)) {
        throw "Ouroboros.exe exited immediately after launch. Inspect the client/runtime logs under $binDir."
    }
    Write-Host "City of Heroes launched (PID $($client.Id))."
    exit 0
} catch {
    Write-Host ''
    Write-Host "PLAY-COH ERROR: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host 'Check the shard status and logs, then retry PLAY-COH.cmd.' -ForegroundColor Yellow
    exit 1
}
