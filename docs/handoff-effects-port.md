# Handoff: GLSL Pilot Effects/Post-Processing Port

**Date**: 2026-08-16
**Branch**: `agent/glsl-pilot-materials`
**Status**: Uncommitted work in progress — **does not compile** (see Bug #1 below)

## What this is

We are porting the City of Heroes renderer from Cg→ARB assembly to native GLSL, material by material. A "GLSL pilot" (`Game/src/render/thread/rt_glslpilot.{c,h}`) intercepts ARB program binds and substitutes hand-written GLSL 1.20 compatibility-profile programs. The pilot is enabled with `-glslPilot 1` and verified against ARB output via `agent/compare-captures.ps1` (same-window A/B screenshot comparison).

The simple scene materials (modulate, multiply, colorBlendDual, addGlow, alphaDetail, bumpColorBlendDual + HQ) are **done and committed**. This handoff covers the **effects/post-processing chain** — the fullscreen-quad passes in `rt_effects.c` that implement HDR bloom.

## Where we are

All 19 effects fragment programs have GLSL source written and are wired into the pilot. The architecture works: the pilot compiles two GLSL program objects per effects material (one paired with the dualtex vertex shader for the final composite pass, one with a fixed-function `ftransform()` vertex shader for the pbuffer passes). Registration, activation, and constant mirroring are all wired.

**Same-window A/B testing showed ~8–12% pixel drift across the full chain.** Root cause was identified: `rt_glslpilot_onEffectsParam` at line 1916 pushes constants to `m->locFx` (the dualtex program's uniform locations) regardless of whether `m->program` or `m->programFF` is active. Pbuffer passes run on `programFF` whose constants live at `m->locFxFor`. Each pass draws with the previous pass's offsets.

A fix was partially applied but introduces a new compile error (see Bug #1).

## Known bugs to fix

### Bug #1: Material table initializer count mismatch (build-breaking)

`activeFF` and `failedFF` were added to the `tPilotMaterial` struct (lines 1151–1152 of `rt_glslpilot.c`). The **material table rows already include these fields** (they have 28 initializers matching 28 struct fields — this is correct as of the current disk state). **However**, the `rt_glslpilot_onEffectsParam` function has NOT been updated to check `m->activeFF`.

So the build *should* succeed right now — but the **runtime bug** (Bug #2) remains.

### Bug #2: `rt_glslpilot_onEffectsParam` pushes to wrong program object (runtime)

**File**: `Game/src/render/thread/rt_glslpilot.c`, line 1916–1927.

Current code:
```c
void rt_glslpilot_onEffectsParam( int fxConstSlot, const GLfloat* vec4 )
{
    if (( fxConstSlot < 0 ) || ( fxConstSlot >= kPilotFxConst_Count ))
        return;
    memcpy( s_fxConstMirrors[fxConstSlot], vec4, sizeof( s_fxConstMirrors[0] ) );
    if ( s_activeMaterial >= 0 )
    {
        tPilotMaterial* m = &s_materials[s_activeMaterial];
        if (( m->fxConstMask & kFxBit( fxConstSlot )) && ( m->locFx[fxConstSlot] >= 0 ))
            __glewUniform4fv( m->locFx[fxConstSlot], 1, s_fxConstMirrors[fxConstSlot] );
    }
}
```

**Problem**: Always uses `m->locFx` (the dualtex-linked program). When the fixed-function path is active (`m->activeFF == true`), it should use `m->locFxFor` (the FF-linked program). The `activeFF` field IS set in `pilotActivate()` (line 1712) but is never read in `onEffectsParam`.

**Fix**: Change the `onEffectsParam` function to select between `locFx` and `locFxFor` based on `m->activeFF`:
```c
if ( s_activeMaterial >= 0 )
{
    tPilotMaterial* m = &s_materials[s_activeMaterial];
    if ( m->fxConstMask & kFxBit( fxConstSlot ))
    {
        GLint loc = m->activeFF ? m->locFxFor[fxConstSlot] : m->locFx[fxConstSlot];
        if ( loc >= 0 )
            __glewUniform4fv( loc, 1, s_fxConstMirrors[fxConstSlot] );
    }
}
```

## Files modified (all uncommitted)

| File | What changed |
|------|-------------|
| `Game/src/render/thread/rt_glslpilot.c` | 19 GLSL fragment sources, fixed-function vertex source, `pilotInitFF()`, dual program selection in `pilotActivate()`, `rt_glslpilot_onEffectsParam()` mirror function, `programFF`/`locFxFor`/`activeFF`/`failedFF` struct fields, 19 material table rows |
| `Game/src/render/thread/rt_glslpilot.h` | 19 `kPilotMaterial_Fx*` enum values, `kPilotVertexKind_FixedFunction`, `tPilotFxConstId` enum, `rt_glslpilot_onEffectsParam()` declaration |
| `Game/src/render/thread/rt_effects.c` | `#include "rt_glslpilot.h"`, `rt_effects_registerGlslPilotTargets()` function that maps fragment program ids to pilot materials |
| `Game/src/render/thread/rt_effects.h` | `rt_effects_registerGlslPilotTargets()` declaration |
| `Game/src/render/thread/rt_shaderMgr.c` | Effects id→name diagnostic printf, calls to `registerGlslPilotTargets()`, registers vertex id 0 as `kPilotVertexKind_FixedFunction` |
| `Game/src/render/thread/wcw_statemgmt.c` | Effects constant dispatch hooks in `WCW_SetCgShaderParamArray4fv` for all 6 `kShaderParam_Effects_*` ids. Also fixed reflection param mirror to be unconditional (was only called when pilot active — real bug for 2D setup). |

## Architecture notes for the next agent

### Effects draw flow
1. `rt_effects.c` runs a chain of fullscreen-quad passes on pbuffer/FBO contexts
2. Each pass: bind effects fragment program → push per-program local constants → draw quad
3. Pbuffer passes run with **no vertex program bound** (tracked id 0, which the pilot treats as `kPilotVertexKind_FixedFunction`)
4. The **final pass** (DOF_BLOOM_FINAL) runs on the backbuffer with the DRAWMODE_SPRITE dualtex vertex program bound (vp id 222, vertex lit mode 5 = FF_UNLIT_GL)
5. Between passes, `OnGLContextChange` fires (pbuffer switch), fully resetting WCW state and deactivating the pilot. The pilot reactivates on each effects bind.

### Live Atlas Park chain (7 passes)
shrink(204) → shrink4LUM(207) → shrink4(206)×2 → shrink4(206) → lightAdaptation(209) → HBLUR(201) → VBLUR(202) → DOF_BLOOM_FINAL(216)

Fragment ids in parens are the ARB program ids the engine binds. tonemap(203), brightpass(211), log(210), shrink4Exp(208) are disabled in default config.

### Per-program local constants
Effects constants are `program.local[]` in the Cg sources. Overlapping slot numbers across programs mean the mirror is keyed by constant identity, not slot. The 6 constants are:
- `TextTransform` (most passes: texcoord scale+offset)
- `ExpectedLum` (tonemap2, brightpass, dofBloomFinal)
- `TimeStep` (lightAdaptation)
- `DofParam2` (dofFinal, dofBloomFinal)
- `DofProject` (dofFinal, dofBloomFinal)
- `DesaturateParam` (tonemap2_desat, dofFinal_desat, dofBloomFinal_desat, simple_desaturate)

### Why two program objects per effects material
Pbuffer passes have no vertex program. The GLSL pilot needs a vertex shader to form a complete program. We can't reuse the dualtex vertex shader because that one expects DRAWMODE_SPRITE's texcoords/pushing conventions. The fixed-function vertex shader uses `ftransform()` and `gl_TextureMatrix[i] * gl_MultiTexCoord[i]`, which is what the hardware does when no vertex program is bound.

## Fix-and-verify checklist

1. Apply Bug #2 fix to `rt_glslpilot_onEffectsParam` (3-line change)
2. Build: `.\agent\build.ps1 -Configuration Release -Platform x86`
3. Start shard: `.\agent\start-shard.ps1` (wait for warm-up)
4. Run smoke test: `.\agent\smoke.ps1 -ExerciseCharacter -AccountName Dummy00009 -TimeoutSeconds 180`
5. Run A/B capture regression:
   - ARB baseline: `.\agent\capture-regression.ps1 -Targets CityHall,North,East,West -BaselineDir agent/baselines`
   - GLSL pilot: `.\agent\capture-regression.ps1 -Targets CityHall,North,East,West -BaselineDir agent/baselines -ExtraClientArgs "-glslPilot 1" -CaptureDir agent/captures-glsl`
   - Compare: `.\agent\compare-captures.ps1 -BaselineDir agent/baselines -CaptureDir agent/captures-glsl`
6. Target: all 4 shots < 2% drift (the simple materials already achieve 0.03–1.7%; effects drift should be in the same range once the constant-mirror bug is fixed)
7. Update `docs/agent-status.md` and `AGENTS.md` with effects port results
8. Commit with message describing the effects port

## Other notes

- Baseline images may need re-adopting if weather or time-of-day differs from the committed baselines (~30%+ uniform drift = weather, not a shader bug). Run the regression suite and re-adopt if needed before comparing.
- The first capture on a fresh mapserver catches sky/sun mid-transition; `capture-regression.ps1` runs a discarded warmup to absorb this. Always use the regression suite, not bare `capture.ps1`.
- The reflection param mirror fix (unconditional `rt_glslpilot_onReflectionParam` call in `wcw_statemgmt.c`) was already applied and should not be reverted.
- Fragment ids 201–216 are the effects ids observed bound in Atlas Park captures. After this port, the only remaining unported ids observed in Atlas Park should be 201–216 (the effects we're fixing now). The coverage diagnostic (`noteUnportedFragmentBind`) in the client log will confirm.
