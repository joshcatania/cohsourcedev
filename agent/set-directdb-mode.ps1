[CmdletBinding()]
param(
    [switch]$Enable,
    [switch]$Disable,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$configPath = Join-Path $repoRoot 'bin\data\server\db\servers.cfg'

if ($Enable -and $Disable) {
    Write-Error 'Choose only one of -Enable or -Disable.'
    exit 2
}

if (-not (Test-Path -LiteralPath $configPath)) {
    Write-Error "DbServer config not found: $configPath"
    exit 2
}

function Get-DirectDbState([string]$Text) {
    $fakeAuth = [bool]($Text -match '(?im)^\s*UseFakeAuth\s+1\s*(?://.*)?$')
    $authServer = [bool]($Text -match '(?im)^\s*AuthServer\s+\S+')
    [pscustomobject]@{
        fakeAuthEnabled = $fakeAuth
        activeAuthServer = $authServer
        mode = if ($fakeAuth -and -not $authServer) { 'direct-db' } elseif ($authServer -and -not $fakeAuth) { 'auth-server' } else { 'invalid' }
    }
}

$beforeText = Get-Content -LiteralPath $configPath -Raw
$before = Get-DirectDbState $beforeText
$changed = $false
$restartRequired = $false
$action = 'status'
$afterText = $beforeText

if ($Enable) {
    $action = 'enable'
    $authMatches = [regex]::Matches($afterText, '(?im)^\s*AuthServer\s+127\.0\.0\.1\s+2104\s*$')
    if ($authMatches.Count -gt 1) { throw 'Refusing to change servers.cfg: multiple exact AuthServer 127.0.0.1 2104 directives were found.' }
    if ($authMatches.Count -eq 1) {
        $afterText = [regex]::Replace($afterText, '(?im)^(\s*)AuthServer\s+127\.0\.0\.1\s+2104\s*$', '$1// AuthServer 127.0.0.1 2104', 1)
    } elseif ($before.activeAuthServer -and $before.mode -ne 'direct-db') {
        throw 'Refusing to change servers.cfg: the active AuthServer directive is not the expected local 127.0.0.1:2104 entry.'
    }
    $fakeMatches = [regex]::Matches($afterText, '(?im)^\s*UseFakeAuth\s+(?:0|1)\s*(?://.*)?$')
    if ($fakeMatches.Count -ne 1) { throw "Refusing to change servers.cfg: expected exactly one active UseFakeAuth directive, found $($fakeMatches.Count)." }
    $afterText = [regex]::Replace($afterText, '(?im)^(\s*)UseFakeAuth\s+(?:0|1)(\s*(?://.*)?)$', '$1UseFakeAuth 1$2', 1)
} elseif ($Disable) {
    $action = 'disable'
    $authMatches = [regex]::Matches($afterText, '(?im)^\s*AuthServer\s+\S+')
    if ($authMatches.Count -gt 1) { throw 'Refusing to change servers.cfg: multiple active AuthServer directives were found.' }
    if ($authMatches.Count -eq 0) {
        $commentedMatches = [regex]::Matches($afterText, '(?im)^\s*//\s*AuthServer\s+127\.0\.0\.1\s+2104\s*$')
        if ($commentedMatches.Count -ne 1) { throw 'Refusing to change servers.cfg: the expected commented local AuthServer directive was not found.' }
        $afterText = [regex]::Replace($afterText, '(?im)^(\s*)//\s*AuthServer\s+127\.0\.0\.1\s+2104\s*$', '$1AuthServer 127.0.0.1 2104', 1)
    }
    $fakeMatches = [regex]::Matches($afterText, '(?im)^\s*UseFakeAuth\s+(?:0|1)\s*(?://.*)?$')
    if ($fakeMatches.Count -ne 1) { throw "Refusing to change servers.cfg: expected exactly one active UseFakeAuth directive, found $($fakeMatches.Count)." }
    $afterText = [regex]::Replace($afterText, '(?im)^(\s*)UseFakeAuth\s+(?:0|1)(\s*(?://.*)?)$', '$1UseFakeAuth 0$2', 1)
}

$after = Get-DirectDbState $afterText
if ($Enable -and $after.mode -ne 'direct-db') { throw "Direct-DB enable would leave servers.cfg in invalid mode: $($after.mode)." }
if ($Disable -and $after.mode -ne 'auth-server') { throw "Auth-server restore would leave servers.cfg in invalid mode: $($after.mode)." }

if ($afterText -cne $beforeText) {
    $tempPath = "$configPath.tmp.$PID"
    Set-Content -LiteralPath $tempPath -Value $afterText -Encoding UTF8
    Move-Item -LiteralPath $tempPath -Destination $configPath -Force
    $changed = $true
    $restartRequired = $true
}

$result = [pscustomobject]@{
    action = $action
    changed = $changed
    restartRequired = $restartRequired
    config = $configPath
    before = $before
    after = $after
    note = 'Restart ServerMonitor after changing mode; running servers keep their existing configuration.'
}

if ($Json) {
    $result | ConvertTo-Json -Depth 5
} else {
    Write-Host ("Mode: {0} -> {1}" -f $before.mode, $after.mode)
    if ($changed) { Write-Host "Updated $configPath" } else { Write-Host 'No change needed.' }
    if ($restartRequired) { Write-Host 'Restart ServerMonitor for the new mode to take effect.' }
}

if ($after.mode -eq 'invalid') { exit 1 }
exit 0
