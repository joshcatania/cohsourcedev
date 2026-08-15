[CmdletBinding()]
param(
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$results = New-Object System.Collections.Generic.List[object]
. (Join-Path $PSScriptRoot 'lib\toolchain.ps1')

function Add-Check {
    param(
        [string]$Name,
        [bool]$Pass,
        [string]$Detail,
        [string]$Fix = '',
        [bool]$Blocking = $true
    )
    $results.Add([pscustomobject]@{
        name = $Name
        pass = $Pass
        blocking = $Blocking
        detail = $Detail
        fix = $Fix
    })
}

Push-Location $repoRoot
try {
    $git = Get-Command git.exe -ErrorAction SilentlyContinue
    Add-Check 'Git' ([bool]$git) $(if ($git) { (& git --version) } else { 'git.exe not found' }) 'Install Git for Windows and ensure git.exe is on PATH.'

    $solution = Join-Path $repoRoot 'build\vs2019\master.sln'
    Add-Check 'VS2019 master solution' (Test-Path $solution) $solution 'Restore the repository build/vs2019 tree.'

    $msbuild = Find-COH-MSBuild
    Add-Check 'MSBuild' ([bool]$msbuild) $(if ($msbuild) { $msbuild } else { 'MSBuild not found' }) 'Install Visual Studio 2019/2022 Build Tools with Desktop development with C++ and the v142 toolset.'

    $v142Compiler = @(Get-COH-V142CompilerInfo)
    $v142CompileFiles = $v142Compiler | Where-Object { $_.compileUsable } | Select-Object -First 1
    $v142CompilerPass = ($null -ne $v142CompileFiles)
    Add-Check 'MSVC v142 compiler files' $v142CompilerPass $(if ($v142CompilerPass) { "compiler $($v142CompileFiles.toolsetVersion) at $($v142CompileFiles.compilerPath); x86 runtime libraries: $($v142CompileFiles.x86RuntimePresent)" } else { 'v142 compiler headers or x86 compiler are missing' }) 'Install the MSVC v142 x86/x64 build tools component.'

    $v142Probe = $null
    $v145Probe = $null
    if ($msbuild) {
        Normalize-COH-ProcessEnvironment
        $v142Probe = Test-COH-MSBuildToolset -MSBuildPath $msbuild -Toolset 'v142' -RepoRoot $repoRoot
        $v142Usable = $v142Probe.success
        $v142Detail = if ($v142Usable) { 'MSBuild compiled and produced the v142 probe library' } elseif ($v142CompilerPass) { "compiler files are present, but the v142 MSBuild project probe failed: $($v142Probe.summary)" } else { "v142 project probe failed: $($v142Probe.summary)" }
        if (-not $v142Usable) {
            $v145Probe = Test-COH-MSBuildToolset -MSBuildPath $msbuild -Toolset 'v145' -RepoRoot $repoRoot
            Add-Check 'Fallback MSVC v145 toolset' $v145Probe.success $(if ($v145Probe.success) { 'MSBuild compiled and produced the v145 probe library; agent/build.ps1 can use this fallback with its compatibility props' } else { "v145 fallback probe failed: $($v145Probe.summary)" }) 'Install a complete current C++ toolset or repair the MSBuild installation.'
        }
        Add-Check 'MSVC v142 toolset' $v142Usable $v142Detail 'Install Visual Studio 2019 Build Tools with the v142 C++ workload, including Win32/x86 and x64 support. Directory presence alone is not sufficient.' -Blocking (-not $v145Probe.success)
    } else {
        Add-Check 'MSVC v142 toolset' $false 'not tested because MSBuild is unavailable' 'Install MSBuild before testing the C++ project toolset.'
    }

    $winSdkFound = $false
    $kits = 'HKLM:\SOFTWARE\Microsoft\Windows Kits\Installed Roots'
    if (Test-Path $kits) {
        $kitRoot = (Get-ItemProperty $kits -ErrorAction SilentlyContinue).KitsRoot10
        if ($kitRoot -and (Test-Path (Join-Path $kitRoot 'Lib'))) { $winSdkFound = $true }
    }
    Add-Check 'Windows 10 SDK' $winSdkFound $(if ($winSdkFound) { 'Windows 10 SDK detected' } else { 'Windows 10 SDK not detected' }) 'Install a Windows 10 SDK through Visual Studio Installer.'

    $expectedSubmodules = @('bin/data/server/maps', 'bin/piggs')
    foreach ($sub in $expectedSubmodules) {
        $path = Join-Path $repoRoot ($sub -replace '/', '\')
        $hasFiles = (Test-Path $path) -and [bool](Get-ChildItem $path -Force -ErrorAction SilentlyContinue | Select-Object -First 1)
        Add-Check "Submodule: $sub" $hasFiles $(if ($hasFiles) { 'content present' } else { 'missing or empty' }) 'Run: git submodule update --init --recursive'
    }

    # The checked-in AuthServer and DbServer configs explicitly name
    # "ODBC Driver 17 for SQL Server". Release|x86 is the locally verified
    # baseline, so the 32-bit registration matters in particular.
    $odbc17x86 = $false
    $odbc17x64 = $false
    try {
        $odbc17x86 = [bool](Get-OdbcDriver -Platform '32-bit' -ErrorAction SilentlyContinue | Where-Object Name -EQ 'ODBC Driver 17 for SQL Server')
        $odbc17x64 = [bool](Get-OdbcDriver -Platform '64-bit' -ErrorAction SilentlyContinue | Where-Object Name -EQ 'ODBC Driver 17 for SQL Server')
    } catch {}
    Add-Check 'SQL Server ODBC Driver 17 (32-bit)' $odbc17x86 $(if ($odbc17x86) { 'ODBC Driver 17 detected for 32-bit processes' } else { 'ODBC Driver 17 not detected for 32-bit processes' }) "Install Microsoft ODBC Driver 17 for SQL Server. On 64-bit Windows, use Microsoft's x64 Driver 17 installer; it installs both 64-bit and 32-bit drivers."
    Add-Check 'SQL Server ODBC Driver 17 (64-bit)' $odbc17x64 $(if ($odbc17x64) { 'ODBC Driver 17 detected for 64-bit processes' } else { 'ODBC Driver 17 not detected for 64-bit processes' }) 'Install Microsoft ODBC Driver 17 for SQL Server.'

    # A driver is not a database server. Both checked-in server configs use
    # Server=localhost with Windows integrated authentication, so verify that
    # exact endpoint. Use ODBC here to exercise the same driver family as the
    # game servers rather than merely checking for a Windows service name.
    $sqlReachable = $false
    $sqlDetail = 'not tested because ODBC Driver 17 (32-bit) is missing'
    $sqlDatabases = @()
    if ($odbc17x86) {
        $conn = $null
        try {
            # ODBC Driver 17 does not accept the SqlClient-style
            # "Connection Timeout" keyword. Keep this connection string aligned
            # with the checked-in game config to avoid misleading diagnostics.
            $connString = 'Driver={ODBC Driver 17 for SQL Server};Server=localhost;Database=master;Trusted_Connection=yes;'
            $conn = New-Object System.Data.Odbc.OdbcConnection($connString)
            $conn.Open()
            $sqlReachable = ($conn.State -eq [System.Data.ConnectionState]::Open)
            if ($sqlReachable) {
                $sqlDetail = 'connected to Server=localhost / Database=master using Windows authentication'
                $cmd = $conn.CreateCommand()
                $cmd.CommandText = "SELECT name FROM sys.databases WHERE name IN ('cohdb','cohauth') ORDER BY name"
                $reader = $cmd.ExecuteReader()
                while ($reader.Read()) { $sqlDatabases += [string]$reader.GetString(0) }
                $reader.Close()
            }
        } catch {
            $sqlDetail = $_.Exception.Message -replace '[\r\n]+',' '
        } finally {
            if ($conn) { try { $conn.Close() } catch {} }
        }
    }
    Add-Check 'SQL Server localhost' $sqlReachable $sqlDetail 'Install/configure SQL Server and ensure the checked-in Server=localhost connection works with Windows authentication. A named instance such as localhost\SQLEXPRESS will not satisfy the current config unless the config is changed.'

    if ($sqlReachable) {
        $hasCohDb = $sqlDatabases -contains 'cohdb'
        $hasCohAuth = $sqlDatabases -contains 'cohauth'
        # cohdb may legitimately be absent before the first DbServer launch;
        # servers.cfg contains SqlInit directives to create it. Report this as
        # informational without making the pre-start doctor fail.
        Add-Check 'Development database: cohdb' $true $(if ($hasCohDb) { 'cohdb exists' } else { 'not present yet; DbServer config contains SqlInit create-database directives' }) ''
        # cohauth is not required for the primary local-development smoke path,
        # which intentionally uses TestClient -db and bypasses AuthServer.
        Add-Check 'Optional auth database: cohauth' $true $(if ($hasCohAuth) { 'cohauth exists' } else { 'not present; AuthServer integration will not be considered ready, but direct-DB development can proceed' }) ''
    } else {
        Add-Check 'Development database: cohdb' $false 'cannot inspect because Server=localhost is unreachable' 'Fix the SQL Server localhost check first.'
        Add-Check 'Optional auth database: cohauth' $true 'not inspected; optional for the primary direct-DB development workflow' ''
    }

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

$failed = @($results | Where-Object { -not $_.pass -and $_.blocking })
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
        $label = if ($r.pass) { 'PASS' } elseif ($r.blocking) { 'FAIL' } else { 'WARN' }
        Write-Host ('[{0}] {1} - {2}' -f $label, $r.name, $r.detail)
        if (-not $r.pass -and $r.fix) { Write-Host ('       Fix: {0}' -f $r.fix) }
    }
    Write-Host ''
    if ($failed.Count -eq 0) { Write-Host 'READY' } else { Write-Host ("NOT READY - {0} check(s) failed" -f $failed.Count) }
}

if ($failed.Count -gt 0) { exit 1 }
exit 0
