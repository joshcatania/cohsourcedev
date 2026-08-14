# Ouroboros / City of Heroes Orchestrator Handoff

Generated: 2026-08-14 (America/Chicago)

Repository: `D:\github\cohsourcedev`

Branch: `agent/agent-dev-foundation`, tracking `origin/agent/agent-dev-foundation`

## Handoff purpose

This is a Windows-native City of Heroes/OuroDev codebase. The immediate engineering goal is a reproducible local development loop, followed by deterministic developer controls for graphical capture. Phase 0 is verified. Phase 1 has an implemented but unverified scaffold and is currently blocked at the graphical client login/map handoff.

The authoritative repository instructions are in `AGENTS.md`. The latest detailed status is in `docs/agent-status.md`. Do not treat this handoff as permission to discard the existing dirty worktree or to claim an unverified path works.

## Project shape

- `Game/`: Ouroboros graphical client and most client gameplay, UI, rendering, networking, login, character creation, and map-entry code.
- `DBServer/`: direct database login, character/account persistence, character selection, map allocation, and server-side database protocol.
- `MapServer/`: runtime map instances and player/entity simulation.
- `Launcher/`: launches and coordinates MapServer instances requested by the shard.
- `ServerMonitor/`: repository-standard process orchestration and `-connect` startup path.
- `AuthServer/` and `AccountServer/`: legacy/optional authentication and account-service integration. They are not required for the verified direct-DB development loop and are not integration-ready on this machine.
- `QueueServer/`: login queue layer. It remains enabled in the current direct-DB configuration; TestClient has queue-drain behavior, while the graphical capture path needed an additional capture-only drain.
- Other shard services: `ChatServer`, `AuctionServer`, `MissionServer`, `ArenaServer`, `RaidServer`, `TurnstileServer`, `StatServer`, `BeaconServer`, `LogServer`, and related shared services.
- `Utilities/TestClient/`: autonomous, non-GUI client with direct login, character creation, map entry, movement, combat, mission, stress, social, and network-test support. Prefer it for headless validation.
- `Common/`, `ServerAPI/`, `libs/`, `3rdparty/`, and `Assets/`: shared code, protocols, libraries, third-party dependencies, and runtime/build assets.
- `bin/`: built executables and runtime data. Important submodules are `bin/data/server/maps` and `bin/piggs`.
- `build/vs2019/master.sln`: primary solution.
- `agent/`: safe wrappers for environment checks, build, shard startup/status/stop, direct-DB mode, smoke tests, and the experimental graphical capture.

## Verified machine and toolchain

Verified on a Windows development machine on 2026-08-14:

- Visual Studio 18 Community is installed at `C:\Program Files\Microsoft Visual Studio\18\Community`.
- MSBuild is `MSBuild\Current\Bin\MSBuild.exe`.
- v142 compiler files exist, including compiler version `14.29.30133`, but the installed MSBuild metadata cannot resolve Platform Toolset `v142` and the x86 v142 runtime libraries are incomplete.
- The exact v142 project probe reproduces `MSB8020`.
- Current MSBuild can compile with the v145 toolset. `agent/build.ps1` requests v142, probes it, and falls back to v145 only when the v145 probe succeeds.
- `agent/v145-compat.props` narrows the fallback warning-policy adjustment to v145 builds.
- Windows 10 SDK, ODBC Driver 17 for both 32-bit and 64-bit processes, SQL Server `Server=localhost`, runtime-data submodules, TestClient, and ServerMonitor inputs are present.
- Do not retarget the whole solution to v145 or report native v142 as working. The fallback is an agent/build selection plus narrowly scoped compatibility changes.

## Database and runtime configuration

The checked-in DbServer configuration is `bin/data/server/db/servers.cfg`.

Current verified direct-development state:

```text
// AuthServer 127.0.0.1 2104
UseFakeAuth 1
UseQueueServer 1
SqlDbName cohdb
SqlLogin "Driver={ODBC Driver 17 for SQL Server};Server=localhost;Persist Security Info=False;Trusted_Connection=yes;"
```

`cohdb` exists and contains the initialized schema. `cohauth` and `cohacc` are absent, so AuthServer/AccountServer integration is not a verified target. Use `agent/set-directdb-mode.ps1 -Disable` only when deliberately switching back to the optional AuthServer configuration; do not make AuthServer readiness a prerequisite for ordinary local edits.

