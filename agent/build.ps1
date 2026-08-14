[CmdletBinding()]
param(
    [ValidateSet('Debug','Release')]
    [string]$Configuration = 'Release',
    [ValidateSet('x86','x64')]
    [string]$Platform = 'x86',
    [ValidateSet('Auto','v142','v145')]
    [string]$Toolset = 'Auto',
    [switch]$Clean,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$solution = Join-Path $repoRoot 'build\vs2019\master.sln'
$logDir = Join-Path $PSScriptRoot 'logs'
$compatProps = Join-Path $PSScriptRoot 'v145-compat.props'
New-Item -ItemType Directory -Path $logDir -Force | Out-Null
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$logPath = Join-Path $logDir "build-$Configuration-$Platform-$stamp.log"

. (Join-Path $PSScriptRoot 'lib\toolchain.ps1')

$msbuild = Find-COH-MSBuild
if (-not $msbuild) {
    Write-Error 'MSBuild was not found. Run .\agent\doctor.ps1 for environment diagnostics.'
    exit 2
}
if (-not (Test-Path $solution)) {
    Write-Error "Solution not found: $solution"
    exit 2
}

Normalize-COH-ProcessEnvironment
$requestedToolset = if ($Toolset -eq 'Auto') { 'v142' } else { $Toolset }
$toolsetProbe = Test-COH-MSBuildToolset -MSBuildPath $msbuild -Toolset $requestedToolset -RepoRoot $repoRoot
$selectedToolset = $requestedToolset
if (-not $toolsetProbe.success -and $Toolset -eq 'Auto') {
    $fallbackProbe = Test-COH-MSBuildToolset -MSBuildPath $msbuild -Toolset 'v145' -RepoRoot $repoRoot
    if ($fallbackProbe.success) {
        $toolsetProbe = $fallbackProbe
        $selectedToolset = 'v145'
    } else {
        $toolsetProbe = [pscustomobject]@{
            success = $false
            toolset = 'v142 -> v145'
            exitCode = $fallbackProbe.exitCode
            summary = "v142 probe: $($toolsetProbe.summary); v145 probe: $($fallbackProbe.summary)"
        }
    }
}
if (-not $toolsetProbe.success) {
    $message = "No usable C++ project toolset was found. Requested $requestedToolset. $($toolsetProbe.summary)"
    if ($Json) {
        [pscustomobject]@{
            success = $false
            configuration = $Configuration
            platform = $Platform
            requestedToolset = $requestedToolset
            selectedToolset = $null
            toolsetProbe = $toolsetProbe
        } | ConvertTo-Json -Depth 5
    } else {
        Write-Error $message
    }
    exit 2
}

$target = if ($Clean) { 'Rebuild' } else { 'Build' }
$args = @(
    $solution,
    # Several legacy post-build steps copy shared runtime DLLs into bin. A
    # parallel solution build makes those copies race and surface as the
    # intermittent MSB3073 failures seen in the development loop.
    '/m:1',
    '/p:BuildInParallel=false',
    "/p:Configuration=$Configuration",
    "/p:Platform=$Platform",
    "/p:PlatformToolset=$selectedToolset",
    "/p:CohBuildToolset=$selectedToolset",
    "/t:$target",
    '/nologo',
    '/verbosity:minimal'
)
if ($selectedToolset -eq 'v145') {
    $args += "/p:ForceImportAfterCppTargets=$compatProps"
}

$started = Get-Date
Write-Host "Building $Configuration/$Platform using $msbuild"
Write-Host "Requested toolset: $requestedToolset; selected toolset: $selectedToolset"
if ($selectedToolset -ne $requestedToolset) { Write-Host "Toolset fallback reason: $($toolsetProbe.summary)" }
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
    requestedToolset = $requestedToolset
    selectedToolset = $selectedToolset
    toolsetProbe = $toolsetProbe
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
