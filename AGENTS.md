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

## Current milestone status

Phase 0 (local development loop), Phase 1 (deterministic graphical capture), and Phase 2 (multi-scene capture + regression harness) are all complete and verified on 2026-08-15:

1. deterministic map selection and teleport — done (`capture <label>` selects from the shot table in `Game/src/game.c`)
2. fixed camera/FOV and hidden UI — done (visually verified in the produced images)
3. a repeatable screenshot command and clean client exit — done (verified runs with exit code 0)
4. machine-readable capture success/failure — done (`agent/capture.ps1 -Json`)
5. multi-scene deterministic capture — done (five Atlas Park shots; `agent/capture-regression.ps1`)
6. capture-comparison regression harness — done (`agent/compare-captures.ps1`; five shots pass at 0.03–1.7% pixel drift, cross-scene comparisons correctly fail)

Known operational constraints:

- The shard needs a warm-up period (a few minutes) after `agent/start-shard.ps1` before logins are admitted. Do not treat a failure as real until it reproduces on a warmed shard that has passed one `agent/smoke.ps1 -ExerciseCharacter` run.
- Deterministic lighting requires the server-side time freeze (`timeset 16; timescale 0`, sent by the capture setup). The server only accepts those commands at access level 9, so capture accounts need `AccessLevel=9` in `cohdb.dbo.Ents` (one-time SQL update per account, or `DefaultAccessLevel 9` in `servers.cfg` before creating the character).
- The world clock runs at `DAY_TIMESCALE` 48 by default; without the freeze, images taken minutes apart differ by in-game hours of lighting.
- The first capture(s) on a fresh mapserver generation catch the sky/sun systems mid-transition to the frozen clock (a fresh shard may need more than the one discarded warmup before the sky settles); `agent/capture-regression.ps1` runs a discarded warmup capture to absorb this. Run the regression suite (not bare `capture.ps1`) when comparing against baselines, and re-verify a single-suite "regression" with a follow-up capture before treating it as real.
- Weather is per-shard-environment state and evolves on ~10-minute scales during storms (sun glare blooming/fading behind the overcast; measured up to ~9% drift on sky-heavy shots), while clear weather is stable all day. When the weather differs from the committed baselines (~30%+ uniform drift), re-adopt baselines with one suite run and run the comparison suite immediately after (same-window A/B). A systematic shader failure flags all shots; weather noise shows as 2-5% sky-region drift on a subset. The war walls (translucent alpha-pass map-edge barriers, visible in the East/West shots) are stable game content rendered identically by both paths and are not a comparison risk.
- Day-shot captures freeze the eye adaptation mid-convergence: the capture time freeze collapses `TIMESTEP` (frame_time_30 ~0.45 -> ~0.015), locking a random ~70%-converged exposure into each screenshot. This shows as a uniform per-shot integer RGB offset: meanDelta 1.3-4.4 on pure-ARB control pairs in the same window. For effects-chain regressions, changedPercent is the discriminating metric (GLSL-vs-ARB passes at 0.02-0.76%); treat meanDelta marginality under ~5 as noise unless it reproduces consistently.
- Shots framing the player character up close vary with the idle-animation phase; `AtlasPlaza_Closeup_01` is therefore excluded from the default regression suite (available via `-Targets`).
- Capture shots can live on any static map: the shot table in `Game/src/game.c` has a `mapId` field, and the capture state machine sends `mapmove <id>` (access level 1) then waits for readiness on the target map (~3 s per hop; verified across nine zones). A loose `bin/capture_override.txt` (not committed) can override the camera position (line 1: `x y z pitch yaw roll`) and the map (line 2: `map <id>`) for zone probing without a rebuild.
- Water/multitex coverage depends on the client's file-backed shadow registry (`bin/registry-keys/hkey_current_user/software/cryptic/coh/` — NOT the real Windows registry): on every clean exit the client persists `shaderDetail` derived from the run's feature bits, so one run that lost `GFXF_MULTITEX` poisons `shaderdetail=0`/`usewater=0` permanently (every later run re-applies and re-saves it). With `shaderdetail=3` + `usewater>=1` the Founders Falls canals bind the fancy-water material (fragment 116) and multi9 (fragments 120+) deterministically; `agent/capture.ps1` pins both values before each launch. FEATTRACE startup diagnostics print the feature-bit lifecycle (registry load, gfxApplySettings, rdrSetChipOptions, InitFPs/InitVPs) and `disableVariantFeature` strips are never silent.

Shader-path findings (2026-08-15, evidence in `docs/agent-status.md`):

