[CmdletBinding()]
param(
    [ValidateSet('Client', 'FastDev', 'Full')]
    [string]$Scope = 'Client',
    [string]$AccountName = 'Dummy00018',
    [string]$Password = '11111111'
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$buildScript = Join-Path $PSScriptRoot 'build.ps1'
$clientBuildScript = Join-Path $PSScriptRoot 'build-client.ps1'
$profileScript = Join-Path $PSScriptRoot 'set-shard-profile.ps1'
$stopScript = Join-Path $PSScriptRoot 'stop-shard.ps1'
$playScript = Join-Path $PSScriptRoot 'play-local.ps1'

function Invoke-ExistingScript {
    param([string]$Path, [string[]]$Arguments)
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Path @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$(Split-Path -Leaf $Path) failed with exit code $LASTEXITCODE." }
}
function Invoke-JsonScript {
    param([string]$Path, [string[]]$Arguments)
    $output = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Path @Arguments 2>&1)
    if ($LASTEXITCODE -ne 0) { throw "$(Split-Path -Leaf $Path) failed with exit code $LASTEXITCODE. $($output -join ' ')" }
    return (($output | ForEach-Object { $_.ToString() }) -join "`n" | ConvertFrom-Json)
}

if (Get-Process -Name Ouroboros -ErrorAction SilentlyContinue) {
    throw 'Close Ouroboros before rebuilding the client or server binaries.'
}

if ($Scope -eq 'Client') {
    $status = Invoke-JsonScript -Path $profileScript -Arguments @('-Status', '-Json')
    if ($status.profile -eq 'Unknown') { throw 'The running configuration is not a known shard profile. Select FastDev or Full explicitly before rebuilding.' }
    Write-Host "Client/data rebuild: preserving the healthy $($status.profile) shard profile and avoiding a shard restart."
    Invoke-ExistingScript -Path $clientBuildScript -Arguments @('-Configuration', 'Release', '-Platform', 'x86')
    Invoke-ExistingScript -Path $playScript -Arguments @('-AccountName', $AccountName, '-Password', $Password, '-ShardProfile', $status.profile, '-NoShardRestart')
    exit 0
}

Write-Host ("{0} rebuild: stopping the disposable shard before updating locked server binaries." -f $Scope)
Invoke-ExistingScript -Path $stopScript -Arguments @('-ForceProcessStop')
Invoke-ExistingScript -Path $buildScript -Arguments @('-Configuration', 'Release', '-Platform', 'x86')
$playArgs = @('-AccountName', $AccountName, '-Password', $Password, '-ShardProfile', $Scope)
Invoke-ExistingScript -Path $playScript -Arguments $playArgs
exit 0
