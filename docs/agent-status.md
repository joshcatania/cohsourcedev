# Autonomous Agent Status

Last verified: 2026-08-15 (America/Chicago)

## Current milestone

Phase 0 and Phase 1 are both complete and verified on the current Windows development machine. The direct-DbServer path reaches login, resumes or creates a reproducible development character, enters a MapServer, and exits with machine-readable evidence. The deterministic graphical capture now produces a real image of Atlas Plaza with fixed camera and hidden UI, reports machine-readable results, and exits Ouroboros cleanly — verified twice consecutively.

## Verified ladder

| Check | Result | Evidence |
|---|---|---|
| Toolchain | PASS with documented fallback | `agent/doctor.ps1 -Json` reports a functional v145 probe; the native v142 probe preserves the exact `MSB8020` warning and is non-blocking because v145 succeeds. |
| Build | PASS | `agent/logs/build-Release-x86-20260815-112932.log`; Release/x86 selected v145 and exited 0 after the TestClient startup-trace link fix was validated. |
| Database | PASS | `Server=localhost` ODBC connectivity succeeded; `cohdb` exists and contains the initialized schema. |
| Server startup | PASS | `agent/start-shard.ps1` observed ServerMonitor, DbServer, and Launcher. |
| Server readiness | PASS | Application-level smoke reached DbServer and MapServer; process presence alone remains diagnostic only. |
| Client startup | PASS | TestClient exited 0 in both smoke stages. |
| Smoke test | PASS | `agent/logs/smoke-directdb-20260815-113420.json` — passed=true with `map_connected=1` on the resume path after the marker fix. |
| Graphical capture | PASS | `agent/logs/capture-AtlasPlaza_CityHall_03-20260815-114556.json` and `agent/logs/capture-AtlasPlaza_CityHall_03-20260815-114700.json` — both passed=true, exitCode=0, real JPG at `agent/captures/AtlasPlaza_CityHall_03.jpg` (verified visually: Atlas Plaza/City Hall, third-person camera, no UI). |

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

The capture scaffold adds the `capture <label>` command and `agent/capture.ps1` wrapper. This path is now verified (see the Phase 1 verified section below); it requires a warmed-up shard, exactly like the TestClient smoke.

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
- The shard needs several minutes of warm-up after startup before logins are admitted reliably. A login/queue/map-entry stall on a freshly started shard is expected to clear once warm; see the 2026-08-15 checkpoint below.

## Phase 1 blocker — RESOLVED 2026-08-15

The graphical-client stall was real but environmental, not a code path defect in the capture scaffold: on a freshly started shard, logins stall in the login queue and the first mapserver update never arrives until the shard has warmed up (TSR mapservers preloading/overload protection window). On a warm shard, the same exact capture command completes. The wrapper also had two real bugs (see below) that produced false results during verification.

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

## Phase 1 debugging checkpoint — 2026-08-14 stopping point

(Superseded by "Phase 1 verified — 2026-08-15" below; kept as historical record.)

The direct-DB TestClient path remains the Phase 0 baseline. The latest exact graphical capture was bounded at 180 seconds and did not produce an image, but it progressed farther than previously recorded:

- Command: agent/capture.ps1 -Target AtlasPlaza_CityHall_03 -AccountName Dummy00018 -Password 11111111 -TimeoutSeconds 180 -Json
- Result: agent/logs/capture-AtlasPlaza_CityHall_03-20260814-171750.json
- Stdout/stderr: agent/logs/capture-AtlasPlaza_CityHall_03-20260814-171750.stdout.log and .stderr.log
- Ouroboros PID: 6792
- Working directory: D:\github\cohsourcedev\bin
- Local runtime: 17:17:50.901 to 17:20:51.051
- Result: passed=false, timedOut=true, exitCode=124, no screenshot

The captured Ouroboros log proves renderer/shader initialization, direct DbServer connection, queue admission, successful login (auth=1 db=4 slots=1), character handoff (TEST-35034), MapServer connection, scene/model/PhysX/texture loading, then Waiting for mapserver update... No capture-ready marker, JPG, or clean exit followed. This is the current known hang point; Phase 1 remains NOT VERIFIED.

The historical direct-DB smoke that must remain the comparison baseline is agent/logs/smoke-directdb-20260814-161149.json (passed=true, character-map, map_connected=true, Dummy00018 / TEST-35034, clean exit). Several later short attempts were timing-sensitive and must not replace that evidence.