The direct graphical/TestClient development path uses `-db 127.0.0.1`, which synthesizes a local DbServer target and bypasses AuthServer. ServerMonitor may still launch optional AuthServer/AccountServer processes; their presence is diagnostic only.

## Verified Phase 0 baseline

The following ladder has been executed successfully:

1. `agent/doctor.ps1 -Json` returns `ready: true`. It reports the v142 MSB8020 probe failure as non-blocking because the v145 fallback probe passes; SQL Server, ODBC, SDK, submodules, and required build inputs pass.
2. `agent/build.ps1 -Configuration Release -Platform x86 -Json` succeeds, selects v145, and exits 0. Latest evidence: `agent/logs/build-Release-x86-20260814-160726.log`.
3. `agent/start-shard.ps1 -StartupWaitSeconds 60 -Json` launches ServerMonitor and observes ServerMonitor, DbServer, and Launcher. Latest observed process-ready time was about 6.7 seconds.
4. `agent/smoke.ps1 -ExerciseCharacter -AccountName Dummy00018 -TimeoutSeconds 180 -Json` succeeds. Latest evidence:
   - result: `agent/logs/smoke-directdb-20260814-161149.json`
   - status: `agent/logs/smoke-directdb-20260814-161149.status`
   - account: `Dummy00018`
   - character: `TEST-35034`
   - `map_connected=1`
   - `error=0`
   - TestClient exit code `0`
5. A read-only SQL verification confirmed `AuthName=Dummy00018`, `Name=TEST-35034`, `StaticMapId=1`, `MapId=NULL`, `LoginCount=1`. `MapId=NULL` is expected after the short autonomous client disconnect; the status marker proves MapServer entry before exit.
6. `agent/stop-shard.ps1 -ForceProcessStop -Json` was used for the disposable local shard and reported no remaining processes. A final status check showed ServerMonitor, DbServer, Launcher, MapServer, Ouroboros, and the known child services stopped.

An earlier verified row is also documented in `AGENTS.md`: `Dummy00009` / `TEST-40283`.

## Changes already present in the dirty worktree

These changes are intentional and belong to the current development foundation. Preserve them unless a specific regression is demonstrated.

### Build and agent infrastructure

- `agent/lib/toolchain.ps1`: MSBuild discovery, v142 compiler inspection, disposable toolset probes, and environment normalization.
- `agent/doctor.ps1`: functional v142/v145 probes, SQL endpoint validation, runtime/submodule checks, and blocking/non-blocking result classification.
- `agent/build.ps1`: Auto toolset selection, v145 compatibility import, serial build settings to avoid legacy copy races, complete log and JSON output.
- `agent/start-shard.ps1`, `agent/status.ps1`, and `agent/stop-shard.ps1`: process-aware startup/readiness/status/cleanup wrappers. Process readiness is not application readiness.
- `agent/set-directdb-mode.ps1`: reversible exact-config switch for fake direct DB auth.
- `agent/smoke.ps1`: direct login smoke plus staged character creation/MapServer smoke with machine-readable JSON/status evidence.

### Compatibility and runtime fixes

- Crypto++ legacy MSVC preprocessor branches were narrowed for modern v145 compilation in `3rdparty/cryptopp-8.3/src/integer.cpp`, `validat3.cpp`, and `zdeflate.cpp`.
- `Utilities/LogParser3/src/LogSearch.cpp` has a v145 compile fix for an unreachable return.
- `Game/src/render/thread/rt_shaderMgr.c` and `Game/src/render/renderUtil.c` defensively handle unknown/null shader capability names encountered by the local graphical client.
- `Game/src/gameComm/initClient.c` allows the TestClient direct-DB mode to proceed without the optional AccountServer and emits an autonomous MapServer-connected marker.
- `Utilities/TestClient/src/main.c` supports direct DB credentials/status handling and clean machine-readable smoke results.

### Phase 1 capture scaffold

- `Game/src/game.c`, `Game/src/game.h`, `Game/src/cmdparse/cmdgame.c`, and `Game/src/cmdparse/cmdgame.h` add command-line credentials/direct-DB handling and a `capture <label>` command.
- The intended capture sequence waits for a connected player/map, hides UI, selects third-person camera/FOV/distance, applies a fixed Atlas Plaza position, invokes `screenshottitle <label>`, waits briefly, and issues `quit`.
- `Game/src/UI/uiLogin.c` adds a capture-only queue-drain path based on TestClient behavior.
- `agent/capture.ps1` launches `bin/Ouroboros.exe`, supplies direct-DB credentials and capture arguments, bounds execution, detects new JPGs, copies the artifact to `agent/captures`, and writes JSON.

