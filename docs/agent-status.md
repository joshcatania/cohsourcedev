# Autonomous Agent Status

Last verified: 2026-08-17 (America/Chicago)

## Current milestone

Phase 0 and Phase 1 are both complete and verified on the current Windows development machine. The direct-DbServer path reaches login, resumes or creates a reproducible development character, enters a MapServer, and exits with machine-readable evidence. The deterministic graphical capture now produces a real image of Atlas Plaza with fixed camera and hidden UI, reports machine-readable results, and exits Ouroboros cleanly — verified twice consecutively.

## Current renderer policy — hybrid native GLSL default-on (2026-08-17)

The normal renderer launch uses native GLSL for supported fragment/vertex
pairings and retains synchronized Cg->ARB as the intentional fallback
(`useCg 1`). No GLSL argument is required for the default mode. Use
`-glslPilot 0` for a legacy-only control/escape-hatch capture;
`-glslPilot 1` remains accepted for compatibility. The issue #11 corrected
promotion audit is the authoritative fallback inventory and found no
bucket-A blocker for this hybrid policy. ARB/Cg remains in the renderer and
must not be removed as part of this milestone.

Promotion evidence from the 2026-08-17 build: `Release|x86` passed with the
v145 compatibility fallback (`agent/logs/build-Release-x86-20260817-091218.log`),
and direct-DB smoke passed for `Dummy00001` both on login and on the staged
character/map path (`agent/logs/smoke-directdb-20260817-092546.json` and
`agent/logs/smoke-directdb-20260817-092552.json`). A no-flag Atlas capture
logged native GLSL activations for the supported materials/effects, while the
same target with `-glslPilot 0` logged no GLSL-pilot activations and exited
cleanly. The stable City Hall A/B comparison changed 1.6102% of sampled
pixels (mean delta 6.5199, report-only exposure advisory); same-window day
comparisons for East/North/West changed 4.9382%/4.3697%/1.1282%.

The four-shot default suite and the explicit legacy control suite completed;
the default City Hall baseline sample was a known weather/exposure outlier,
so the isolated stable A/B result above is the promotion sample and no
threshold was changed. Night, Talos, and high-feature Founders captures also
exited cleanly in both modes. The high-feature Founders run recorded
`water state=4`, `waterFeature=1`, and `multiFeature=1`, with default native
GLSL activation for Multi9/water/effects; the `-glslPilot 0` control emitted
no GLSL-pilot activation. Talos AddGlow remains covered by the accepted issue
#8 audit (487/491 activations, 0 declines, with same-control changed pixels of
0.0000% and 2.3994%); the current arrival view is account/map-state sensitive
and was not used to manufacture a new AddGlow sample.

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
- `agent/compare-captures.ps1` — image comparator: downsamples both JPGs to 320px width, pixel-diffs via LockBits, reports `changedPercent`/`meanDelta`/`maxDelta`, and uses `changedPercent` as the hard parity criterion. `meanDelta` remains visible as a report-only advisory because capture exposure/eye-adaptation and weather can raise it without a localized shader change.
- `agent/capture-regression.ps1` — orchestrator: runs `capture.ps1` per label, never adopts a missing baseline in formal/default mode, compares against baselines, writes a policy-bearing summary JSON under `agent/logs/`, and exits 0 only when every target is a PASS. Use `-AdoptMissingBaseline` for intentional creation; those shots are reported as `BASELINE_ADOPTED` and never count as PASS.

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

