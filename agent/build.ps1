[CmdletBinding()]
param(
    [ValidateSet('Debug','Release')]
    [string]$Configuration = 'Release',
    [ValidateSet('x86','x64')]
    [string]$Platform = 'x86',
    [switch]$Clean,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$solution = Join-Path $repoRoot 'build\vs2019\master.sln'
$logDir = Join-Path $PSScriptRoot 'logs'
New-Item -ItemType Directory -Path $logDir -Force | Out-Null
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$logPath = Join-Path $logDir "build-$Configuration-$Platform-$stamp.log"

function Find-MSBuild {
    $cmd = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path $vswhere) {
        return (& $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' 2>$null | Select-Object -First 1)
    }
    return $null
}

$msbuild = Find-MSBuild
if (-not $msbuild) {
    Write-Error 'MSBuild was not found. Run .\agent\doctor.ps1 for environment diagnostics.'
    exit 2
}
if (-not (Test-Path $solution)) {
    Write-Error "Solution not found: $solution"
    exit 2
}

$target = if ($Clean) { 'Rebuild' } else { 'Build' }
$args = @(
    $solution,
    '/m',
    '/p:BuildInParallel=true',
    "/p:Configuration=$Configuration",
    "/p:Platform=$Platform",
    "/t:$target",
    '/nologo',
    '/verbosity:minimal'
)

$started = Get-Date
Write-Host "Building $Configuration/$Platform using $msbuild"
Write-Host "Full log: $logPath"

& $msbuild @args 2>&1 | Tee-Object -FilePath $logPath | ForEach-Object {
    if ($_ -match ': error | error [A-Z]+[0-9]+:|Build FAILED|Build succeeded') { Write-Host $_ }
}
$exitCode = $LASTEXITCODE
$elapsed = [math]::Round(((Get-Date) - $started).TotalSeconds, 1)

$errors = @()
if (Test-Path $logPath) {
    $errors = @(Select-String -Path $logPath -Pattern ': error | error [A-Z]+[0-9]+:' | Select-Object -First 10 | ForEach-Object { $_.Line.Trim() })
}

$result = [pscustomobject]@{
    success = ($exitCode -eq 0)
    configuration = $Configuration
    platform = $Platform
    target = $target
    exitCode = $exitCode
    durationSeconds = $elapsed
    log = $logPath
    firstErrors = $errors
}

if ($Json) {
    $result | ConvertTo-Json -Depth 4
} else {
    Write-Host ''
    if ($exitCode -eq 0) {
        Write-Host "BUILD PASS ($Configuration/$Platform) - ${elapsed}s"
    } else {
        Write-Host "BUILD FAIL ($Configuration/$Platform) - exit $exitCode"
        if ($errors.Count -gt 0) {
            Write-Host 'First useful errors:'
            $errors | ForEach-Object { Write-Host "  $_" }
        } else {
            Write-Host 'No compiler-style error was extracted; inspect the full log.'
        }
    }
}

exit $exitCode