## Phase 1 status: not verified

The Phase 1 exit test is not complete. The capture target was `AtlasPlaza_CityHall_03`.

Latest bounded attempt:

- command wrapper: `agent/capture.ps1`
- result: `agent/logs/capture-AtlasPlaza_CityHall_03-20260814-160829.json`
- timeout: 180 seconds
- screenshot: none
- clean Ouroboros exit: no
- stdout log: zero bytes
- stderr: repeated legacy registry-file lookup warnings only

Earlier queue-enabled diagnostic attempts showed the graphical client authenticating and receiving queue-position traffic but not reaching a usable player-list/map handoff. A capture-only queue drain was added and rebuilt; the final existing-character run still hung before capture login markers. Queue-disabled experiments were reverted. The current config is back to `UseQueueServer 1`.

Do not tell downstream systems that `capture AtlasPlaza_CityHall_03` works. No image exists in `agent/captures` and no deterministic graphical regression artifact has been verified.

## Recommended next actions

Follow this order and preserve the Phase 0 baseline:

1. Read `AGENTS.md`, this handoff, and `docs/agent-status.md`; inspect `git status --short --branch` before editing. The worktree is intentionally dirty; do not reset or discard it.
2. Run `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\agent\doctor.ps1 -Json` and `agent/status.ps1 -Json`. Confirm no shard/client processes are left over.
3. Rebuild with `agent/build.ps1 -Configuration Release -Platform x86 -Json`. Keep the v145 fallback; do not broadly retarget the solution.
4. Start the disposable shard with `agent/start-shard.ps1 -StartupWaitSeconds 60 -Json`, then rerun the verified TestClient smoke before each new graphical experiment.
5. Diagnose the graphical hang with a native debugger or a reliable Windows hang dump/stack. The important comparison is the same direct-DB shard and account flow: TestClient reaches MapServer, while Ouroboros hangs before its capture login marker. Determine whether the stall is in graphical startup, command-line credential/resume initialization, login queue handling, packet dispatch, or renderer initialization.
6. Keep every graphical attempt bounded and record stdout/stderr, process exit code, exact arguments, and server status. Do not turn an unbounded `NO_TIMEOUT` path into a claimed success; the wrapper timeout is the safety boundary.
7. Only after the graphical client reaches MapServer should you validate the fixed teleport/camera/UI commands, inspect the actual JPG, repeat the same capture command, and update `docs/agent-status.md` to Phase 1 verified.
8. Stop the disposable shard with `agent/stop-shard.ps1 -ForceProcessStop -Json` and confirm no remaining processes.

## Useful commands

Run from `D:\github\cohsourcedev` in PowerShell:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\agent\doctor.ps1 -Json
.\agent\build.ps1 -Configuration Release -Platform x86 -Json
.\agent\set-directdb-mode.ps1 -Enable -Json
.\agent\start-shard.ps1 -StartupWaitSeconds 60 -Json
.\agent\smoke.ps1 -Json
.\agent\smoke.ps1 -ExerciseCharacter -AccountName Dummy00018 -TimeoutSeconds 180 -Json
.\agent\capture.ps1 -Target AtlasPlaza_CityHall_03 -AccountName Dummy00018 -Password 11111111 -TimeoutSeconds 180 -Json
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\agent\status.ps1 -Json
.\agent\stop-shard.ps1 -ForceProcessStop -Json
```

## Definition of done for the current mission

Phase 0 remains done when doctor, Release/x86 build, direct-DB login, reproducible character creation, MapServer entry, machine-readable status, and clean TestClient exit all pass.

Phase 1 is done only when a bounded command such as:

```powershell
.\agent\capture.ps1 -Target AtlasPlaza_CityHall_03 -AccountName Dummy00018 -Password 11111111 -TimeoutSeconds 180 -Json
```

reliably produces a real JPG at `agent/captures/AtlasPlaza_CityHall_03.jpg`, reports a successful JSON result, and exits Ouroboros cleanly. The image must visibly reflect the intended map/teleport, fixed camera/FOV, and hidden UI. Repeat the command at least once to establish repeatability before changing the status to verified.
