[CmdletBinding()]
param(
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$results = New-Object System.Collections.Generic.List[object]

function Add-Check {
    param(
        [string]$Name,
        [bool]$Pass,
        [string]$Detail,
        [string]$Fix = ''
    )
    $results.Add([pscustomobject]@{
        name = $Name
        pass = $Pass
        detail = $Detail
        fix = $Fix
    })
}

function Find-MSBuild {
    $cmd = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }

    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path $vswhere) {
        $path = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' 2>$null | Select-Object -First 1
        if ($path) { return $path }
    }
    return $null
}

Push-Location $repoRoot
try {
    $git = Get-Command git.exe -ErrorAction SilentlyContinue
    Add-Check 'Git' ([bool]$git) $(if ($git) { (& git --version) } else { 'git.exe not found' }) 'Install Git for Windows and ensure git.exe is on PATH.'

    $solution = Join-Path $repoRoot 'build\vs2019\master.sln'
    Add-Check 'VS2019 master solution' (Test-Path $solution) $solution 'Restore the repository build/vs2019 tree.'

    $msbuild = Find-MSBuild
    Add-Check 'MSBuild' ([bool]$msbuild) $(if ($msbuild) { $msbuild } else { 'MSBuild not found' }) 'Install Visual Studio 2019/2022 Build Tools with Desktop development with C++ and the v142 toolset.'

    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    $v142Found = $false
    $winSdkFound = $false
    if (Test-Path $vswhere) {
        $installations = & $vswhere -products * -format json | ConvertFrom-Json
        foreach ($install in $installations) {
            $root = $install.installationPath
            if (Test-Path (Join-Path $root 'VC\Tools\MSVC')) {
                $v142Found = $v142Found -or [bool](Get-ChildItem (Join-Path $root 'VC\Tools\MSVC') -Directory -ErrorAction SilentlyContinue | Where-Object { $_.Name -like '14.2*' })
            }
        }
    }
    $kits = 'HKLM:\SOFTWARE\Microsoft\Windows Kits\Installed Roots'
    if (Test-Path $kits) {
        $kitRoot = (Get-ItemProperty $kits -ErrorAction SilentlyContinue).KitsRoot10
        if ($kitRoot -and (Test-Path (Join-Path $kitRoot 'Lib'))) { $winSdkFound = $true }
    }
    Add-Check 'MSVC v142 toolset' $v142Found $(if ($v142Found) { 'v142 toolset detected' } else { 'v142 toolset not detected' }) 'Add the MSVC v142 build tools component in Visual Studio Installer.'
    Add-Check 'Windows 10 SDK' $winSdkFound $(if ($winSdkFound) { 'Windows 10 SDK detected' } else { 'Windows 10 SDK not detected' }) 'Install a Windows 10 SDK through Visual Studio Installer.'

    $expectedSubmodules = @('bin/data/server/maps', 'bin/piggs')
    foreach ($sub in $expectedSubmodules) {
        $path = Join-Path $repoRoot ($sub -replace '/', '\')
        $hasFiles = (Test-Path $path) -and [bool](Get-ChildItem $path -Force -ErrorAction SilentlyContinue | Select-Object -First 1)
        Add-Check "Submodule: $sub" $hasFiles $(if ($hasFiles) { 'content present' } else { 'missing or empty' }) 'Run: git submodule update --init --recursive'
    }

    # The checked-in AuthServer config explicitly names "ODBC Driver 17 for SQL Server".
    # Release|x86 is the currently validated baseline, so the 32-bit driver is required.
    # Microsoft's x64 Driver 17 installer installs both 64-bit and 32-bit drivers on x64 Windows.
    $odbc17x86 = $false
    $odbc17x64 = $false
    try {
        $odbc17x86 = [bool](Get-OdbcDriver -Platform '32-bit' -ErrorAction SilentlyContinue | Where-Object Name -EQ 'ODBC Driver 17 for SQL Server')
        $odbc17x64 = [bool](Get-OdbcDriver -Platform '64-bit' -ErrorAction SilentlyContinue | Where-Object Name -EQ 'ODBC Driver 17 for SQL Server')
    } catch {}
    Add-Check 'SQL Server ODBC Driver 17 (32-bit)' $odbc17x86 $(if ($odbc17x86) { 'ODBC Driver 17 detected for 32-bit processes' } else { 'ODBC Driver 17 not detected for 32-bit processes' }) 'Install Microsoft ODBC Driver 17 for SQL Server. On 64-bit Windows, use Microsoft\'s x64 Driver 17 installer; it installs both 64-bit and 32-bit drivers.'
    Add-Check 'SQL Server ODBC Driver 17 (64-bit)' $odbc17x64 $(if ($odbc17x64) { 'ODBC Driver 17 detected for 64-bit processes' } else { 'ODBC Driver 17 not detected for 64-bit processes' }) 'Install Microsoft ODBC Driver 17 for SQL Server.'

    foreach ($rel in @('bin\data\server\db', 'bin\etc', 'bin')) {
        $path = Join-Path $repoRoot $rel
        Add-Check "Runtime path: $rel" (Test-Path $path) $path 'Ensure repository runtime data is present.'
    }

    $requiredBuildInputs = @(
        'Utilities\TestClient\build\vs2019\TestClient.vcxproj',
        'ServerMonitor\build\vs2019\ServerMonitor.vcxproj'
    )
    foreach ($rel in $requiredBuildInputs) {
        $path = Join-Path $repoRoot $rel
        Add-Check "Build input: $rel" (Test-Path $path) $path 'Restore missing source/build files from git.'
    }
}
finally {
    Pop-Location
}

$failed = @($results | Where-Object { -not $_.pass })
if ($Json) {
    [pscustomobject]@{
        ready = ($failed.Count -eq 0)
        repoRoot = $repoRoot
        checks = $results
    } | ConvertTo-Json -Depth 5
} else {
    Write-Host 'COH DEVELOPMENT DOCTOR'
    Write-Host ''
    foreach ($r in $results) {
        $label = if ($r.pass) { 'PASS' } else { 'FAIL' }
        Write-Host ('[{0}] {1} - {2}' -f $label, $r.name, $r.detail)
        if (-not $r.pass -and $r.fix) { Write-Host ('       Fix: {0}' -f $r.fix) }
    }
    Write-Host ''
    if ($failed.Count -eq 0) { Write-Host 'READY' } else { Write-Host ("NOT READY - {0} check(s) failed" -f $failed.Count) }
}

if ($failed.Count -gt 0) { exit 1 }
exit 0
