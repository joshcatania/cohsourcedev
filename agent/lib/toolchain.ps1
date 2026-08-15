function Normalize-COH-ProcessEnvironment {
    # Some host launchers expose both PATH and Path in the Windows process
    # environment. .NET Framework's ProcessStartInfo treats those as duplicate
    # keys and MSBuild then fails when it starts cl.exe. Collapse them to one
    # process-local entry without changing the machine environment.
    try {
        $environment = [Environment]::GetEnvironmentVariables('Process')
        $pathValue = [string]$environment['Path']
        if ([string]::IsNullOrEmpty($pathValue)) {
            $pathValue = [string]$environment['PATH']
        }
        if (-not [string]::IsNullOrEmpty($pathValue)) {
            [Environment]::SetEnvironmentVariable('PATH', $null, 'Process')
            [Environment]::SetEnvironmentVariable('Path', $pathValue, 'Process')
        }
    } catch {
        # Environment cleanup is a compatibility aid. Leave the original
        # environment intact if this host does not permit process updates.
    }
}

function Find-COH-MSBuild {
    $cmd = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }

    $programFilesX86 = ${env:ProgramFiles(x86)}
    if ([string]::IsNullOrEmpty($programFilesX86)) { return $null }
    $vswhere = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        $path = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' 2>$null | Select-Object -First 1
        if ($path) { return [string]$path }
    }
    return $null
}

function Get-COH-VSInstallations {
    $programFilesX86 = ${env:ProgramFiles(x86)}
    if ([string]::IsNullOrEmpty($programFilesX86)) { return @() }
    $vswhere = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere)) { return @() }
    try {
        $json = & $vswhere -products * -format json 2>$null | ConvertFrom-Json
        if ($null -eq $json) { return @() }
        return @($json)
    } catch {
        return @()
    }
}

function Get-COH-V142CompilerInfo {
    $matches = New-Object System.Collections.Generic.List[object]
    foreach ($install in (Get-COH-VSInstallations)) {
        $root = [string]$install.installationPath
        if ([string]::IsNullOrEmpty($root)) { continue }
        $msvcRoot = Join-Path $root 'VC\Tools\MSVC'
        if (-not (Test-Path -LiteralPath $msvcRoot)) { continue }
        foreach ($toolset in (Get-ChildItem -LiteralPath $msvcRoot -Directory -ErrorAction SilentlyContinue | Where-Object { $_.Name -like '14.2*' })) {
            $cl = Join-Path $toolset.FullName 'bin\HostX86\x86\cl.exe'
            $include = Join-Path $toolset.FullName 'include\vcruntime.h'
            $x86Runtime = Join-Path $toolset.FullName 'lib\x86\libcmt.lib'
            $matches.Add([pscustomobject]@{
                installationPath = $root
                toolsetVersion = $toolset.Name
                compilerPath = $cl
                compilerPresent = (Test-Path -LiteralPath $cl)
                headersPresent = (Test-Path -LiteralPath $include)
                x86RuntimePresent = (Test-Path -LiteralPath $x86Runtime)
                compileUsable = ((Test-Path -LiteralPath $cl) -and (Test-Path -LiteralPath $include))
            })
        }
    }
    return $matches.ToArray()
}

function Test-COH-MSBuildToolset {
    param(
        [Parameter(Mandatory = $true)]
        [string]$MSBuildPath,
        [Parameter(Mandatory = $true)]
        [ValidateSet('v142', 'v145')]
        [string]$Toolset,
        [string]$RepoRoot = (Get-Location).Path
    )

    if (-not (Test-Path -LiteralPath $MSBuildPath)) {
        return [pscustomobject]@{
            success = $false
            toolset = $Toolset
            exitCode = 2
            artifact = $null
            summary = 'MSBuild executable was not found.'
        }
    }

    $probeRoot = Join-Path (Join-Path $RepoRoot 'agent') ('.coh-msbuild-probe-' + [Guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $probeRoot -Force | Out-Null
    $sourcePath = Join-Path $probeRoot 'probe.c'
    $projectPath = Join-Path $probeRoot 'probe.vcxproj'
    $artifact = Join-Path $probeRoot 'bin\coh_toolset_probe.lib'

    $project = @'
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Release|Win32">
      <Configuration>Release</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <ProjectGuid>{A0D5A1F6-0B9E-4E75-8D15-6AA6CB2C3F22}</ProjectGuid>
    <Keyword>Win32Proj</Keyword>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration">
    <ConfigurationType>StaticLibrary</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>__TOOLSET__</PlatformToolset>
    <CharacterSet>NotSet</CharacterSet>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <TargetName>coh_toolset_probe</TargetName>
    <OutDir>__PROBE_ROOT__\bin\</OutDir>
    <IntDir>__PROBE_ROOT__\obj\</IntDir>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>false</SDLCheck>
      <PreprocessorDefinitions>WIN32;NDEBUG;_LIB;%(PreprocessorDefinitions)</PreprocessorDefinitions>
    </ClCompile>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ClCompile Include="__SOURCE_PATH__" />
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
</Project>
'@
    $project = $project.Replace('__TOOLSET__', $Toolset)
    $project = $project.Replace('__PROBE_ROOT__', $probeRoot)
    $project = $project.Replace('__SOURCE_PATH__', $sourcePath)
    Set-Content -LiteralPath $sourcePath -Value 'int coh_toolset_probe(void) { return 0; }' -Encoding UTF8
    Set-Content -LiteralPath $projectPath -Value $project -Encoding UTF8

    Normalize-COH-ProcessEnvironment
    $output = @(& $MSBuildPath $projectPath '/t:Rebuild' '/p:Configuration=Release' '/p:Platform=Win32' "/p:PlatformToolset=$Toolset" '/nologo' '/verbosity:minimal' 2>&1 | ForEach-Object { $_.ToString() })
    $exitCode = $LASTEXITCODE
    $success = ($exitCode -eq 0 -and (Test-Path -LiteralPath $artifact))
    $summaryLines = @($output | Where-Object { $_ -match 'error |Build FAILED|Build succeeded|MSB[0-9]+' } | Select-Object -First 4)
    if ($summaryLines.Count -eq 0) {
        $summaryLines = @($output | Select-Object -Last 2)
    }

    try { Remove-Item -LiteralPath $probeRoot -Recurse -Force -ErrorAction SilentlyContinue } catch {}
    return [pscustomobject]@{
        success = $success
        toolset = $Toolset
        exitCode = $exitCode
        artifact = $artifact
        summary = (($summaryLines -join ' ') -replace '[\r\n]+', ' ').Trim()
    }
}
