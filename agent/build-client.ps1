[CmdletBinding()]
param(
    [ValidateSet('Debug','Release')]
    [string]$Configuration = 'Release',
    [ValidateSet('x86','x64')]
    [string]$Platform = 'x86',
    [ValidateSet('Auto','v142','v145')]
    [string]$Toolset = 'Auto',
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$project = Join-Path $repoRoot 'Game\build\vs2019\Game.vcxproj'
$solution = Join-Path $repoRoot 'build\vs2019\master.sln'
$compatProps = Join-Path $PSScriptRoot 'v145-compat.props'
$logDir = Join-Path $PSScriptRoot 'logs'
New-Item -ItemType Directory -Path $logDir -Force | Out-Null
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$logPath = Join-Path $logDir "build-client-$Configuration-$Platform-$stamp.log"

. (Join-Path $PSScriptRoot 'lib\toolchain.ps1')

function Finish {
    param([object]$Result, [int]$ExitCode)
    if ($Json) {
        $Result | ConvertTo-Json -Depth 5
    } else {
        if ($Result.success) {
            Write-Host ("CLIENT BUILD PASS ({0}/{1}) - {2}s" -f $Configuration, $Platform, $Result.durationSeconds)
        } else {
            Write-Host ("CLIENT BUILD FAIL ({0}/{1}) - exit {2}" -f $Configuration, $Platform, $ExitCode)
            if ($Result.reason) { Write-Host ("Reason: {0}" -f $Result.reason) }
        }
        Write-Host ("Full log: {0}" -f $logPath)
    }
    exit $ExitCode
}

if (-not (Test-Path -LiteralPath $project)) {
    Finish ([pscustomobject]@{
        success = $false; configuration = $Configuration; platform = $Platform
        durationSeconds = 0; exitCode = 2; log = $logPath
        reason = "Game project not found: $project"
    }) 2
}
if (-not (Test-Path -LiteralPath $solution)) {
    Finish ([pscustomobject]@{
        success = $false; configuration = $Configuration; platform = $Platform
        durationSeconds = 0; exitCode = 2; log = $logPath
        reason = "Solution not found: $solution"
    }) 2
}

$client = Get-Process -Name @('Ouroboros', 'Ouroboros_Debug') -ErrorAction SilentlyContinue
if ($client) {
    $pids = (@($client | ForEach-Object { $_.Id }) -join ', ')
    Finish ([pscustomobject]@{
        success = $false; configuration = $Configuration; platform = $Platform
        durationSeconds = 0; exitCode = 3; log = $logPath
        reason = "A City of Heroes client is still running (PID $pids); close it before updating the client binaries."
    }) 3
}

Normalize-COH-ProcessEnvironment
$msbuild = Find-COH-MSBuild
if (-not $msbuild) {
    Finish ([pscustomobject]@{
        success = $false; configuration = $Configuration; platform = $Platform
        durationSeconds = 0; exitCode = 2; log = $logPath
        reason = 'MSBuild was not found. Run .\agent\doctor.ps1 for environment diagnostics.'
    }) 2
}

$requestedToolset = if ($Toolset -eq 'Auto') { 'v142' } else { $Toolset }
$toolsetProbe = Test-COH-MSBuildToolset -MSBuildPath $msbuild -Toolset $requestedToolset -RepoRoot $repoRoot
$selectedToolset = $requestedToolset
if (-not $toolsetProbe.success -and $Toolset -eq 'Auto') {
    $fallbackProbe = Test-COH-MSBuildToolset -MSBuildPath $msbuild -Toolset 'v145' -RepoRoot $repoRoot
    if ($fallbackProbe.success) {
        $toolsetProbe = $fallbackProbe
        $selectedToolset = 'v145'
    } else {
        Finish ([pscustomobject]@{
            success = $false; configuration = $Configuration; platform = $Platform
            durationSeconds = 0; exitCode = 2; log = $logPath
            requestedToolset = $requestedToolset; selectedToolset = $null
            reason = "No usable C++ project toolset was found. v142: $($toolsetProbe.summary); v145: $($fallbackProbe.summary)"
        }) 2
    }
}
if (-not $toolsetProbe.success) {
    Finish ([pscustomobject]@{
        success = $false; configuration = $Configuration; platform = $Platform
        durationSeconds = 0; exitCode = 2; log = $logPath
        requestedToolset = $requestedToolset; selectedToolset = $null
        reason = "No usable C++ project toolset was found: $($toolsetProbe.summary)"
    }) 2
}

$msbuildPlatform = if ($Platform -eq 'x86') { 'Win32' } else { 'x64' }
$target = 'Build'
$args = @(
    $project,
    '/m:1',
    '/p:BuildInParallel=false',
    "/p:Configuration=$Configuration",
    "/p:Platform=$msbuildPlatform",
    "/p:PlatformToolset=$selectedToolset",
    "/p:CohBuildToolset=$selectedToolset",
    "/p:SolutionPath=$solution",
    # The legacy event also copies shared PhysX DLLs into bin. Those DLLs are
    # loaded by the warmed shard, so leave them in place for a client-only
    # iteration and copy only the client outputs below.
    '/p:PostBuildEventUseInBuild=false',
    "/t:$target",
    '/nologo',
    '/verbosity:minimal'
)
if ($selectedToolset -eq 'v145') {
    $args += "/p:ForceImportAfterCppTargets=$compatProps"
}

$started = Get-Date
$exitCode = 1
$copySucceeded = $false
$reason = $null
$errors = @()
try {
    Write-Host "Building Game client $Configuration/$Platform using $msbuild"
    Write-Host "Requested toolset: $requestedToolset; selected toolset: $selectedToolset"
    if ($selectedToolset -ne $requestedToolset) { Write-Host "Toolset fallback reason: $($toolsetProbe.summary)" }
    Write-Host "Full log: $logPath"

    & $msbuild @args 2>&1 | Tee-Object -FilePath $logPath | ForEach-Object {
        if ($_ -match ': error | error [A-Z]+[0-9]+:|Build FAILED|Build succeeded') { Write-Host $_ }
    }
    $exitCode = $LASTEXITCODE
    if ($exitCode -eq 0) {
        $outputRoot = Join-Path $repoRoot ("Game\bin\{0}\{1}" -f $(if ($Platform -eq 'x86') { 'x86' } else { 'x64' }), $Configuration)
        $clientExe = Join-Path $outputRoot 'Game.exe'
        $clientPdb = Join-Path $outputRoot 'Game.pdb'
        $clientStem = if ($Configuration -eq 'Debug') { 'Ouroboros_Debug' } else { 'Ouroboros' }
        $binExe = Join-Path $repoRoot ("bin\{0}.exe" -f $clientStem)
        $binPdb = Join-Path $repoRoot ("bin\{0}.pdb" -f $clientStem)
        if (-not (Test-Path -LiteralPath $clientExe)) { throw "Game build succeeded but output is missing: $clientExe" }
        Copy-Item -LiteralPath $clientExe -Destination $binExe -Force
        if (Test-Path -LiteralPath $clientPdb) { Copy-Item -LiteralPath $clientPdb -Destination $binPdb -Force }
        $copySucceeded = $true
    }
} catch {
    $exitCode = 1
    $reason = $_.Exception.Message
}
$elapsed = [math]::Round(((Get-Date) - $started).TotalSeconds, 1)
if (Test-Path -LiteralPath $logPath) {
    $errors = @(Select-String -Path $logPath -Pattern ': error | error [A-Z]+[0-9]+:' | Select-Object -First 10 | ForEach-Object { $_.Line.Trim() })
}
$result = [pscustomobject]@{
    success = ($exitCode -eq 0 -and $copySucceeded)
    configuration = $Configuration
    platform = $Platform
    target = $target
    requestedToolset = $requestedToolset
    selectedToolset = $selectedToolset
    toolsetProbe = $toolsetProbe
    exitCode = if ($exitCode -eq 0 -and $copySucceeded) { 0 } else { $exitCode }
    durationSeconds = $elapsed
    clientOutputCopied = $copySucceeded
    log = $logPath
    firstErrors = $errors
    reason = $reason
}
if ($result.success) { Finish $result 0 } else { Finish $result ([int]$(if ($exitCode -eq 0) { 1 } else { $exitCode })) }
