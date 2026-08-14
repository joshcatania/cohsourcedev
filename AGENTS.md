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
- TestClient supports `-db <address>`. In that mode its login code bypasses AuthServer and synthesizes a direct local DbServer target.
- The checked-in DbServer configuration targets local SQL Server and contains initialization directives capable of creating the `cohdb` database.
- AuthServer is a separate/optional integration layer for the primary local-development loop; its checked-in config expects a `cohauth` database and listens for game clients on TCP 2106 when fully initialized.

## Locally verified development baseline

Verified on a Windows development machine on 2026-08-14:

- Git, MSBuild, the Windows 10 SDK, both runtime-data submodules, ODBC Driver 17 (32-bit and 64-bit), SQL Server `Server=localhost`, TestClient, and ServerMonitor build inputs are functional.
- The installed v142 compiler files are present, but the v142 MSBuild project probe reproduces `MSB8020`; its x86 runtime library set is incomplete. The doctor reports this as a non-blocking warning when the fallback succeeds.
- `agent/build.ps1 -Configuration Release -Platform x86` completes successfully by selecting the tested v145 fallback and importing `agent/v145-compat.props`. The latest verified rebuild took 39.0 seconds; earlier full v145 builds also passed.
- `agent/start-shard.ps1` successfully launched ServerMonitor and observed `ServerMonitor`, `DbServer`, and `Launcher` after 6.83 seconds. Process readiness is followed by the application-level smoke test.
- The direct-DB smoke path created character `TEST-40283` for `Dummy00009`, reached a MapServer, and exited cleanly. The row was confirmed in `cohdb` with `StaticMapId=1` and `LoginCount=1`.
- `cohdb` is available. `cohauth` and the optional `cohacc`/other auxiliary databases are not present; they are outside the critical direct-DB development loop.

`Release|x86` with the v145 fallback is the current locally verified build baseline. See [`docs/agent-status.md`](docs/agent-status.md) for exact evidence and log paths.

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

The doctor now distinguishes the ODBC driver from an actual SQL Server endpoint and checks the exact `Server=localhost` configuration used by the checked-in server files.

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

This intentionally wraps the repository's existing `ServerMonitor.exe -connect` startup convention and polls for `DbServer` and `Launcher` instead of relying on a fixed sleep. For a slower machine, use `-StartupWaitSeconds 60`.

Successful launch of ServerMonitor alone does **not** prove the complete shard is healthy.

### 4. Inspect process state

```powershell
.\agent\status.ps1
```

or:

```powershell
.\agent\status.ps1 -Json
```

Process presence is diagnostic only. Do not equate process existence with application readiness.

### 5. Run the primary smoke test

```powershell
.\agent\smoke.ps1
```

The primary local-development smoke path intentionally uses TestClient's `-db 127.0.0.1` mode and bypasses AuthServer. This matches the simpler OuroDev local-development model and removes the legacy `cohauth` database from the critical edit/build/test loop. Ensure the reversible development switch is enabled with `.\agent\set-directdb-mode.ps1 -Enable`.

The default smoke proves direct DbServer login. The staged character/map check proves reproducible character creation, Launcher/MapServer startup, client MapServer entry, and a clean exit:

```powershell
.\agent\smoke.ps1 -ExerciseCharacter -AccountName Dummy00009 -TimeoutSeconds 180
```

The smoke test writes complete stdout/stderr, an autonomous status file, and JSON under `agent/logs/`, and returns a non-zero exit code on failure. Headless runs include `-nosharedmemory` so the autonomous path does not depend on the GUI client's shared heap.

### 6. Optional AuthServer diagnostics

AuthServer is not required for the primary direct-DB development loop. When AuthServer integration itself is being worked on, use:

```powershell
.\agent\diagnose-auth.ps1
```

This reports its process command line, owned TCP listeners, expected config ports, and recent runtime logs.

### 7. Stop

A graceful command-line ServerMonitor shutdown interface has not yet been repository-verified. Therefore `stop-shard.ps1` refuses destructive process termination by default.

```powershell
.\agent\stop-shard.ps1
```

For a disposable local development shard only:

```powershell
.\agent\stop-shard.ps1 -ForceProcessStop
```

The forced path now includes the known ServerMonitor children (`LogServer`, `BeaconServer`, and `BeaconClient`) and performs a bounded post-shutdown rescan so late-spawned children and exit races are reported accurately. Prefer replacing it with a verified graceful ServerMonitor control path once discovered.

## Current highest-priority missing capability

Phase 0 is complete. An unverified Phase 1 deterministic developer-control scaffold is present; do not report it as working until it produces a real image and clean client exit. The next priority is:

1. deterministic map selection and teleport
2. fixed camera/FOV and hidden UI
3. a repeatable screenshot command and clean client exit
4. machine-readable capture success/failure and regression evidence

Prefer TestClient over GUI automation of `Ouroboros.exe`.

## Rules for future agents

- Never report a build/run/test path as working unless you actually executed it successfully.
- Preserve complete logs but keep normal context output concise.
- Reuse ServerMonitor and TestClient instead of duplicating their functionality.
- Prefer the direct `-db` development path for ordinary edit/build/test loops; treat AuthServer as a separate integration target unless a task specifically needs it.
- Avoid broad project modernization unless a concrete compatibility failure requires it.
- Before gameplay/AI/render changes, establish a reproducible baseline and rerun the same validation afterward.
- Keep agent scripts safe to rerun.
- Treat `Release|x86` as the current locally verified build baseline.
- Do not infer that other configurations work merely because they are declared in the solution.
