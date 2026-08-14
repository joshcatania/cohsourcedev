# Agent Development Guide

This repository contains a large Windows-native City of Heroes/OuroDev codebase. Future coding agents should prefer the existing build, runtime, ServerMonitor, and TestClient infrastructure rather than inventing replacements.

## Verified from the repository

- The primary solution is `build/vs2019/master.sln`.
- Existing CI builds `Release|x86` with MSBuild.
- The solution also declares Debug/Release x86/x64 configurations, but those configurations must not be called working until tested on a Windows development machine.
- `1-start-servers.bat` starts `bin/ServerMonitor.exe -connect` from the `bin` directory.
- Runtime data is supplied by two git submodules:
  - `bin/data/server/maps`
  - `bin/piggs`
- `Utilities/TestClient` is part of the modern build and contains autonomous login, movement, combat, mission, stress, social, and network-test functionality.

## Locally verified development baseline

Verified on a Windows development machine on 2026-08-14:

- `agent/doctor.ps1` reports `READY`.
- Git, MSBuild, MSVC v142, Windows 10 SDK, both runtime-data submodules, SQL Server ODBC driver, runtime paths, TestClient, and ServerMonitor build inputs were all detected successfully.
- `Release|x86` full build completed successfully with `agent/build.ps1`.
- Observed full-build duration: 311.4 seconds.

This makes `Release|x86` the current locally verified development baseline.

## Agent workflow

Run these from the repository root in PowerShell.

### 1. Check the environment

```powershell
.\agent\doctor.ps1
```

Machine-readable output:

```powershell
.\agent\doctor.ps1 -Json
```

Do not start changing source code until failures here are understood.

### 2. Build

Locally verified baseline:

```powershell
.\agent\build.ps1 -Configuration Release -Platform x86
```

The wrapper writes complete logs under `agent/logs/` while keeping terminal output compact.

Debug and/or x64 builds are candidates, not yet locally validated by this guide:

```powershell
.\agent\build.ps1 -Configuration Debug -Platform x86
.\agent\build.ps1 -Configuration Release -Platform x64
```

If one of those is tested successfully, update this document with the result.

### 3. Start the shard monitor

```powershell
.\agent\start-shard.ps1
```

This intentionally wraps the repository's existing `ServerMonitor.exe -connect` startup convention.

Important: at present, successful launch only proves ServerMonitor remained alive after startup. It does **not** prove the complete shard is healthy.

### 4. Inspect process state

```powershell
.\agent\status.ps1
```

or:

```powershell
.\agent\status.ps1 -Json
```

Process presence is diagnostic only. Do not equate process existence with application readiness.

### 5. Stop

A graceful command-line ServerMonitor shutdown interface has not yet been repository-verified. Therefore `stop-shard.ps1` refuses destructive process termination by default.

```powershell
.\agent\stop-shard.ps1
```

For a disposable local development shard only:

```powershell
.\agent\stop-shard.ps1 -ForceProcessStop
```

Prefer replacing the forced path with a verified graceful ServerMonitor control path once discovered.

## Current highest-priority missing capability

Build a real `agent/smoke.ps1` around `Utilities/TestClient` / `Utilities/TestClientLauncher`.

The desired smoke test should establish, with non-zero exit codes on failure:

1. shard responds
2. TestClient connects
3. authentication succeeds
4. a character can be selected or created
5. the client enters a map
6. the client remains healthy for a short controlled period
7. no new fatal errors/assertions/crashes are observed

Prefer TestClient over GUI automation of `Ouroboros.exe`.

The smoke test should emit both a concise terminal summary and JSON.

## Rules for future agents

- Never report a build/run/test path as working unless you actually executed it successfully.
- Preserve complete logs but keep normal context output concise.
- Reuse ServerMonitor and TestClient instead of duplicating their functionality.
- Avoid broad project modernization unless a concrete compatibility failure requires it.
- Before gameplay/AI/render changes, establish a reproducible baseline and rerun the same validation afterward.
- Keep agent scripts safe to rerun.
- Treat `Release|x86` as the current locally verified build baseline.
- Do not infer that other configurations work merely because they are declared in the solution.
