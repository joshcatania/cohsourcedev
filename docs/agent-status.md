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

## Phase 2 verified — 2026-08-15 (multi-scene capture + regression harness)

Phase 2 extends Phase 1 from a single shot to a deterministic multi-scene capture suite with a machine-readable image-comparison harness.

Changes:

- `Game/src/game.c` — the capture setup now selects from a shot table (`s_captureShots`): five Atlas Park labels (CityHall_03 default, East_01, North_01, West_01, Closeup_01 with camdist 10) mapping to fixed `setpospyr`/`camdist` values; unknown labels keep the historically verified default. The setup also freezes the world clock (`timeset 16; timescale 0`).
- `agent/compare-captures.ps1` — image comparator: downsamples both JPGs to 320px width, pixel-diffs via LockBits, reports `changedPercent`/`meanDelta`/`maxDelta`, fails above `MaxChangedPercent` (2.0) or `MaxMeanDelta` (2.0).
- `agent/capture-regression.ps1` — orchestrator: runs `capture.ps1` per label, adopts missing baselines into `agent/baselines` (unless `-NoAdopt`), compares against baselines, writes a summary JSON under `agent/logs/`, exit 0 only when no shot regressed or failed.

Determinism findings:

- The world clock runs at `DAY_TIMESCALE 48` (svr_tick.c) — one in-game hour every 75 real seconds — so images taken minutes apart differ by hours of lighting unless frozen. `timeset`/`timescale` are server commands requiring access level 9; characters created under the default config have `AccessLevel 0`, so the freeze was silently rejected until the capture characters were granted `AccessLevel=9` in `cohdb.dbo.Ents`. Note the MapServer can serve a stale in-memory entity for the first login after the SQL change; re-adopt baselines once after granting access.
- Verified results: cross-scene comparison fails as expected (35.6% changed); same-scene reruns pass at 0.03–1.7% changed pixels across all five shots.

Evidence:

- Regression summary (adopt + verify pair): `agent/logs/regression-20260815-121*.json` and `agent/logs/regression-20260815-12*.json` under `agent/logs/`
- Baselines: `agent/baselines/AtlasPlaza_{CityHall_03,East_01,North_01,West_01,Closeup_01}.jpg`
- Visual check: `AtlasPlaza_East_01` confirmed as a genuinely different valid view (Atlas statue dominant) vs the City Hall default.

Next: renderer changes guarded by `agent/capture-regression.ps1` before/after runs.

## Renderer shader-path findings + harness hardening — 2026-08-15

The first renderer investigation ran under the regression harness with the following verified results:

Shader-path map (all evidence from live runs):

- Default: `game_state.useCg = 1` — Cg compiles the shipped `.cgfx`/`.cg` sources to ARB assembly (`CG_PROFILE_ARBFP1/ARBVP1`). This is the only fully working shader path; all Phase 1/2 captures used it.
- `useCg 0`: loads precompiled ARB programs (`shaders/arb/*.fp`) without the Cg runtime.
- `useCg 2` (Cg->GLSL): **broken**. Live run (`-useCg 2`, capture JSON `agent/logs/capture-AtlasPlaza_CityHall_03-20260815-124346*`): every CgFX shader fails with `CG ERROR: "The program could not load"` at `-profile glslf`, followed by fallback error shaders; the resulting image differs from baseline at 100% of pixels (meanDelta 149). Reaching GLSL therefore requires porting shader sources, not just flipping the mode.
- Legacy branches for `R200` (ATI_fragment_shader), `NV1X/NV2X` (register combiners), and `TEX_ENV_COMBINE` are selected from `rdr_caps.chip` in `rt_state.c`/`wcw_statemgmt.c` but cannot activate on modern drivers because those GL extensions no longer exist. They are deletion candidates, not runtime risks.

Tooling:

- `agent/capture.ps1` gained `-CgMode <n>`, passed to the client as `-useCg <n>` (the generic command-line parser turns it into the `useCg` console command before `finalizeRenderer()` applies it). This enables no-rebuild shader-mode A/B experiments.

Harness hardening (all empirically driven):

- Capture settle time raised from 90 to 300 frames (`game_processCapture`): sky/sun/fog interpolate for seconds after the teleport + time freeze.
- `capture-regression.ps1` now runs one discarded warmup capture per suite run: the first client on a fresh mapserver generation is the one that freezes the clock, and baselining that first image caught a mid-transition scene (94.9% false regression, reproduced and eliminated).
- `AtlasPlaza_Closeup_01` removed from the default suite: at camdist 10 the player idle-animation phase differs between runs (17.4% measured drift). Pose determinism is future engine work; the shot remains available via `-Targets`.
- Comparator thresholds set to 6% changed / 3.0 mean: measured same-scene variance peaks near 4% on the sun-facing West shot (glare shimmer) while cross-scene comparisons measure 35%+.

