[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [int]$TargetPid,
    [Parameter(Mandatory = $true)]
    [string]$OutputPath,
    [string]$MetadataPath,
    [string]$WorkingDirectory,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'

if (-not $MetadataPath) {
    $MetadataPath = "$OutputPath.json"
}

$dumpDirectory = Split-Path -Parent $OutputPath
if ($dumpDirectory) {
    New-Item -ItemType Directory -Force -Path $dumpDirectory | Out-Null
}
$metadataDirectory = Split-Path -Parent $MetadataPath
if ($metadataDirectory) {
    New-Item -ItemType Directory -Force -Path $metadataDirectory | Out-Null
}

if (-not ('CoHAgentMiniDump.NativeMethods' -as [type])) {
    Add-Type -TypeDefinition @'
using System;
using System.ComponentModel;
using System.IO;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

namespace CoHAgentMiniDump
{
    public static class NativeMethods
    {
        [Flags]
        public enum ProcessAccess : uint
        {
            QueryInformation = 0x0400,
            QueryLimitedInformation = 0x1000,
            VmRead = 0x0010,
            DuplicateHandle = 0x0040
        }

        [Flags]
        public enum MiniDumpType : uint
        {
            Normal = 0x00000000,
            WithDataSegs = 0x00000001,
            WithFullMemory = 0x00000002,
            WithHandleData = 0x00000004,
            WithUnloadedModules = 0x00000020,
            WithFullMemoryInfo = 0x00000800,
            WithThreadInfo = 0x00001000,
            WithTokenInformation = 0x00004000
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr OpenProcess(ProcessAccess access, bool inheritHandle, uint processId);

        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool CloseHandle(IntPtr handle);

        [DllImport("dbghelp.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool MiniDumpWriteDump(
            IntPtr process,
            uint processId,
            SafeHandle file,
            MiniDumpType dumpType,
            IntPtr exceptionParam,
            IntPtr userStreamParam,
            IntPtr callbackParam);
    }
}
'@
}

function Get-TargetSnapshot {
    param([int]$ProcessId)

    $process = Get-Process -Id $ProcessId -ErrorAction Stop
    $cim = Get-CimInstance Win32_Process -Filter "ProcessId=$ProcessId" -ErrorAction SilentlyContinue
    $threads = @()
    foreach ($thread in @($process.Threads)) {
        $threads += [ordered]@{
            id = $thread.Id
            state = [string]$thread.ThreadState
            waitReason = [string]$thread.WaitReason
            priority = $thread.CurrentPriority
            startAddress = $thread.StartAddress
        }
    }

    $modules = @()
    try {
        foreach ($module in @($process.Modules)) {
            $modules += [ordered]@{
                name = $module.ModuleName
                path = $module.FileName
                baseAddress = ('0x{0:X}' -f $module.BaseAddress.ToInt64())
                size = $module.ModuleMemorySize
            }
        }
    }
    catch {
        $modules = @([ordered]@{ error = $_.Exception.Message })
    }

    [ordered]@{
        pid = $ProcessId
        capturedAt = (Get-Date).ToString('o')
        executablePath = $cim.ExecutablePath
        commandLine = $cim.CommandLine
        parentProcessId = $cim.ParentProcessId
        creationDate = $cim.CreationDate
        workingDirectory = $WorkingDirectory
        processStartTime = $process.StartTime.ToString('o')
        responding = $process.Responding
        cpuSeconds = $process.CPU
        workingSetBytes = $process.WorkingSet64
        threadCount = $threads.Count
        threads = $threads
        modules = $modules
    }
}

$result = [ordered]@{
    passed = $false
    pid = $TargetPid
    outputPath = $OutputPath
    metadataPath = $MetadataPath
    dumpType = 'WithDataSegs,WithFullMemory,WithHandleData,WithUnloadedModules,WithFullMemoryInfo,WithThreadInfo,WithTokenInformation'
    startedAt = (Get-Date).ToString('o')
}

$handle = [IntPtr]::Zero
$file = $null
try {
    $result.target = Get-TargetSnapshot -ProcessId $TargetPid

    $access = [CoHAgentMiniDump.NativeMethods+ProcessAccess]::QueryInformation -bor
              [CoHAgentMiniDump.NativeMethods+ProcessAccess]::VmRead -bor
              [CoHAgentMiniDump.NativeMethods+ProcessAccess]::DuplicateHandle
    $handle = [CoHAgentMiniDump.NativeMethods]::OpenProcess($access, $false, [uint32]$TargetPid)
    if ($handle -eq [IntPtr]::Zero) {
        $access = [CoHAgentMiniDump.NativeMethods+ProcessAccess]::QueryLimitedInformation -bor
                  [CoHAgentMiniDump.NativeMethods+ProcessAccess]::VmRead
        $handle = [CoHAgentMiniDump.NativeMethods]::OpenProcess($access, $false, [uint32]$TargetPid)
    }
    if ($handle -eq [IntPtr]::Zero) {
        throw "OpenProcess failed with Win32 error $([Runtime.InteropServices.Marshal]::GetLastWin32Error())"
    }

    $file = [System.IO.File]::Open($OutputPath, [System.IO.FileMode]::CreateNew,
                                   [System.IO.FileAccess]::Write, [System.IO.FileShare]::Read)
    $dumpFlags = [CoHAgentMiniDump.NativeMethods+MiniDumpType]::WithDataSegs -bor
                 [CoHAgentMiniDump.NativeMethods+MiniDumpType]::WithFullMemory -bor
                 [CoHAgentMiniDump.NativeMethods+MiniDumpType]::WithHandleData -bor
                 [CoHAgentMiniDump.NativeMethods+MiniDumpType]::WithUnloadedModules -bor
                 [CoHAgentMiniDump.NativeMethods+MiniDumpType]::WithFullMemoryInfo -bor
                 [CoHAgentMiniDump.NativeMethods+MiniDumpType]::WithThreadInfo -bor
                 [CoHAgentMiniDump.NativeMethods+MiniDumpType]::WithTokenInformation
    $ok = [CoHAgentMiniDump.NativeMethods]::MiniDumpWriteDump(
        $handle, [uint32]$TargetPid, $file.SafeFileHandle, $dumpFlags,
        [IntPtr]::Zero, [IntPtr]::Zero, [IntPtr]::Zero)
    if (-not $ok) {
        throw "MiniDumpWriteDump failed with Win32 error $([Runtime.InteropServices.Marshal]::GetLastWin32Error())"
    }
    $result.passed = $true
    $result.bytes = (Get-Item -LiteralPath $OutputPath).Length
}
catch {
    $result.error = $_.Exception.Message
    if (Test-Path -LiteralPath $OutputPath) {
        $result.partialBytes = (Get-Item -LiteralPath $OutputPath).Length
    }
}
finally {
    if ($file) { $file.Dispose() }
    if ($handle -ne [IntPtr]::Zero) { [CoHAgentMiniDump.NativeMethods]::CloseHandle($handle) | Out-Null }
    $result.finishedAt = (Get-Date).ToString('o')
    $result | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $MetadataPath -Encoding UTF8
}

if ($Json) {
    $result | ConvertTo-Json -Depth 8
}
else {
    if ($result.passed) {
        Write-Host "MINIDUMP PASS - $OutputPath"
    }
    else {
        Write-Host "MINIDUMP FAIL - $($result.error)"
    }
}

if ($result.passed) { exit 0 }
exit 1
