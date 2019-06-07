# This script assumes that the powershell script executionpolicy is set correctly
# and MSBuild is reachable

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Invoke-Build {
    param(
        [string] $Solution,
        [string] $Configuration,
        [string] $Platform,
        [string] $Target
    )

    if ($Target -ne "") {
        msbuild "$Solution" /t:"$Target" /p:Configuration="$Configuration" /p:Platform="$Platform"
    } else {
        msbuild "$Solution" /p:Configuration="$Configuration" /p:Platform="$Platform"
    }
    
    if ($LASTEXITCODE -ne 0) { 
        exit
    }
}

function Invoke-BuildAll {
    param (
        [string] $Solution
    )

    Invoke-Build -Solution $Solution -Configuration "Release" -Platform "Win32"
    Invoke-Build -Solution $Solution -Configuration "Debug" -Platform "Win32"
}

Invoke-BuildAll "..\3rdparty\cryptopp\cryptlib.sln"
Invoke-BuildAll "..\3rdparty\zlibsrc\zlibvc.sln"
Invoke-BuildAll "..\3rdparty\zeromq2-1\builds\msvc\msvc.sln"
Invoke-BuildAll "..\3rdparty\IJGWin32\IJGWin32.sln"
Invoke-BuildAll "..\3rdparty\yajl\yajl_s.sln"

Invoke-BuildAll "..\Utilities\StructParser\StructParser.sln"

Invoke-BuildAll "..\AuthServer\src\AuthServer\AuthServer.sln"

Invoke-Build "ArtTools.sln" -Configuration "Debug" -Platform "Win32"

Invoke-BuildAll "MasterSolution.sln"