Final verification: full regression suite green (exit 0) with per-shot drift 0-1.5%; one shot measured exactly 0.0% changed pixels. Evidence: `agent/logs/regression-20260815-13*.json`.

## Native GLSL pilot for BLENDMODE_MODULATE — 2026-08-15

The shader-port feasibility spike completed successfully: one material now renders
through a hand-written native GLSL program, harness-verified visually equivalent
to the Cg->ARB path.

### Root cause of the `-useCg 2` failure (completed diagnosis)

Reproduced offline with the repo's own compiler (`3rdparty/cg/bin/cgc.exe`, version
2.2.0017, matching `bin/cg.dll`):

- The shipped shaders' `TIE(ENVn)` constant-bind semantics (from
  `shaders/cgfx/variants.cgh`, applied throughout `constants_fp.cgh` /
  `constants_vp.cgh`) crash the Cg 2.2 `glslf`/`glslv` backends with
  `fatal error C9999: *** exception during compilation ***`. GL-state semantics
  like `state.fog.params` compile fine; any custom `ENVn` crashes.
- With ties disabled (`#if !defined(GLSL)` guard around `#define USE_CONSTANT_TIES`),
  all 16 main scene shaders compile cleanly under both GLSL profiles with the
  production define set. The 12 remaining failures are `effects/` post-processing
  shaders (bloom/tonemap/DOF) that carry their own `TIE(ENV0)` struct-member
  semantics via `constants_vfx.cgh` — unused at default capture settings.
- Even compiling, Cg->GLSL mode cannot run: the engine always builds with
  `RT_SUPPORT_CG_COMBO_SHADERS 0`, so `WCW_SetCgShaderParamArray4fv` pushes
  constants to `program.env[]` registers, which GLSL shaders cannot read. Full
  GLSL-mode revival would need a native GLSL parameter path — which is exactly
  what the pilot now demonstrates for one material.

Shader sources live in `bin/piggs/misc.pigg`; extract with
`Utilities/pig/bin/x86/Release/pig.exe x bin\piggs\misc.pigg` (already-built tool).

### Pilot implementation

- `Game/src/render/thread/rt_glslpilot.{c,h}` (new): one GLSL 1.20
  compatibility-profile program replicating `modulatefp.cg` + the DUALTEX-family
  `vp_master_vp.cg` variants (`VERT_COLOR`/`FF_LIT_GL`/`FF_UNLIT_GL` ×
  `TC_MATRIX` + faux reflection). It reads the same GL server state the Cg
  `state.*` semantics read (`gl_ModelViewMatrix`, `gl_TextureMatrix`,
  `gl_LightSource[0]`, `gl_Fog`, `gl_FogFragCoord`), so no new engine parameter
  plumbing exists beyond `g_ReflectionParamVP`, which `WCW_SetCgShaderParamArray4fv`
  mirrors into a pilot uniform.
- Activation rides the existing WCW state machine (`WCW_BindFragmentProgram`,
  `WCW_BindVertexProgram`, enable/disable/reset paths in `wcw_statemgmt.c`):
  while the pilot's GLSL program is bound with `glUseProgram` it overrides the
  tracked ARB programs, and unbinding restores them untouched. Because the
  engine's binds are id-cached, the (modulate fragment, simple vertex variant)
  pairing is re-checked on both bind paths and on enable after disable/enable
  cycles — without this, activation is bind-order dependent and nondeterministic.
- Program ids are registered from `shaderMgr_InitVPs`/`shaderMgr_InitFPs` after
  every shader reload (ids regenerate). GLSL entry points are the raw GLEW
  pointers (`__glewUseProgram` etc.) because `ogl.h` `#undef`s the macro names
  as a "do not use" policy for the fixed pipelines.
- Enabled at startup with `-glslPilot 1` (new `game_state.glslPilot`, registered
  as the `glslPilot` console command; command-line passthrough works because
  argument parsing precedes renderer init). `agent/capture.ps1` gained
  `-ExtraClientArgs` to pass it.

### Verification (final clean binary, shard warm, clock settled)

- Control (pilot off) vs baseline, AtlasPlaza_CityHall_03: 1.27% changed / mean 1.43 — PASS.
- Pilot (verified active via `GLSL pilot: BLENDMODE_MODULATE program compiled and linked`
  in `agent/logs/capture-AtlasPlaza_CityHall_03-20260815-145*.stdout.log`):
  - CityHall_03: 1.27% / 1.53 — PASS
  - East_01: 1.51% / 2.64 — PASS
- Earlier broken variants measured 6.7% (timing-dependent activation) and 99.99%
  (unfrozen world clock), both diagnosed and fixed; the passing numbers above are
  from the committed code.
