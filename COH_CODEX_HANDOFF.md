# Ouroboros / City of Heroes Orchestrator Handoff

Generated: 2026-08-14 (America/Chicago)

Repository: `D:\github\cohsourcedev`

Branch: `agent/agent-dev-foundation`, tracking `origin/agent/agent-dev-foundation`

GitHub repository: [joshcatania/cohsourcedev](https://github.com/joshcatania/cohsourcedev)

Published branch: [agent/agent-dev-foundation](https://github.com/joshcatania/cohsourcedev/tree/agent/agent-dev-foundation)

Draft pull request: [#1 – Establish local development foundation and capture scaffold](https://github.com/joshcatania/cohsourcedev/pull/1), targeting `64-bit-fx`

## Handoff purpose

This is a Windows-native City of Heroes/OuroDev codebase. The immediate engineering goal was a reproducible local development loop, followed by deterministic developer controls for graphical capture. **Phase 0 and Phase 1 are both verified as of 2026-08-15.** The most recent checkpoint ("Phase 1 verified — 2026-08-15" at the end of this document) supersedes all earlier checkpoint wording.

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

## Latest Phase 1 debugging checkpoint — 2026-08-14 stopping point

(Superseded by "Phase 1 verified — 2026-08-15" at the end of this document; kept as historical record.)

This section was previously the most recent handoff. The exact bounded capture run below proves that the graphical client gets substantially farther: it initializes the renderer, performs the direct-DB login, resumes the existing character, connects to MapServer, loads the target scene, and then hangs before capture readiness/clean exit.

### Mission and non-goals

The active mission is still Phase 1: make

```powershell
.\agent\capture.ps1 -Target AtlasPlaza_CityHall_03 -AccountName Dummy00018 -Password 11111111 -TimeoutSeconds 180 -Json
```

produce a real JPG, report the known map/location and camera state, and exit Ouroboros cleanly with machine-readable evidence. Do not start texture generation, renderer modernization, or broad graphical-asset work. Do not replace the existing ServerMonitor/TestClient infrastructure. Do not paper over the hang with arbitrary sleeps.

### Repository and GitHub location

- Local repository: D:\github\cohsourcedev
- GitHub repository: joshcatania/cohsourcedev
- Current branch: agent/agent-dev-foundation
- Branch URL: https://github.com/joshcatania/cohsourcedev/tree/agent/agent-dev-foundation
- Draft PR: https://github.com/joshcatania/cohsourcedev/pull/1, targeting 64-bit-fx
- Last clean published commit before this debugging attempt: 56c4b5740 (Document GitHub handoff location)

The current worktree is intentionally dirty because the instrumentation below was started but not completed. Do not reset or discard it without reviewing the diff.

### Known-good Phase 0 baseline

The direct database path is the reliable baseline. bin/data/server/db/servers.cfg is in the reversible local-development mode:

```text
// AuthServer 127.0.0.1 2104
UseFakeAuth 1
UseQueueServer 1
```

TestClient uses -db 127.0.0.1, bypasses AuthServer, reaches DbServer and MapServer, and can exit cleanly. The historical verified character/map run is:

- Result JSON: agent/logs/smoke-directdb-20260814-161149.json
- Status: agent/logs/smoke-directdb-20260814-161149.status
- Account: Dummy00018
- Character: TEST-35034
- passed=true, stage=character-map, map_connected=true, exit code 0
- Duration: 58.93 seconds
- Read-only SQL confirmation: AuthName=Dummy00018 Name=TEST-35034 StaticMapId=1 MapId=NULL LoginCount=1

Several later short smoke attempts were timing-sensitive: some logged in or reached MapServer but returned before the TestClient map_connected marker. Treat those as diagnostic observations, not replacements for the historical verified baseline. Before the next graphical experiment, rerun the full -ExerciseCharacter smoke and preserve its JSON/status/log paths.

### Exact graphical hang reproduction

The exact current capture command was run from the repository root with bin as the client working directory:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\agent\capture.ps1 -Target AtlasPlaza_CityHall_03 -AccountName Dummy00018 -Password 11111111 -TimeoutSeconds 180 -Json
```

Evidence:

- Result JSON: agent/logs/capture-AtlasPlaza_CityHall_03-20260814-171750.json
- Complete stdout: agent/logs/capture-AtlasPlaza_CityHall_03-20260814-171750.stdout.log
- Complete stderr: agent/logs/capture-AtlasPlaza_CityHall_03-20260814-171750.stderr.log
- Local start: 2026-08-14T17:17:50.9014304-05:00
- Local finish: 2026-08-14T17:20:51.0509751-05:00
- Timeout: 180 seconds
- Ouroboros PID: 6792
- Parent PowerShell PID observed: 15060; capture shell parent observed: 24384
- Command line:

  ```text
  "D:\github\cohsourcedev\bin\Ouroboros.exe" -db 127.0.0.1 -authname Dummy00018 -password 11111111 -noverify -quicklogin 1 -noversioncheck -capture AtlasPlaza_CityHall_03 -fullscreen 0 -screen 1280 720 -stopinactivedisplay 0
  ```

- Working directory: D:\github\cohsourcedev\bin
- While hung, PID 6792 was present, Responding=True, working set approximately 73 MB, and had minimal CPU activity.
- Result: passed=false, timedOut=true, exitCode=124, reason Ouroboros timed out before clean capture exit, screenshot path empty.
- No JPG was produced.
- The old wrapper did not save a complete inherited environment snapshot. The next capture.ps1 implementation must capture the environment before launch rather than claiming this run preserved it.

The important stdout sequence (logger timestamps are UTC, approximately five hours ahead of the local wrapper timestamps) was:

```text
22:17:51 Project: Ouroboros; Preloaded PhysX DLLs
22:17:51 Loaded message stores
22:20:24 Initialized error log
22:20:25 Initialized hardware lights
22:20:25-22:20:25 Compiled fragment/vertex shaders; Renderer initialization complete
22:20:25-22:20:33 Loaded folders, sounds, tricks, textures, fonts, network library, game data, FX, NPCs, costume bins, animations, and other data
22:20:33.779 Capture quick login: account=Dummy00018 populated=0 max=0
22:20:33.834 Connecting to DBServer 127.0.0.1:7000 (UDP)
22:20:33.958 Capture waiting for DbServer queue admission
22:20:34.149 Capture queue result: 2
22:20:34.149 Capture login result: auth=1 db=4 slots=1 max=48 error=none
22:20:34.225 Capture character handoff: slot=0 name=TEST-35034 result=1 error=none
22:20:34.263 Connecting to MapServer 127.0.0.1:7001 (UDP)
22:20:38.110 Detailed trays; 22:20:38.158 Welded 764 interior models
22:20:38.281 Applied 4057 different, 381590 same texture swaps
22:20:38.285 Loaded 7 zowies; 22:20:41.810 Created 2311 PhysX objects
22:20:42.287 Loaded 28 textures
22:20:42.287 Waiting for mapserver update..
```

There is no later Loaded all data!, no capture-ready marker, no screenshot request/result, and no clean Ouroboros exit. This places the current known stop after MapServer scene/asset initialization and before the capture state can finish. The exact blocking call still needs a native stack or more precise markers.

The repeated stderr messages are legacy registry lookup warnings (regfileLoadKeyValue: No such file or directory). They were not the apparent fatal cause.

### Instrumentation currently present but not usable yet

Uncommitted source edits add startup markers in these files:

- Game/src/main.c: process start, registry/folder setup, command-line parse, graphics/audio load, resume info, data load, and game-loop entry.
- Game/src/game.c / Game/src/game.h: startup trace functions plus markers around argument parsing, direct credentials, quick login, game_beforeParseArgs, renderer finalization, audio/input, window setup, network start, data load, capture processing, game_beforeLoop, and first game-loop update.
- Game/src/UI/uiLogin.c: DbServer connection and queue markers.
- Game/src/clientcomm/authclient.c: direct-auth request/response markers.
- Game/src/clientcomm/dbclient.c: DbServer login, character selection, and MapServer handoff markers.
- Game/src/clientcomm/clientcomm.c: MapServer connection, scene request, groups/entities, and scene completion markers.

The marker design is intended to reveal the actual transition between checkQuickLogin(), game_beforeLoop(), MapServer packet handling, commReqScene(), and capture readiness. No instrumented trace has been collected yet.

Important implementation problems to fix before using this build:

1. Game/src/game.c currently declares the trace output as standard FILE *, but this translation unit has a project FileWrapper/FILE collision. The build emitted C4133 warnings around the trace writer. Replace it with an unambiguous Win32 handle or another type-safe writer before trusting the trace.
2. authclient.c is shared with TestClient and now calls game_startupTrace, but TestClient does not link the Ouroboros game.c implementation. The build therefore fails at TestClient link with LNK2001: unresolved external symbol _game_startupTrace and LNK1120.
3. A previous attempt to add a TestClient trace implementation did not apply. Either add a small compatible implementation to the TestClient target or move the trace implementation into a shared source/library linked by both targets. Keep TestClient's existing autonomous status behavior intact.

The current build log is agent/logs/build-Release-x86-20260814-172832.log. Do not run capture from this failed/incomplete build and do not report the instrumentation as validated.

### Native debugging discovery

No command-line CDB, WinDbg, WinDbgX, or ProcDump executable was found in PATH or the searched Visual Studio/Windows SDK locations. The Windows SDK does contain dbghelp.dll/dbgcore.dll, and the repository contains:

- Utilities/dumpstk/bin/x86/Release/dumpstk.exe
- Utilities/dumpstk/src/dumpstk.cpp
- bin/Ouroboros.exe and bin/Ouroboros.pdb

dumpstk.exe is a dump analyzer, not a live attach debugger. It accepts a dump with -f, an image path with -i, symbols with -y, and address options. It can be useful after a dump is created.

agent/dump-process.ps1 was added as a separate-process minidump helper. It uses dbghelp.dll MiniDumpWriteDump and records process metadata (command line, parent, modules, threads) beside the dump. It has only been tested against an already-exited PID, so it produced metadata with Cannot find a process with the process identifier 6792 and no .dmp. Test and repair it against a live hung Ouroboros process before relying on it. If MiniDumpWriteDump is blocked by access rights or P/Invoke flags, solve that in the helper or install/use a real Windows debugger; do not make Ouroboros dump itself.

The target process was stopped after the run with agent/stop-shard.ps1 -ForceProcessStop -Json; no ServerMonitor, DbServer, Launcher, MapServer, Ouroboros, or TestClient processes remain.

### Required next actions, in order

1. Inspect git status --short, git diff, AGENTS.md, this section, and docs/agent-status.md. Preserve the existing edits; do not reset.
2. Repair the trace implementation/type collision and provide the shared trace symbol for TestClient. Build with the only verified baseline: agent/build.ps1 -Configuration Release -Platform x86 -Json. Confirm the build completes before any new capture.
3. Run agent/doctor.ps1 -Json, start a disposable shard, and immediately run the historical-style TestClient character/map smoke. Save the JSON/status/log paths and ensure it passes before comparing Ouroboros.
4. Start one bounded capture. At timeout, invoke agent/dump-process.ps1 while Ouroboros is still alive. Capture PID, command line, parent, cwd, environment, modules, threads, dump path, and last startup marker. Analyze the dump with dumpstk.exe using Ouroboros's PDB and save all evidence under agent/logs/.
5. Use the marker and native stack to determine whether the wait is in MapServer update/packet dispatch, game_beforeLoop, rendering, input/audio, or synchronization. Compare the same direct-DB account flow against TestClient; do not assume UseFakeAuth 1 eliminates every graphical login-state dependency.
6. Update agent/capture.ps1 so timeout is self-diagnosing: machine-readable failure JSON, timestamps, PID/cmdline/cwd/environment, client stdout/stderr, server status/logs, last marker, dump/stack evidence, bounded client cleanup, and disposable shard cleanup. Preserve the normal success JSON and screenshot validation.
7. Test one hypothesis at a time. Do not add arbitrary delays. Only after the client reaches the known map and produces a real JPG should you work on deterministic camera repeatability and Atlas Park capture.
8. Update this handoff and docs/agent-status.md with every confirmed result. Phase 1 must remain explicitly unverified until two bounded captures produce the intended JPG and clean exits.

### Final state at handoff

- Phase 0: historically verified and should remain the regression baseline.
- Phase 1 graphical capture: NOT VERIFIED; no JPG exists.
- Exact hang: after direct-DB login, character handoff, MapServer connection, and scene loading; last baseline message is Waiting for mapserver update..
- Native stack: NOT YET COLLECTED.
- Capture timeout diagnostics: NOT YET IMPLEMENTED in agent/capture.ps1.
- Instrumented build: NOT BUILDING because of the TestClient unresolved trace symbol; also has a FILE/FileWrapper warning issue in the trace writer.
- Working tree: dirty with the source instrumentation and agent/dump-process.ps1; no commit or push was made for this debugging attempt.

## Phase 1 verified — 2026-08-15 final checkpoint

This section supersedes every earlier checkpoint. Phase 1 is complete: the deterministic graphical capture produces a real image with fixed camera and hidden UI, reports machine-readable results, and exits Ouroboros cleanly — verified twice consecutively. The dump/stack tooling was never needed because the startup traces localized both stalls without it.

### What the 2026-08-14 next-actions produced

1. The committed instrumentation build was validated (`agent/logs/build-Release-x86-20260815-084019.log` and `agent/logs/build-Release-x86-20260815-112932.log`, both exit 0). The TestClient `LNK2001` was resolved by `Utilities/TestClient/src/startupTrace.c`; the `FILE*`/FileWrapper collision by the Win32-handle writer in `Game/src/game.c`.
2. The startup traces localized the stalls:
   - TestClient started ~30 seconds after shard start hung at `db.login.response.queue-or-other` for the full 180-second window — login admitted by DbServer but held in the login queue with no admission packets. On a warmed shard the same smoke passes. Root cause family: shard warm-up (launcher/TSR preload; overload protection keeps `queueserver_letPlayersThrough()` frozen and `s_skipQueue()` queueing everyone).
   - The graphical client's `Waiting for mapserver update..` stall also cleared on a warm shard: the full entity update arrives, `notifyReceivedCharacter()` fires, and the capture sequence (`capture.readiness.ready` → `capture.camera.fixed` → `capture.screenshot.request` → `capture.quit.request`) completes with a clean exit.
3. Defects fixed during verification:
   - `Utilities/TestClient/src/main.c` — the character-resume path never set `g_agent_smoke_map_connected`, so smokes against an account with an existing character reported `map_connected=0` despite full map entry. This produced the recorded "timing-sensitive" smoke failures. The marker is now set when `commReqScene(1)` succeeds on resume.
   - `agent/capture.ps1` — helper functions declared a `$Pid` parameter (collides with the read-only automatic `$PID`), aborting the wrapper before any launch. Renamed to `$ProcessId`.
   - `agent/capture.ps1` — `Start-Process -PassThru` with redirected streams never populates `ExitCode` on this PowerShell build (confirmed with a controlled `cmd /c exit 7` test), misclassifying clean exits as failures; managed stream-drain event handlers crashed the PowerShell host outright (the earlier exit-code-5 orphan). The launcher now runs Ouroboros through `cmd.exe /v:on /c` with file redirection and records the client's real exit code via `!ERRORLEVEL!`.

### Definition of done evidence

- Run 1: `agent/logs/capture-AtlasPlaza_CityHall_03-20260815-114556.json` — passed=true, exitCode=0, timedOut=false.
- Run 2: `agent/logs/capture-AtlasPlaza_CityHall_03-20260815-114700.json` — passed=true, exitCode=0, timedOut=false.
- Artifact: `agent/captures/AtlasPlaza_CityHall_03.jpg` (verified visually twice: Atlas Plaza with City Hall and the Atlas statue, third-person camera behind the character, no visible UI).
- Supporting baseline: `agent/logs/smoke-directdb-20260815-113420.json` (passed=true, `map_connected=1`, exit 0).
- The disposable shard was stopped and a post-stop scan showed zero running shard processes.

### Operational rule discovered

A freshly started shard does not admit logins for the first few minutes (queue/overload warm-up). Always let the shard warm up and pass one `agent/smoke.ps1 -ExerciseCharacter` run before judging any login, queue, map-entry, or capture failure as real.

### Next actions

1. Expand deterministic capture to more scenes/labels (multiple fixed positions across maps).
2. Build the capture-comparison regression harness (baseline vs. current image, machine-readable diff verdict).
3. Only then begin renderer changes, guarded by before/after captures.
4. Update `AGENTS.md` and `docs/agent-status.md` with each verified result, as done here.