- The default and only verified full-scene shader path is Cg compiling to ARB assembly (`useCg 1`). `useCg 0` uses precompiled ARB programs without Cg.
- `useCg 2` (Cg->GLSL) is **broken** with the shipped shader sources: the custom `TIE(ENVn)` constant-bind semantics crash the Cg 2.2 `glslf`/`glslv` compiler backends (reproduced offline with `3rdparty/cg/bin/cgc.exe`), and even with those guarded out the engine pushes constants to `program.env[]` registers that GLSL cannot read. Do not default to it.
- A **native GLSL pilot** renders `BLENDMODE_MODULATE`, `BLENDMODE_MULTIPLY`, `BLENDMODE_COLORBLEND_DUAL` (the costume-tint material), `BLENDMODE_BUMPMAP_COLORBLEND_DUAL` (the bumped dual-tint material, both the variant-0 LQ and the BIT_HIGH_QUALITY variant, fragment 101), `BLENDMODE_BUMPMAP_MULTIPLY` (fragment 84, the model-space bump material), and `BLENDMODE_WATER` variant 0 (fragment 116, the fancy-water material with depth-skewed refraction) through hand-written GLSL 1.20 compatibility-profile programs (`Game/src/render/thread/rt_glslpilot.{c,h}`), enabled with `-glslPilot 1` (or `agent/capture.ps1 -ExtraClientArgs "-glslPilot 1"`; `agent/capture-regression.ps1` forwards the same parameter for whole-suite runs). It is harness-verified visually equivalent to the ARB path day and night: the full 4-shot suite is green with all active materials confirmed in the client log. The simple materials share one replicated `vp_master_vp.cg` dualtex vertex shader (mode-switched by uniform); each bump variant has its own vertex shader covering both the static `bump_dual` variant and the skinned `skin_bump` variant (two-bone skinning through the mirrored bone-matrix array; on Atlas Park the bump-dual coverage is mostly the skinned player costume). The HQ bump fragment builds the tangent basis per pixel from normal/tangent/position interpolants and reads `g_LightDirFP` (TIE(ENV11), pushed only for HQ draws) instead of the vertex-interpolated light vector; `light_ts` is renormalized after the basis change but `half_ts` deliberately is not (matches the Cg/ARB specular response). The water material pairs with the FAUX_MULTI static vertex variant (`kPilotVertexKind_BumpMulti`: uv0 + faux spheremap reflection uv in TEXCOORD0, constant-white PRELIT vertex color) and reads `gl_FragCoord` as the WPOS `fragment_pos`. Mirrored engine constants cover `g_ReflectionParamVP`, `g_Env0FP`/`g_Env1FP` (constColor0/1), `g_GlowParamFP`, the bump set (`g_LightDirVP`, `g_LightDirFP`, `g_AmbientColorFP`/`g_DiffuseColorFP`/`g_GlossParamFP`/`g_Specular1ColorAndExponentFP`, `g_BoneMatrixArrVP`), and the water set (`g_ConstColor0FP`/`g_ConstColor1FP`, `g_WaterRefractionTransformFP`, `g_WaterRefractionParamsFP`, `g_BumpMultiFlagsFP`, `g_ScrollScaleArrFP[10]`). `BLENDMODE_ADDGLOW` is ported and registered but **unverified: no available capture view ever binds it** (night Atlas shots and nine map-transfer zones included). `BLENDMODE_ALPHADETAIL` is likewise ported but had no Atlas Park coverage until the map-transfer path landed; the `FoundersCanal_01` shot (Founders Falls canals) now binds it deterministically. The effects/ post-processing family (19 programs: shrink/shrink4/blurs/tonemap2/dofFinal/dofBloomFinal/lightAdaptation/brightpass/log/desaturate variants, `rt_effects.c` fullscreen passes) is also ported and harness-verified: the pbuffer passes pair with a fixed-function `ftransform()` vertex shader (vertex program id 0, `kPilotVertexKind_FixedFunction` — each effects material keeps a second GLSL program object for that pairing), the final composite pairs with the DRAWMODE_SPRITE dualtex variant, and the per-program `g_Effects_*` locals are mirrored by constant identity (`tPilotFxConstId`) with `m->activeFF` selecting which program object's uniform locations receive the push. Unported materials idle as safe fallbacks; with the pilot on, a per-process coverage diagnostic logs every unported fragment program id bound during a capture.
- The legacy fixed-function/ATI/NV-combiner branches (`R200`, `NV1X/NV2X`, `TEX_ENV_COMBINE`) cannot execute on modern drivers (extensions no longer exist) and are candidates for later deletion.

The next priorities are:

1. renderer material ports in small, harness-verified steps (modulate, multiply, colorBlendDual, bumpmapColorblendDual variant 0 + HQ variant 101, the effects/ post-processing set, `bumpmapMultiply` (fragment 84), and `water` variant 0 (fragment 116, refraction; verified on `FoundersCanal_01` with waterMode=2) all verified; addGlow ported but unverified — no binding view known). `multi9` (the big one) now has deterministic static-map coverage in the FoundersCanal view (fragments 120/121/136/137/152 bind with `GFXF_MULTITEX` alive) and no longer waits on a mission-instance capture path; water variants 117/118/119 (shadowmap/planar-reflection; 118 needs `usewater>=3`) are the remaining water-side step; `FoundersCanal_01` (map 10, Founders Falls canals) is the non-Atlas regression shot covering water, alphaDetail, and multi9
2. dead-path deletion (`shadersATI.c`, `shadersTexEnv.c`, and their `rt_state.c`/`wcw_statemgmt.c` branches) as bounded hygiene once a renderer change justifies touching those files
3. idle-animation phase determinism if close-up captures are needed for future shader work

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