- Operational note: `capture.ps1`'s default account `Dummy00010` lacked
  `AccessLevel 9` in `cohdb.dbo.Ents`, so `timeset 16; timescale 0` was silently
  rejected and the world clock drifted (orange-sky captures, ~100% false diffs).
  Fixed with `UPDATE cohdb.dbo.Ents SET AccessLevel=9 WHERE AuthName='Dummy00010'`.
  Any future capture account needs the same grant.

### What this unlocks

The migration template is now proven end to end for simple materials: replicate
the Cg math in compatibility GLSL, read GL state through built-ins, mirror the
one program-local constant, hook the WCW bind paths, and verify with
`compare-captures.ps1` against the ARB baseline. Next candidates by coverage:
`multiplyRegfp` (BLENDMODE_MULTIPLY, pairs with vertex 227/225-class variants),
then `colorBlendDualfp`/`addGlowfp`. Bumpmapped materials additionally need
tangent-space interpolants and more engine constants, and the `effects/`
post-processing set needs the same `TIE` treatment or native ports before
`-useCg 2` could ever be revisited.

## GLSL pilot extended to BLENDMODE_MULTIPLY — 2026-08-15

The second material now renders through native GLSL, reusing the pilot's
migration template. `multiplyRegfp` (variant 0: no HQ/cubemap/shadow bits) is
`tex_base * tex_blend` (alpha included), `rgb *= 8 * vertex color` (x8 to
match the register-combiner program and old assets), `a *= g_Env0FP.a`, then
fog — replicated exactly in `rt_glslpilot.c`.

### Changes

- `rt_glslpilot.{c,h}` refactored from a single program to a material table
  (`tPilotMaterial`): each material owns its ARB fragment target id, GLSL
  fragment source, program object, and uniform locations. Both materials
  share one compiled `vp_master_vp.cg` vertex shader object (the DUALTEX
  variants are identical for both materials), attached to both programs.
- New `g_Env0FP` mirror: `setFragmentProgramConstColor(0, ...)` in
  `wcw_statemgmt.c` (the single funnel for the engine's `constColor0`, i.e.
  fragment `program.env[8]` via `TIE(ENV8)`) now calls
  `rt_glslpilot_onEnv0Param`, which mirrors continuously regardless of
  active state — same lesson as `g_ReflectionParamVP`: engine constant
  pushes are not ordered relative to pilot activation.
- `rt_shaderMgr.c` registers `g_shaderMgrFragmentProgramVariants[BLENDMODE_MULTIPLY][0]`
  as the second pilot fragment target after every shader reload.
- Per-material one-time activation logging (`GLSL pilot: <material> active
  (vertex lit mode N)`) so captures carry positive activation evidence.
- MSVC C lesson: static initializers may reference string-literal arrays but
  not pointer *variables* — the GLSL sources are `static const char[]` so
  the material table can point at them (error C2099 otherwise).
- `agent/capture-regression.ps1` gained `-ExtraClientArgs`, forwarded to
  every capture.ps1 invocation (warmup included) for whole-suite shader A/B
  runs. Empty by default; splatted only when non-empty because an empty
  `-ExtraClientArgs` is dropped by the child invocation and reported as a
  missing parameter.

### Verification (fresh shard, warmed, clock settled)

- Warm-up: `smoke.ps1 -ExerciseCharacter` timed out once at 180 s ~3 min
  after `start-shard.ps1` (documented fresh-shard warm-up), then passed
  after 4 more minutes.
- Control suite (pilot off): East/North/West PASS; CityHall_03 flagged
  48.8% once — fresh-generation sky transition (stormy mid-transition sky
  vs the clear baseline), not reproduced: an immediate single re-capture of
  CityHall_03 measured 0.14%/2.15 vs baseline. A fresh shard can need more
  than the suite's single discarded warmup before the sky settles.
- Pilot suite (`-ExtraClientArgs "-glslPilot 1"`), summary
  `agent/logs/regression-20260815-152502.json`: **all four shots PASS** —
  CityHall_03 0.00%/0.26, East_01 0.00%/0.19, North_01 1.48%/1.48,
  West_01 2.15%/2.73 (thresholds 6%/3.0).
- Both materials confirmed rendering through GLSL in every client log
  (`GLSL pilot: BLENDMODE_MODULATE active (vertex lit mode 5)` and
  `GLSL pilot: BLENDMODE_MULTIPLY active (vertex lit mode 4)` — the
  multiply activation exercises the replicated fixed-function lighting
  branch too).
- Build: `agent/logs/build-Release-x86-20260815-151109.log` (PASS, 24.7 s).
- Shard stopped afterwards; post-stop rescan found no shard processes.

Next candidate by coverage: `colorBlendDualfp` (consumes `g_ConstColor0FP`/
`g_ConstColor1FP` program-local constants and `g_Env0FP`/`g_Env1FP` — the
mirror technique extends directly), then `addGlowfp` (`g_GlowParamFP`).
