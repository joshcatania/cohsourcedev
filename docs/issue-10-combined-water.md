# Issue #10: GLSL pilot combined shadow and planar water

## Scope and preflight

This change composes the already-verified issue #6 shadow permutation and
issue #9 planar-reflection permutation. The shipped reference was
`agent/shadersrc/shaders/cgfx/waterfp.cg`. Its existing order is preserved:
shadowed lighting, refraction, planar reflection, gloss, then fog. The
combined source has no additional interaction or math, so no new water math
was added.

The legacy preflight selected the symbolic combined permutation
`BMB_SHADOWMAP | BMB_PLANAR_REFLECTION` (table index 10), generated fragment
program `119`, rendered with `blendBits=10`, and supplied a non-null planar
reflection pbuffer.

## Implementation

The GLSL pilot now has a distinct
`kPilotMaterial_WaterShadowPlanar` row. It reuses the existing
`s_waterFragmentSource`, water samplers, and static `bump_dual_multi` vertex
source. Both existing constant blocks are enabled for this row:

- shadow constants for `g_UseShadowMap`, CSM parameters, and shadow samplers;
- planar constants for the reflection transform, reflection parameters, and
  reflection pbuffer.

The target is registered symbolically from
`g_shaderMgrFragmentProgramVariants[BLENDMODE_WATER][BMB_SHADOWMAP |
BMB_PLANAR_REFLECTION]`; no historical program ID is used for selection.

## Combined capture evidence

The paired Founders Falls canal capture used the issue #9 fixed view and these
temporary settings:

```text
useWater=4
reflectionEnable=1
separate_reflection_viewport=1
shadowmode=2
shadowmapshader=1
shadowmapdistance=0
shadowmapsize=0
glslPilot=1
```

The final pilot trace showed `BLENDMODE_WATER_SHADOW_PLANAR`, fragment `119`,
vertex `234`, kind `bump_dual_multi`, and `blendBits=10`. The capture exited
cleanly.

## Fixed-view parity matrix

All four rows were compared against their same-view ARB/Cg controls with the
existing harness policy: 12 channel tolerance, 6% hard changed-pixel limit,
and 3 mean-delta advisory.

| Variant | Fragment | Changed pixels | Mean delta | Max delta | Verdict |
| --- | ---: | ---: | ---: | ---: | --- |
| Base water | 116 | 0.6709% | 0.9237 | 143 | PASS |
| Shadow | 117 | 0.4714% | 1.1026 | 141 | PASS |
| Planar reflection | 118 | 0.8086% | 1.3222 | 163 | PASS |
| Shadow + planar reflection | 119 | 0.1095% | 0.8939 | 49 | PASS |

## Representative scene, build, and smoke checks

- Atlas City Hall, East, North, and West each passed the formal comparison in
  final runs. The first four-shot batch was 3/4 because City Hall had a
  transient 93.2% capture mismatch; its isolated rerun passed at 2.274%.
- `agent/doctor.ps1 -Json`: ready; the documented non-blocking v142 probe
  warning remains, with the v145 fallback passing.
- `agent/build.ps1 -Configuration Release -Platform x86`: passed.
- `agent/start-shard.ps1 -StartupWaitSeconds 60`: passed.
- `agent/smoke.ps1 -Json`: passed direct-DB login with TestClient exit code 0.
- `agent/smoke.ps1 -ExerciseCharacter -AccountName Dummy00001
  -TimeoutSeconds 120 -Json`: passed character creation and MapServer entry
  with exit code 0. The requested `Dummy00009` retry timed out at the DbServer
  queue-response stage without a login or fatal error; the alternate local
  development account completed the same staged check.

## Restoration

The temporary capture override was removed. The file-backed registry was
restored to its preflight values:

```text
shadowmode=1
shadowmapshader=0
shadowmapdistance=-2
shadowmapsize=0
shaderdetail=2
usewater=2
```