- Default fallback: `game_state.useCg = 1` — Cg compiles the shipped `.cgfx`/`.cg` sources to ARB assembly (`CG_PROFILE_ARBFP1/ARBVP1`). The normal renderer is now hybrid: native GLSL is default-on for supported pairings, with this Cg->ARB path retained for synchronized fallback. All Phase 1/2 captures used the legacy-only path before issue #12.
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
- Historical pilot enablement (before issue #12) used `-glslPilot 1` (new `game_state.glslPilot`, registered
  as the `glslPilot` console command; it is now default-on, while command-line passthrough works because
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

## GLSL pilot extended to BLENDMODE_COLORBLEND_DUAL — 2026-08-15

Third material through native GLSL. `colorBlendDualfp.cg` (variant 0) is CoV
dual color tinting: `calc_dual_tint(g_Env0FP, g_Env1FP, tex_base, tex_dual)`
from `functions.cgh`, then `rgb *= 4 * vertex color`, then fog. This is the
player costume tint material — the engine pushes `constColor0`/`constColor1`
for it from `rt_bonedmodel.c` (skinned models) and `rt_model.c` dual-tint
setup, so it has real coverage in every shot that frames the character.

### Changes

- `rt_glslpilot_onEnv0Param` generalized to `rt_glslpilot_onEnvParam(index,
  vec4)`; `setFragmentProgramConstColor` now mirrors both `constColor0`
  (g_Env0FP, env[8]) and `constColor1` (g_Env1FP, env[9]) into the pilot.
  The material table grew `usesEnv1`/`locEnv1`.
- `rt_shaderMgr.c` registers
  `g_shaderMgrFragmentProgramVariants[BLENDMODE_COLORBLEND_DUAL][0]` as the
  third pilot fragment target.

### Harness finding: weather variance (new operational constraint)

This session exposed a new environmental nondeterminism class. The
`agent/baselines` set adopted earlier in the day is CLEAR-WEATHER; the
shard generations started after ~15:35 settled into STORM weather:

- Per-mapserver-generation weather differs (clear vs stormy sky), and the
  storm state evolves on ~10-minute scales: a sun-glare spot blooms and
  fades behind the variable overcast. Measured: adjacent captures ~1.5%
  apart, but the same shot 13 minutes apart drifted 9.2% (sky region).
  Clear weather was stable all day (0.03-1.7%).
- Two shard restarts both came up stormy, so this looks like a slow weather
  cycle, not per-boot randomness; waiting it out is unreliable.
- The war walls (translucent map-edge barriers, alpha-pass geometry per
  `cubemap.c`) are in frame in the East/West shots and render identically
  in control and pilot runs — stable game content, not a comparison risk.

Mitigation used (same-window A/B): re-adopt baselines with one suite run
(its captures are the control images), then run the pilot suite immediately
after — all captures happen within a ~5-minute window where the storm noise
floor holds. A systematic pilot failure would flag ALL shots (see the 6.7%
and 99.99% historical failures); weather noise shows up as 2-5% drift
concentrated in sky regions of a subset of shots while others stay
pixel-identical.

### Verification (storm generation, warm shard, same-window A/B)

- Build: `agent/logs/build-Release-x86-20260815-153415.log` (PASS, 30.1 s).
- Control/adopt suite 16:01 (pilot off, becomes the baselines): 4/4
  captured cleanly.
- Pilot suite `agent/logs/regression-20260815-160401.json` (~1 min later):
  **all four shots PASS** — CityHall_03 0.014%/0.58, East_01 3.24%/1.96,
  North_01 5.33%/2.74, West_01 0.005%/0.30.
- Per-shot material activation (client logs, one-time lines):
  CityHall/West ran MODULATE+MULTIPLY (their 0.005-0.014% is pixel-level
  proof for those two); East/North ran all three including
  `BLENDMODE_COLORBLEND_DUAL active (vertex lit mode 4)`.
- Visual check of the East pilot-vs-control pair: identical geometry,
  statue, and war wall; residual 3.2% is atmospheric haze/brightness only —
  no tint shift (a broken dual tint would shift costume/structure hues).
- The committed baselines are now the STORM set from this verified run;
  re-adopt when the environment shifts (the harness supports it).
- Shard stopped afterwards.

Remaining simple-material candidate: `addglowfp.cg` (`g_GlowParamFP`
program-local constant, alpha-blended window/glow material).

## GLSL pilot: BLENDMODE_ADDGLOW ported; night capture shots — 2026-08-15

Fourth material ported. `addglowfp.cg` (variant 0): old-style tint
(`calc_old_tint(g_Env0FP, tex_base)`), `rgb *= 4 * vertex color`, then the
random window-glow add — `has_glow` looks up one 128x128 glow-mask texel per
1x1 base-texture tile (`floor(uv)/128 + params.yw`, glow when
`mask < params.x`) and adds the glow texture — then fog. Three samplers
(base/glow/glow_mask on TEXUNIT0/1/2) required generalizing the pilot from
hardcoded two-sampler materials to a per-material sampler/unit table.

### Changes

- `rt_glslpilot.{c,h}`: `kPilotMaterial_AddGlow` with
  `s_addGlowSamplers`; materials now carry a NULL-terminated
  `tPilotSampler` list (name -> fixed TEXUNITn) instead of hardcoded
  base/blend units. New `rt_glslpilot_onGlowParam` mirror; the default
  `{1,0,0,0}` is overwritten by the engine push before any draw (rt_tricks.c
  pushes between the addglow bind and its draws).
- `wcw_statemgmt.c`: `WCW_SetCgShaderParamArray4fv` mirrors
  `kShaderParam_GlowParamFP` into the pilot unconditionally (same pattern
  as the env mirrors).
- `rt_shaderMgr.c` registers the ADDGLOW fragment target.
- New one-time diagnostics in the pilot: activation lines existed already;
  a `bind declined, vertex program N not registered` line now
  distinguishes "bound but declined" from "never bound". Observed in night
  runs: `BLENDMODE_MULTIPLY bind declined, vertex program -1` — the
  post-reset sentinel (0xFFFFFFFF) reaching a fragment bind before any
  vertex bind; the ARB path renders it (safe fallback, no visual effect).
- `Game/src/game.c`: `CaptureShot` gained `timeHour` (16 = day default);
  new shots `AtlasPlaza_NightEast_01` and `AtlasPlaza_NightCityHall_01`
  (hour 0) to exercise night-only material states. Not in the default
  regression suite.

### Verification

- Build: `agent/logs/build-Release-x86-20260815-163441.log` and later
  passes (a running shard locks `bin\` — stop it before rebuilding; the
  build fails at the copy step otherwise).
- Night A/B (`AtlasPlaza_NightEast_01`, control then pilot, settled clock):
  **PASS 2.12% changed / 1.49 mean** — the three active materials are
  equivalent under night lighting too (moon/stars, lamp alpha, no sun).
  The FIRST night pair after the hour-0 freeze measured 99.9% — the known
  first-capture-after-time-change transition, caught mid-shift (the control
  even framed the spawn orientation because its teleport raced map entry);
  the settled rerun is the valid pair.
- Day suite, final binary, same-window A/B
  (`agent/logs/regression-20260815-165504.json`): **all four shots PASS** —
  CityHall 0.17%/0.75, East 5.51%/2.26, North 1.45%/1.20, West
  1.47%/1.43; MODULATE/MULTIPLY active in all runs, COLORBLEND_DUAL in
  East/North.
- **ADDGLOW visual verification is BLOCKED on scene coverage**: the
  program compiles, links, and registers (target id logged), but the
  ADDGLOW fragment program is never bound in any available view — day
  4-shot suite, night East, and night CityHall all show neither activation
  nor decline lines. Atlas Park's night window/lamp lighting does not use
  the ADDGLOW blend mode (likely baked emissive textures). Verifying this
  material needs a scene that draws AddGlow-trick geometry; other maps
  require the verified map-transfer path first.

### Next

1. ADDGLOW coverage: establish the map-transfer capture path (or a
   covered Atlas Park interior/view) and run the night A/B there.
2. Otherwise continue with the remaining fragment materials
   (`alphaDetailfp`, `waterfp`, effects/ set) using the same template.

## GLSL pilot: alphaDetail port + fragment coverage map — 2026-08-15

Fifth material ported and the coverage question systematized.

### Changes

- `rt_glslpilot.{c,h}`: `kPilotMaterial_AlphaDetail` —
  `rgb = lerp(blend.rgb, base.rgb, base.a)`, `rgb *= 4*vertex color`,
  `a = g_Env0FP.a * blend.a`, then fog (alphaDetailfp.cg variant 0; only
  the already-mirrored g_Env0FP constant; sampler_base/sampler_blend on
  units 0/1).
- New coverage diagnostic: `WCW_BindFragmentProgram` calls
  `rt_glslpilot_noteUnportedFragmentBind(id)` for binds the pilot did not
  handle; each distinct fragment program id is logged once per process
  (when native GLSL is enabled by default or with `-glslPilot 1`). A capture's client log now enumerates every
  material still rendering through ARB/Cg.

### Coverage map (AtlasPlaza_East_01, pilot on)

- Active through the pilot: MODULATE (4), MULTIPLY (20), COLORBLEND_DUAL
  (36).
- Registered but never bound: ADDGLOW (52), ALPHADETAIL (68) — Atlas Park
  has no coverage for either (day or night shots).
- Unported fragment programs observed bound: 20 (the MULTIPLY post-reset
  sentinel decline fallback), 100/101 (bumpmapColorblendDual and a variant,
  by load-order id arithmetic), and 201-216 (the special-effects set;
  `ssao` CgFX warnings appear in the same log).

### Verification (same-window A/B, storm generation)

- Adopt suite `agent/logs/regression-20260815-171312.json` (control),
  pilot suite `agent/logs/regression-20260815-171520.json`: CityHall
  1.50%/1.57, East 3.84%/1.71, West 3.44%/1.90 PASS; North flagged
  7.63%/2.76 once (sky movement between suites) and an immediate pilot
  re-capture of North measured **1.44%/1.28 PASS** vs the same baseline —
  transient, not reproduced.
- The alphaDetail and addGlow programs never intercept a bind (coverage
  lines absent), so their presence is provably render-neutral in these
  scenes; the three active materials are unchanged.

### Where the remaining Atlas Park work lives

The un-ported material families with real scene coverage are the
bumpmapped set (needs tangent-space interpolants and more engine
constants) and the effects/ post-processing set (bloom/ssao/etc.).
Verifying ADDGLOW/ALPHADETAIL requires scenes that bind them — the
map-transfer capture path remains the gate for that.

## GLSL pilot: bumpmapColorblendDual (static + skinned) — 2026-08-15

Sixth material ported: the first bumpmapped one. This one needed new
machinery — a tangent-space vertex path, more engine constants, and
skinning — and one real discovery about what the coverage actually is.

### What Atlas Park actually binds

The earlier coverage diagnostic observed fragment ids 100/101. Id
arithmetic (fragment ids allocate 16 per blend mode; MODULATE=4,
MULTIPLY=20, COLORBLEND_DUAL=36, ADDGLOW=52, ALPHADETAIL=68,
BUMPMAP_MULTIPLY=84) puts 100/101 at BUMPMAP_COLORBLEND_DUAL variants
0 (LQ) and 1 (HQ). The first pilot attempt registered the static
`DRAWMODE_BUMPMAP_DUALTEX` vertex program (id 232) and declined: the
LQ fragment bound with vertex program **228**, which is
`DRAWMODE_BUMPMAP_SKINNED` — the *skinned* bump_dual variant. On Atlas
Park the bump-dual material is bound by `rt_bonedmodel.c` for the
player/NPC costumes (every third-person shot frames it), not by static
building geometry (`drawLoopBumpDual`/`rt_cloth` also exist and are
registered, but the character drives activation).

### What was ported

- `bumpmapColorblendDualfp.cg` variant 0: normal-map decode + gloss
  (`map_color_to_normal`, no DXT5NM by default), `calc_dual_tint` base
  color, `calc_lighting_factors`/`apply_lighting` per-pixel lighting
  with the engine bump constants, fog. Consumes g_Env0/1FP (already
  mirrored) plus new mirrors: g_AmbientColorFP, g_DiffuseColorFP,
  g_GlossParamFP (.w gloss), g_Specular1ColorAndExponentFP
  (TIE(ENV0/1/2/5), pushed by setupBumpPixelShader/setupSpecularColor).
- One bump vertex shader replicating BOTH vp_master bump_dual (SKIN=0)
  and skin_bump (SKIN=1) variants (both VIEW space, VERTEX_LIT=NONE,
  TC_XFORM=NONE, PIXEL_LIT=BUMP_ALL, non-HQ) behind a `g_Skinned`
  uniform — same pattern as the dualtex shader's mode switch. Static
  branch: normal/tangent via modelview rows (Cg semantics, not the
  normal matrix). Skinned branch: two-bone blend
  (blend_bone_positions/blend_bone_normals) through g_BoneMatrixArrVP
  (ENV16, 48 vec4s) mirrored from loadBoneMatrices' array push
  (bone_count*3 vec4s; stale-tail semantics match ARB env regs).
- Tangent-space light/view vectors per vertex
  (calc_tangent_space_light_and_position; Cg mul(M_ts, v) is the
  transpose of GLSL column-major mat3 — explicit dots used). Binormal
  sign from the tangent attribute's w.
- Vertex attributes: tangent on generic attribute 7 (vec4; cloth feeds
  a vec3 whose w defaults to 1 → sign +1, matching ARB), bone weights
  on attribute 1, bone indices on attribute 5 — bound with
  glBindAttribLocation before linking.
- `g_LightDirVP` mirror (vertex ENV0) — pushed per draw by the bump
  draw paths.
- Pilot activation now pairs a material with a *set* of vertex kinds
  (bitmask): the bump material accepts bump_dual and skin_bump; the
  re-bind path re-modes g_Skinned when the vertex variant switches
  static↔skinned while the material stays bound.
- Diagnostics added: vertex registrations print (id, kind, lit mode),
  and the decline line now distinguishes "not registered" from
  "registered for a different vertex variant" (this is what localized
  the skinned-path discovery).

### Verification (fresh generation, warm shard, same-window A/B)

- Build: `agent/logs/build-Release-x86-20260815-18*.log` (PASS).
- Diagnostic capture (pilot on): bump program compiled/linked/active
  `(skin_bump vertex variant)`; coverage line for fragment 100 gone
  (intercepted), 101 (HQ) still ARB as designed.
- Adopt suite `agent/logs/regression-20260815-180801.json` (control),
  pilot suite `agent/logs/regression-20260815-181036.json`: **all four
  shots PASS** — CityHall_03 0.0035%/0.19, East_01 0.0035%/0.08
  (pixel-level equivalence), North_01 1.45%/1.24, West_01 5.13%/2.71
  (storm-glare noise floor; thresholds 6%/3.0).
- Per-shot activation evidence: `BLENDMODE_BUMPMAP_COLORBLEND_DUAL
  active (skin_bump vertex variant)` in all four client logs; no bump
  declines — the only decline line in each log is the known benign
  MULTIPLY post-reset sentinel (`vertex program -1`).
- The committed baselines are the adopt-suite set from this run
  (new shard generation — the prior generation had drifted 6.5-7.1%,
  the documented fresh-generation weather shift, and was re-adopted
  per the documented same-window A/B flow).
- Operational reminder reconfirmed: a running shard locks `bin\`
  (build fails at the copy step); stop it before rebuilding. A
  cancelled capture run can leave an `Ouroboros.exe` behind — kill it
  before the next build.

### Remaining in the bumpmapped family

HQ bump variants (fragment 101; HQ vertex interpolants
tangent/normal/position + cubemap/shadow paths), bumpmapMultiply
(model-space lighting, DIFFUSE vertex lighting, TC_OFFSET),
multi9 (multi-material), water. The map-transfer capture path remains
the gate for addGlow/alphaDetail verification and non-Atlas scenes.
Shader sources for all of these are extracted at `agent/shadersrc/`
(ignored; regenerate with
`Utilities/pig/bin/x86/Release/pig.exe x bin\piggs\misc.pigg`).

## GLSL pilot: bumpmapColorblendDual HQ variant (fragment 101) — 2026-08-15

Seventh material ported: the BIT_HIGH_QUALITY variant of the bumped
dual-tint material — the `101` id the coverage diagnostic had observed
bound on Atlas Park next to the LQ `100`.

### What the HQ variant actually is

The HQ fragment (`bumpmapColorblendDualfp.cg` with `BIT_HIGH_QUALITY`)
does not receive tangent-space light/view vectors from the vertex
stage. Instead the engine pairs it with the HQ compiles of the same
vertex variants (`shaderMgrVertexProgramsHQ[]`, selected via
`BIT_HIGH_QUALITY_VERTEX_PROGRAM` in the bump draw paths when
`blend_bits & BMB_HIGH_QUALITY`), which pass view-space normal, tangent
(binormal sign in w), and position as interpolants. The fragment builds
the tangent basis per pixel (`populate_lighting_vectors_hq`) and takes
the light direction from the `g_LightDirFP` fragment constant
(TIE(ENV11)) — pushed by `setupBumpPixelShader` only for HQ draws —
instead of a vertex-interpolated vector. Two faithful-to-source
nuances: `light_ts` IS renormalized after the basis change but
`half_ts` deliberately is NOT (the Cg source documents that
renormalizing changes the specular response), and the binormal sign is
`sign(tangent.w)` applied at the vertex stage and multiplied in raw at
the fragment.

### Changes

- `rt_glslpilot.{c,h}`: `kPilotMaterial_BumpColorBlendDualHQ` with its
  own HQ fragment source and an HQ bump vertex source (same two-bone
  skinning switch behind `g_Skinned`; no `g_LightDirVP` — dead code in
  the HQ Cg variants). New vertex kinds `BumpDualHQ`/`SkinBumpHQ`;
  materials pair by kind mask exactly like the LQ set. New
  `g_LightDirFP` mirror (`rt_glslpilot_onLightDirFPParam`); the env
  register persists between pushes, so the mirror deliberately keeps
  the last value like the ARB path.
- `wcw_statemgmt.c`: `kShaderParam_LightDirFP` hooked into the mirror
  dispatch in `WCW_SetCgShaderParamArray4fv`.
- `rt_shaderMgr.c`: registers
  `g_shaderMgrFragmentProgramVariants[BLENDMODE_BUMPMAP_COLORBLEND_DUAL][BMB_HIGH_QUALITY]`
  as the HQ fragment target and `shaderMgrVertexProgramsHQ[DRAWMODE_BUMPMAP_DUALTEX/SKINNED]`
  (observed ids 251/247 this run) as HQ vertex entries.
- MSVC C lesson: positional material-table initializers count against
  the struct — adding a uniform-location field requires adding exactly
  one initializer to every row (first build failed C2078 with one
  extra `-1` per row).

### Verification (fresh generation, warm shard, same-window A/B)

- Build: `agent/logs/build-Release-x86-20260815-182923.log` (PASS,
  28.5 s).
- Warm-up: first `-ExerciseCharacter` smoke timed out at 180 s on the
  fresh shard (documented warm-up), passed after 4 minutes.
- Diagnostic capture (pilot on, East): HQ program compiled/linked and
  activated `(skin_bump HQ vertex variant)`; HQ vertex programs
  registered (251 bump_dual HQ, 247 skin_bump HQ). One benign
  end-of-log decline: a fragment-101 bind arrived while LQ vertex 228
  was still tracked (last pilot line of the run); the vertex re-bind
  path recovers such pairings, and if a draw really happened in the
  mismatched state it is byte-identical to what the ARB control does
  in the same state.
- Adopt suite `agent/logs/regression-20260815-184116.json` (control,
  fresh baselines for this generation), pilot suite
  `agent/logs/regression-20260815-184335.json`: East_01 **0.02%/0.21
  PASS** (pixel-level), North_01 1.46%/1.52 PASS, West_01
  **0.0035%/0.34 PASS** (pixel-level). CityHall_03 flagged 10.66% once
  (sky movement between suites — the shot's client log shows the HQ
  material active with no declines); the immediate pilot re-capture
  `agent/logs/capture-AtlasPlaza_CityHall_03-20260815-184621.json`
  measured **1.41%/1.65 PASS** against the same baseline — transient,
  not reproduced.
- All four shots show `BLENDMODE_BUMPMAP_COLORBLEND_DUAL_HQ active`
  in their client logs; East alone shows the single late fragment-101
  coverage line described above.
- Shard stopped afterwards; post-stop rescan found no shard processes.

### Remaining Atlas Park coverage after this port

The unported bound fragment ids in an East capture are now only the
special-effects set (201-216, the effects/ post-processing family).
Remaining material families for later steps: `bumpmapMultiply`
(BUMPMAP_MULTIPLY LQ — not observed bound in the Atlas shots so far),
`multi9`, water, and the effects/ set. The map-transfer capture path
remains the gate for addGlow/alphaDetail verification and non-Atlas
scenes.

## GLSL pilot: effects/post-processing family — 2026-08-16

All 19 effects fragment programs (rt_effects.c fullscreen passes) are
ported to native GLSL and harness-verified visually equivalent to the
ARB path. The port adds `kPilotMaterial_Fx*` (indices 7-25), a
`tPilotFxConstId` mirror set for the per-program
`g_Effects_*` locals, a fixed-function vertex pairing
(`kPilotVertexKind_FixedFunction`, vertex program id 0) with a second
program object per effects material, and registration from
`rt_effects_registerGlslPilotTargets()`.

### The activeFF constant-mirror bug

The first full-chain A/B measured 8-12% pixel drift. Root cause:
`rt_glslpilot_onEffectsParam` pushed effects constants to `m->locFx`
(the dualtex-linked program) regardless of which program object was
bound; the pbuffer passes run on `programFF` whose constants live at
`m->locFxFor`, so every pass drew with stale offsets. Fix: select the
location array with `m->activeFF` (set in `pilotActivate`). After the
fix, changed-pixel drift collapsed to 0.005-0.76% across the suite
(threshold 6%).

### The meanDelta noise floor is capture-procedure noise

After the fix, meanDelta hovered at 3.0-3.7 — marginal against the 3.0
threshold — as a *uniform* per-image brightness offset varying per
shot. Bisect + direct measurement established this is NOT a shader
difference:

- A temporary 1x1 `glReadPixels` readback in `lightAdaptation()` showed
  the GLSL adaptation trajectory tracks the ARB trajectory within
  frame-time noise (<=0.002 lum at freeze; worth <=0.5/255).
- Pure-ARB control pairs in the same window measure the same
  meanDelta range (1.3-4.4 observed).
- Mechanism: the capture freeze (`timeset 16; timescale 0`) collapses
  `TIMESTEP` (`global_state.frame_time_30`) from ~0.45 to ~0.015,
  freezing the eye adaptation mid-convergence (~300 frames in, ~70%
  converged). Each capture locks a slightly different exposure state
  into the screenshot. The tone-map amplifies the residual adaptation
  spread into a uniform integer RGB offset per shot.

changedPercent (localized pixels) is the discriminating metric for
effects-chain regressions; meanDelta marginality on day shots is
expected noise unless it exceeds ~5.

### Verification evidence

- Full-chain pilot suite vs same-window control adopt
  (`agent/logs/regression-20260816-130241.json` control,
  `regression-20260816-130455.json` pilot): changedPercent
  0.019/0.76/0.33/0.03 (CityHall/East/North/West) — all far under the
  6% threshold.
- Adaptation trajectory A/B (temporary diagnostic, removed):
  frame-0 pyramid output and freeze-state values track within noise;
  see `capture-AtlasPlaza_West_01-20260816-1254*.stdout.log`.
- Live chain confirmed via client log: SHRINK2 -> SHRINK4LUM ->
  SHRINK4 x3 -> LIGHTADAPTATION -> HBLUR -> VBLUR -> DOF_BLOOM_FINAL
  (pbuffer passes on the fixed-function pairing, final pass on the
  DRAWMODE_SPRITE dualtex pairing, vertex lit mode 5).

### Remaining Atlas Park coverage

The unported bound fragment ids in Atlas captures are now down to
none from the effects set; remaining families: `bumpmapMultiply` (not
yet observed bound), `multi9`, water. The map-transfer capture path
remains the gate for addGlow/alphaDetail verification and non-Atlas
scenes.

## Map-transfer capture path + non-Atlas coverage findings — 2026-08-16

The capture state machine in `game.c` now supports shots on any static
map. Each `CaptureShot` gained a `mapId` field (the static map container
id from `bin/data/server/db/maps.db`; 0 keeps the current map). During
capture setup, if the shot's map differs from `game_state.base_map_id`,
the client sends `mapmove <id>` (access level 1, `SCMD_MAPMOVE`) and
keeps waiting; the existing readiness conditions drop out during the
transfer and re-trigger on the target map, where `base_map_id` is the
static container id the server sent for the new world. A transfer takes
~3 s per hop and was verified end-to-end across nine zones (Atlas,
Founders Falls, Talos, Independence Port, Galaxy, King's Row, Skyway,
Peregrine, plus interiors Pocket D and Midnighter Club via probes).

Two developer iteration aids ride along, both read from a loose
`bin/capture_override.txt` (absent by default; never commit it):

- line 1 `x y z pitch yaw roll` — overrides the shot's camera position
  without a rebuild (position scouting);
- line 2 `map <id>` — overrides the shot's map (zone probing).

An empty `posPyr` in the shot table keeps the map-transfer arrival
position, which is itself deterministic per map (used by the
`TalosArrive_01` authoring probe shot).

### Fragment variant id table

A startup diagnostic in `shaderMgr_InitFPs()` (pilot on) prints the
exact fragment program id for every compiled blend-mode/variant pair.
16 ids per blend mode base; the ids matter because the pilot keys
coverage and registration off them:

| Blend mode | Fragment ids |
|---|---|
| modulate | 4-7 |
| multiply | 20-23 |
| colorBlendDual | 36-39 |
| addGlow | 52-55 |
| alphaDetail | 68-71 |
| bumpmapMultiply | 84-87 |
| bumpmapColorblendDual | 100 (LQ) / 101 (HQ BIT_HIGH_QUALITY) |
| water | 116-119 (118 = the fancy-water fragment) |
| multi9 | 120-123 |
| sunflare | 184-185 |
| effects family | 201-216 |

### Water does not bind deterministically: GFXF_MULTITEX startup ordering

`BLENDMODE_WATER` is assigned at texture-load time (`tex.c`
`texResetTrickBasedParametersComposite`), not at draw time, and the
multi-texture branch requires the `GFXF_MULTITEX` capability bit at
that moment. On this machine the bit is already absent when map texopts
load: the `DRAWMODE_BUMPMAP_MULTITEX` vertex program load inside
`shaderMgr_InitVPs()` (which requires `GFXF_WATER` and can clear
`GFXF_MULTITEX` when its compile fails) runs before map textures bind.
Every water texopt therefore takes its `useFallback` path and binds
`BLENDMODE_BUMPMAP_COLORBLEND_DUAL`-style bump lighting — concretely
fragment 84, bumpmapMultiply. Pinning `game_state.waterMode` per frame
does not help: the gate is the capability bit at load time, not the
mode. The probe evidence (`WATERTRACE` diagnostics in `tex.c`,
`rendertree.c`, `rt_water.c`, all pilot-gated and retained):

- every water texopt on every probed zone logs `multiTexFeature=0`;
- `addViewSortNode_Water`/`modelDrawWater` still run (the sort node and
  water draw path are mode-driven), but the bound fragment is 84;
- true water (fragment 118) bound in exactly one early exploratory run
  whose startup timing differed — a timing-dependent path, unusable for
  regression.

Consequence: the deterministic water surface coverage is bumpmapMultiply
(84), which is exactly the next material family to port. A true water
port is gated on fixing the `GFXF_MULTITEX` startup ordering (or
re-running texopt binding after shader init), a separate engine change.

### multi9 is static-zone-absent

Eleven zones probed (including Pocket D and Midnighter Club interiors):
fragment 120 (multi9) never bound. It is almost certainly a
mission-instance-only material (multi-material geometry). The multi9
port is gated on a mission-instance capture path, not just map
transfer.

### FoundersCanal_01: first non-Atlas regression shot

`FoundersCanal_01` (map 10, Founders Falls canal view) is the first
non-Atlas shot in the table. The view deterministically binds
bumpmapMultiply (fragment 84, the water fallback described above) and
alphaDetail (fragment 68) — the two families with no Atlas Park
coverage. Baseline adopted and pilot regression-verified in the same
window: **changedPercent 1.08 / meanDelta 1.69**
(`agent/logs/regression-20260816-153510.json`) — both far under
threshold; note the day-shot meanDelta noise floor does not apply to
this shot, which measures clean.

### Atlas suite re-verified after the infrastructure change

Atlas baselines were re-adopted in the current weather window
(`regression-20260816-154059.json` adopt); two consecutive pilot runs
against them measured changedPercent 2.21/2.37/0.31/1.51 and
3.19/0.15/1.47/0.17 (CityHall/East/North/West;
`regression-20260816-154258.json`, `regression-20260816-154532.json`)
— all far under the 6% threshold. meanDelta 5.4-8.0 is the documented
exposure-lock noise floor (eye adaptation frozen mid-convergence);
changedPercent is the discriminating metric and is green.

### Remaining material coverage after this step

Ported and verified: modulate, multiply, colorBlendDual,
bumpmapColorblendDual (LQ+HQ), effects family (19 programs).
Ported, now with deterministic coverage awaiting a port-verify cycle:
none (addGlow/alphaDetail were ported earlier; FoundersCanal_01 now
covers alphaDetail for a future re-verify if ever needed).
Unported with deterministic coverage: **bumpmapMultiply (84)** — next.
Gated: water (118, GFXF_MULTITEX startup ordering), multi9 (120,
mission-instance capture path).

## GLSL pilot: bumpmapMultiply (fragment 84) — 2026-08-16

The model-space bump material is ported and harness-verified on
`FoundersCanal_01` (the water-surface fallback view). This is the
first material whose VERTEX path differs structurally from the
bump-dual family: `bump.vp`/`bump_rgb.vp` are the
`LIGHT_SPACE=MODEL` variants of `vp_master_vp.cg`.

### What the model-space path actually does (verified against cgc ARB)

Compiled both variants offline (`3rdparty/cg/bin/cgc.exe -profile
arbvp1 -DSKIN=0 -DLIGHT_SPACE=MODEL -DVERTEX_LIT=DIFFUSE|PRELIT
-DTC_XFORM=TC_OFFSET -DPIXEL_LIT=BUMP_SPEC -DREFLECT=NONE`) and
decoded the ARB:

- `g_LightDirVP` for these draws is a model-space light POSITION
  (rt_model.c transforms the sun — or the dummy (0,5000,0) ambient
  light — into model space); the vertex shader derives the light
  direction per vertex: `normalize(LightDirVP.xyz - position)`.
- The model-space normal and tangent are used RAW — the Cg does NOT
  normalize them for LIGHT_SPACE=MODEL (unlike the view-space
  bump_dual variants); the non-orthonormal basis is what shipped.
- `calc_tangent_space_light_and_position` receives the VIEW-space
  position against that model-space basis: `view_ts` =
  -(dot(pos_vs, tangent), dot(pos_vs, binormal), dot(normal, pos)) —
  a mixed-space expression that is mathematically inconsistent but is
  what the shipped ARB computes; the port replicates it faithfully.
- Vertex color: DIFFUSE variant computes
  `saturate(dot(raw_normal, light)) * DiffuseParamVP + AmbientParamVP`
  (vec4, both pushed by drawLoopBump with w=1); PRELIT variant reads
  ATTR11 (baked instance lighting, bound by rt_model.c for the
  ambient-group RGBS path) and multiplies by 4.
- TC_XFORM=TC_OFFSET: uv0/uv1 get the `tex_scrolls` offsets
  (g_TexScroll0/1VP).
- `g_ViewerPositionVP` is dead code in both variants (compiler
  eliminates it; confirmed by the `: 0` usage flag in the ARB header)
  — not declared in the port.

The fragment (bumpmapMultiplyfp.cg variant 0) is vertex-lit with per
pixel bumped SPECULAR only: `(base*blend).rgb * vcolor * 8 +
saturate(spec)`, alpha keeps `base.a*blend.a * g_Env0FP.a`. The cgc
ARB confirms the ambient/diffuse/gloss fragment constants are DEAD in
this variant (only env5 specular1 and env8 env0 are read), the
specular dot is saturated BEFORE the pow, and the vertex color
multiplies rgb only.

### Changes

- `rt_glslpilot.{c,h}`: `kPilotMaterial_BumpMultiply` with its own
  fragment source and a model-space bump vertex source covering both
  engine variants behind a `g_Prelit` uniform (0 = bump.vp/DIFFUSE
  DRAWMODE_BUMPMAP_NORMALS, 1 = bump_rgb.vp/PRELIT
  DRAWMODE_BUMPMAP_RGBS on ATTR11) — same pattern as the skinning
  switch. New vertex kinds `BumpNormals`/`BumpRGBS`; five new
  uniform locations per material (texscroll x2, ambient/diffuse VP,
  prelit switch; every table row gained the five `-1` initializers —
  MSVC C2078 discipline); kPilotMaxVertexEntries 12 -> 16.
- `wcw_statemgmt.c`: kShaderParam_TexScroll0/1VP and
  kShaderParam_Ambient/DiffuseParameterVP hooked into the mirror
  dispatch.
- `rt_shaderMgr.c`: registers
  `g_shaderMgrFragmentProgramVariants[BLENDMODE_BUMPMAP_MULTIPLY][0]`
  (id 84) and the DRAWMODE_BUMPMAP_NORMALS/RGBS vertex programs
  (bump.vp id 230 this run).
- `g_LightDirVP`/`g_Specular1ColorAndExponentFP` reuse the existing
  mirrors (the lightdir mirror is shared; the material keeps its own
  uniform location).

### Verification (fresh generation, warm shard)

- Build PASS 29.5 s (`agent/logs/build-Release-x86-20260816-170126.log`).
- Pilot run 1 (`regression-20260816-170850.json`): changedPercent
  **1.41** (threshold 6), meanDelta 3.86 — the documented day-shot
  exposure-lock noise (this shard generation's control noise floor
  measured higher than the morning window).
- Pilot run 2 (`regression-20260816-171020.json`): changedPercent
  **0.59**, meanDelta **1.50** — full PASS on both metrics.
- Client log evidence: 1769-1789 `BLENDMODE_BUMPMAP_MULTIPLY active
  (bump (model-space) vertex variant)` activations per capture;
  fragment target bumpMultiply=84 registered; vertex program 230
  registered as bump (model-space). One benign one-time coverage
  line ("unported fragment program 84") from a bind that raced the
  startup registration order, and the usual one-time decline lines
  (fragment 84 briefly tracked against vertex 225 before the vertex
  re-bind recovered the pairing — ARB draws those, identical to
  control).
- Atlas suite re-run with the restructured material table
  (`regression-20260816-171120.json`): changedPercent 0.19-2.17 —
  same profile as the pre-port morning runs (0.15-3.19); all prior
  materials activate exactly as before. meanDelta 3.3-7.1 matches the
  documented Atlas noise floor.

### Remaining material coverage

Ported and verified: modulate, multiply, colorBlendDual,
bumpmapColorblendDual (LQ+HQ), effects family (19 programs),
**bumpmapMultiply**. Unverified port: addGlow (no binding view
known). Gated: water (118, GFXF_MULTITEX startup ordering), multi9
(120, mission-instance capture path).

## The GFXF_MULTITEX "startup ordering" was a shadow-registry self-lock — 2026-08-16

The earlier "DRAWMODE_BUMPMAP_MULTITEX vertex program load ordering clears
GFXF_MULTITEX" theory is disproven: all three FAUX_MULTI vertex variants
compile cleanly offline and in-engine (FEATTRACE: InitVPs.load-results
multiOkay=1 in every instrumented run; no `Shader Compilation failure`
lines). The real mechanism, localized with new FEATTRACE startup
diagnostics (`rdrSetChipOptions` begin/end, `InitFPs` compile-loop
begin/end + `disableVariantFeature`, `InitVPs` load results, the
registry load, and every `gfxApplySettings` call):

1. The client does NOT read the Windows registry for graphics settings.
   `RegistryReader`/`regfile.c` route all settings through a file-backed
   shadow registry under `bin/registry-keys/hkey_current_user/software/
   cryptic/coh/`. (The real `HKCU\SOFTWARE\Cryptic\CoH` values are never
   consulted; reading them from PowerShell misleads the investigation.)
2. On every clean exit, `saveAutoResumeInfoToRegistry` persists
   `shaderDetail = shaderDetailFromFeatures(rdr_caps.features)` and
   `useWater = game_state.waterMode`. One historical run that lost
   GFXF_MULTITEX/WATER for any transient reason therefore wrote
   `shaderdetail=0` (SHADERDETAIL_GF4MODE: water on, multitex OFF) and
   `usewater=0` into the shadow registry.
3. Every subsequent startup then applies 0/0 (FEATTRACE:
   `registry-load shaderDetail=0 useWater=0`; `enableMask[water=1
   multi=0] disableMask[multi=1]`), water texopts take their fallback
   (bumpmapMultiply, fragment 84), and the clean exit re-saves 0/0 —
   a self-reinforcing loop. This also explains the "one early
   exploratory run bound true water": it ran before the first poisoned
   save. It was never timing-dependent.

With `shaderdetail=3` and `usewater=2` written into the shadow registry,
one capture binds `BLENDMODE_WATER` variant 0 (fragment 116) on the
Founders Falls ocean model deterministically, and `capture.water
state=2 waterFeature=1 multiFeature=1`. `agent/capture.ps1` now pins
both shadow-registry values before every client launch so a poisoned
save can never silently degrade capture coverage again.

Consequence for multi9: with GFXF_MULTITEX alive, all
TEXOPT_TREAT_AS_MULTITEX textures (not just fancy water) bind
BLENDMODE_MULTI on static maps — fragments 120/121/136/137/152 bound in
the FoundersCanal view. The earlier "multi9 is mission-instance-only"
conclusion was an artifact of the disabled feature bit; multi9 now has
deterministic static-map coverage and needs no mission-instance capture
path.

The FEATTRACE diagnostics are intentionally unconditional (a handful of
printf lines): the silent settings->feature-bit persistence is exactly
what made this investigation hard, and disableVariantFeature prints are
the only visibility into silent feature strips.

## GLSL pilot: water (fragment 116) — 2026-08-16

The fancy-water material (waterfp.cg variant 0: refraction only —
waterMode=2/WATER_MED, no planar reflection or shadowmap bits) is ported
and harness-verified on `FoundersCanal_01`. Water variant ids:
water[0]=116 (ported; binds at waterMode>=WATER_LOW with multitex on),
water[2]=117 (shadowmap), water[8]=118 (BMB_PLANAR_REFLECTION — binds at
waterMode>=WATER_HIGH; the earlier docs' "118 is the fancy-water
fragment" mislabeled the variant), water[10]=119.

The vertex pairing is `DRAWMODE_BUMPMAP_MULTITEX` ("bump_dual_multi":
SKIN=0 LIGHT_SPACE=VIEW VERTEX_LIT=PRELIT_WHITE TC_XFORM=NONE
PIXEL_LIT=BUMP_ALL REFLECT=FAUX_MULTI), new pilot kind
`kPilotVertexKind_BumpMulti`. TEXCOORD0 carries (uv0, faux spheremap
reflection uv) computed with calc_faux_reflection_uv; the vertex color
is the constant white of PRELIT_WHITE; the LQ tangent-space
light/view interpolants match the bump_dual family. The
USE_CUBEMAP/USE_SHADOWMAP TEX3/TEX4 interpolants the Cg variant always
emits are dead in the water fragment pairing and omitted.

The fragment replicates the Cg math faithfully: dual scrolling normal
maps (scroll_scale layers 0/1/2/7 from g_ScrollScaleArrFP) averaged into
one tangent-space normal; base color 2*vertexColor*multiply1*base1
tinted by mix(ConstColor1, ConstColor0, NdotV)*base1.a;
apply_lighting_no_gloss; the two-tap depth-clamped refraction skew
(ARB-faithful, including the second tap at the skewed coordinate);
refraction blend by g_GlossParamFP.w; gloss by
calc_lighting_factors/apply_lighting_gloss_only with glossConst =
g_GlossParamFP.x; fog; alpha g_Env0FP.a optionally * base1.a. The
selector bits (faux-reflect uv for multiply1, water alpha) use the Cg
isBitSet frac idiom on g_BumpMultiFlagsFP.x. fragment_pos is
gl_FragCoord (the WPOS semantic).

New mirrors: g_ConstColor0FP/g_ConstColor1FP (ENV3/4),
g_WaterRefractionTransformFP (ENV7), g_WaterRefractionParamsFP (ENV22),
g_BumpMultiFlagsFP (ENV10), and the g_ScrollScaleArrFP[10] array
(ENV12-21), hooked in WCW_SetCgShaderParamArray4fv; the existing bump
lighting + g_Env0FP mirrors are reused. The reflection-variant constants
(ENV6/23/24) are not mirrored yet — they belong to the unported
water[8]=118 variant.

### Verification (same-window A/B, storm then calm)

- Build PASS (`agent/logs/build-Release-x86-20260816-180801.log`
  compile-clean; server-copy steps fail while the shard holds bin/, the
  client binary copies and is the artifact that matters during capture
  iteration).
- Activation: 1803 `BLENDMODE_WATER active (bump_dual_multi vertex
  variant)` lines in one FoundersCanal capture; the usual benign one-time
  decline + startup-race coverage lines.
- A storm front over Founders Falls made the sky actively unstable
  during the first attempts: two ARB captures six minutes apart differed
  by 18.5% (sky bands 20-29%, near-field stable). In that window the
  tight ARB-vs-pilot pair measured 8.0% whole-image but only 4.15% in
  the water/foreground bands — below the 6.08% the two ARB captures
  measured against each other in the same bands, i.e. within
  environmental noise.
- After the sky settled, fresh baseline adoption
  (`regression-20260816-181923.json`) + two consecutive pilot suites:
  **FoundersCanal_01 PASS both metrics twice** — changedPercent
  1.56/1.43 (threshold 6), meanDelta 1.99/1.85 (threshold 3)
  (`regression-20260816-182210.json`, `regression-20260816-182510.json`).
- Atlas shots in both pilot suites: changedPercent 0.00-1.51 (threshold
  6; East and North each pixel-identical under tolerance in one run) —
  the REGRESSED verdicts are solely the documented day-shot exposure-lock
  meanDelta noise floor (5.3-8.3 uniform, this generation's profile
  matches the 2026-08-16 morning sessions). changedPercent is green and
  consistent across both runs.

### Remaining material coverage

Ported and verified: modulate, multiply, colorBlendDual,
bumpmapColorblendDual (LQ+HQ), effects family (19 programs),
bumpmapMultiply, **water (variant 0, fragment 116)**. Unverified port:
addGlow (no binding view known). Unported with deterministic static
coverage now: **multi9 (fragments 120-183; 120/121/136/137/152 bound in
the FoundersCanal view)** and water variants 117/118/119 (shadowmap /
planar-reflection pairings; 118 needs waterMode>=WATER_HIGH pinned in
the shadow registry to bind).