The worktree now contains uncommitted startup instrumentation in Game/src/main.c, Game/src/game.c, Game/src/game.h, Game/src/UI/uiLogin.c, Game/src/clientcomm/authclient.c, Game/src/clientcomm/clientcomm.c, and Game/src/clientcomm/dbclient.c, plus agent/dump-process.ps1. No instrumented trace has been captured. The rebuild failed at TestClient link because shared authclient.c references _game_startupTrace; the trace writer in Game/src/game.c also needs its FILE/FileWrapper type collision corrected. Build log: agent/logs/build-Release-x86-20260814-172832.log.

No CDB/WinDbg/ProcDump executable was found. The Windows SDK dbghelp.dll is present, and the separate-process dump helper was added but has only been exercised after the target PID had already exited. It must be tested against a live hang and analyzed with Utilities/dumpstk/bin/x86/Release/dumpstk.exe plus bin/Ouroboros.pdb.

The next agent should repair/build the instrumentation, rerun the verified TestClient smoke immediately before capture, obtain a live dump/stack at timeout, and then make agent/capture.ps1 preserve PID, command line, environment, cwd, client/server logs, last marker, dump/stack evidence, cleanup state, and machine-readable failure JSON. Do not claim Phase 1 success or begin camera/asset work until a real AtlasPlaza_CityHall_03.jpg and clean Ouroboros exit are verified twice. Full context and exact resume steps are in [COH_CODEX_HANDOFF.md](../COH_CODEX_HANDOFF.md).

## Phase 1 verified — 2026-08-15

The 2026-08-14 checkpoint's next actions were executed and Phase 1 passed its definition of done:

1. The committed instrumentation build was validated: `agent/logs/build-Release-x86-20260815-084019.log` and `agent/logs/build-Release-x86-20260815-112932.log` both exited 0. The TestClient `LNK2001` was resolved by `Utilities/TestClient/src/startupTrace.c` and the `FILE*`/FileWrapper collision by the Win32-handle trace writer in `Game/src/game.c`.
2. The startup traces immediately localized both prior stalls:
   - A TestClient smoke run started ~30 seconds after shard start hung at `db.login.response.queue-or-other` for the full window (login queued, no admission). Root cause family: shard warm-up — launcher/TSR preload and overload protection keep queue admissions closed on a fresh shard.
   - On a warmed shard, the same smoke passed: `agent/logs/smoke-directdb-20260815-113420.json` (passed=true, `map_connected=1`, exit 0).
   - The graphical client's historical `Waiting for mapserver update..` stall also cleared on a warm shard: the full-update packet arrives, `capture.readiness.ready` → `capture.camera.fixed` → `capture.screenshot.request` → `capture.quit.request` completes, and Ouroboros exits.
3. Three defects were found and fixed on the way to a truthful PASS:
   - `Utilities/TestClient/src/main.c`: the resume path never set `g_agent_smoke_map_connected`, so smokes against an account with an existing character reported `map_connected=0` despite full map entry (the source of the "timing-sensitive" smoke failures recorded on 2026-08-14). The marker is now set when `commReqScene(1)` succeeds on resume.
   - `agent/capture.ps1`: helper functions declared a `$Pid` parameter, which collides with PowerShell's read-only automatic `$PID` and aborted the script before any run. Renamed to `$ProcessId`.
   - `agent/capture.ps1`: `Start-Process -PassThru` with redirected streams never populates `ExitCode` on this PowerShell build (verified with a controlled `cmd /c exit 7` test), which misclassified a clean exit as a failure; managed `DataReceivedEventHandler` scriptblocks crashed the host outright. The launcher now runs the client through `cmd.exe /v:on /c` with file redirection and records the client's real exit code via `!ERRORLEVEL!` to a file. Two consecutive official runs then passed with exitCode=0.
4. Verification evidence:
   - Run 1: `agent/logs/capture-AtlasPlaza_CityHall_03-20260815-114556.json` (passed=true, exitCode=0)
   - Run 2: `agent/logs/capture-AtlasPlaza_CityHall_03-20260815-114700.json` (passed=true, exitCode=0)
   - Artifact: `agent/captures/AtlasPlaza_CityHall_03.jpg` — visually verified twice (Atlas Plaza with City Hall and the Atlas statue, third-person camera behind the character, no visible UI).
5. The disposable shard was stopped with `agent/stop-shard.ps1 -ForceProcessStop -Json`; a post-stop status scan showed zero running shard processes.

Next priorities are recorded in `AGENTS.md`: multi-scene deterministic capture, a capture-comparison regression harness, and only then renderer changes guarded by before/after captures.
