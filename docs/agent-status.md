# Autonomous Agent Status

Last verified: 2026-08-14 (America/Chicago)

## Current milestone

Phase 0 is complete on the current Windows development machine. The documented direct-DbServer path reaches login, creates a reproducible development character, starts/uses a MapServer, and exits with machine-readable evidence. Phase 1 capture scaffolding is present, but the graphical client capture exit test is not yet verified.

## Verified ladder

| Check | Result | Evidence |
|---|---|---|
| Toolchain | PASS with documented fallback | `agent/doctor.ps1 -Json` reports a functional v145 probe; the native v142 probe preserves the exact `MSB8020` warning and is non-blocking because v145 succeeds. |
| Build | PASS | `agent/logs/build-Release-x86-20260814-160726.log`; Release/x86 selected v145 and exited 0 after the capture-path rebuild. |
| Database | PASS | `Server=localhost` ODBC connectivity succeeded; `cohdb` exists and contains the initialized schema. |
| Server startup | PASS | `agent/start-shard.ps1` observed ServerMonitor, DbServer, and Launcher. |
| Server readiness | PASS | Application-level smoke reached DbServer and MapServer; process presence alone remains diagnostic. |
| Client startup | PASS | TestClient exited 0 in both smoke stages. |
| Smoke test | PASS | Latest character/map run: `agent/logs/smoke-directdb-20260814-161149.json`; TestClient exited 0 and reached MapServer after the capture-path rebuild. |
| Graphical capture | NOT VERIFIED | `agent/logs/capture-AtlasPlaza_CityHall_03-20260814-160829.json` timed out after 180 seconds without a screenshot or clean Ouroboros exit. |

The latest character/map smoke also wrote `agent/logs/smoke-directdb-20260814-161149.status`:

```text
account=Dummy00018
map_connected=1
error=0
exit_code=0
character=TEST-35034
```

A read-only SQL check confirmed:

```text
AuthName=Dummy00018 Name=TEST-35034 StaticMapId=1 MapId=NULL LoginCount=1
```

`MapId=NULL` is expected after the short autonomous client disconnect; the TestClient agent marker proves it reached the MapServer before exit.

The capture scaffold adds the `capture <label>` command and `agent/capture.ps1` wrapper. The graphical client can be built and launched, but the direct-DB Ouroboros path did not emit its login/map markers during the bounded run and produced no JPG. A capture-only queue-drain path was added to mirror TestClient's queue handling; it did not change that result. Keep this path explicitly unverified until a real image and clean exit are observed.

## Toolchain decision

Visual Studio 18 is installed, with v142 compiler files visible under the installation, but its MSBuild metadata cannot resolve the v142 Platform Toolset and the v142 x86 runtime libraries are incomplete. The functional probe reproduces `MSB8020`. The build wrapper therefore requests v142, probes it, and falls back to v145 only when the v145 probe succeeds. The fallback is narrowly scoped with `agent/v145-compat.props`; the solution was not broadly retargeted.

The v145 compatibility fixes are limited to legacy Crypto++ compiler branches, the LogParser3 unreachable return, and the C4703 warning policy needed by the fallback compiler. The latest rebuild after the capture queue-handling change also passed.

## Direct-DB development mode

`bin/data/server/db/servers.cfg` is currently in the explicit reversible direct-DB mode:

```text
// AuthServer 127.0.0.1 2104
UseFakeAuth 1
UseQueueServer 1
```

Use `agent/set-directdb-mode.ps1 -Disable` to restore the checked-in AuthServer directive and `UseFakeAuth 0` when working on AuthServer integration. The optional `cohauth` and `cohacc` databases are not required for the primary loop. The TestClient `-db` path now bypasses the legacy AccountServer availability guard for local development, while preserving the normal guard for other modes.

## Known limitations

- Native v142 remains unavailable on this machine; do not describe it as a working build configuration.
- AuthServer/AccountServer integration remains unverified because `cohauth`/`cohacc` are absent. ServerMonitor may still launch those processes; their process presence is not integration readiness.
- `agent/start-shard.ps1` establishes process readiness. `agent/smoke.ps1` is the application-level readiness check.
- The repository still emits legacy map-data warnings during startup. They did not prevent the verified character/map smoke.
- The graphical `agent/capture.ps1` path remains unverified: the latest bounded run timed out with no screenshot. The direct TestClient path remains the reliable Phase 0 baseline.

## Phase 1 blocker / next action

The remaining blocker is before deterministic map entry: the rebuilt graphical client hangs before producing the capture login marker, while the same shard and database pass the TestClient character/map smoke. Future work should capture a native client hang/stack or run the graphical direct-DB login under a debugger, then resolve that handoff before claiming the screenshot command is repeatable.

## Useful commands

```powershell
.\agent\doctor.ps1 -Json
.\agent\build.ps1 -Configuration Release -Platform x86 -Json
.\agent\set-directdb-mode.ps1 -Enable -Json
.\agent\start-shard.ps1 -StartupWaitSeconds 60 -Json
.\agent\smoke.ps1 -Json
.\agent\smoke.ps1 -ExerciseCharacter -AccountName Dummy00009 -TimeoutSeconds 180 -Json
.\agent\capture.ps1 -Target AtlasPlaza_CityHall_03 -AccountName Dummy00018 -Password 11111111 -TimeoutSeconds 180 -Json
.\agent\status.ps1 -Json
.\agent\stop-shard.ps1 -ForceProcessStop -Json
```
