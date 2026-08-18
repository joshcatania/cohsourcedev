# Issue #9: GLSL pilot planar reflection

## Scope

This change ports the shipped `BIT_PLANAR_REFLECTION` water permutation to the
native GLSL pilot. It does not add the combined shadow-plus-planar permutation,
change the legacy Cg/ARB path, or change the water-mode policy.

## Shipped math verified before the port

The reference was `agent/shadersrc/shaders/cgfx/waterfp.cg` and its shipped
helpers, rather than an assumption about current water rendering.

- `texture3` / TEXUNIT3 is the planar-reflection source pbuffer. `texture5`
  and `texture6` are the refraction color and depth sources.
- The two bump normals are averaged, then the normal, view, light, and half
  vectors are normalized in tangent space.
- The reflection UV starts at `IN.fragment_pos.xy *`
  `g_WaterReflectionTransformFP.xy`, flips Y as `y = y * -1 + 1`, and adds
  the normal skew multiplied by `g_WaterReflectionParamsFP.x`.
- The color order is base/tint interpolation, no-gloss lighting, refraction
  mix, then planar reflection. Reflection uses the shipped Fresnel helper:
  `saturate(baseBias + scale * pow(1 - saturate(dot(eye, normal)), power))`.
  The reflection amount is `(1 - gloss) * reflectionScale * fresnel`, and
  the existing gloss contribution is added afterward.
- The shipped static-water `bump_dual_multi` fragment path samples the
  reflection pbuffer from WPOS-derived UVs. `PlanarReflectionPlaneVP` is
  pushed by the surrounding water code but is not consumed by this fragment
  permutation, so it is intentionally not mirrored as a GLSL uniform.

## Implementation

The GLSL pilot now has a separate `kPilotMaterial_WaterPlanar` target mapped
to the shipped water fragment program variant 118. It uses the existing
`bump_dual_multi` vertex program 234, samples reflection from TEXUNIT3, and
mirrors the reflection transform, reflection parameters, and Fresnel
parameters through the existing water-constant path. The runtime selector is
kept separate so the ordinary water and shadow permutations remain unchanged.

Bounded `WATERTRACE` diagnostics cover the candidate score, reflection request,
live pbuffer, selected blend bits, and final fragment/vertex/kind pairing.

## Runtime evidence

The explicit planar recipe was:

```powershell
.\agent\capture.ps1 -Target FoundersCanal_01 -AccountName Dummy00018 `
  -TimeoutSeconds 300 `
  -ExtraClientArgs '-glslPilot 1 -useWater 4 -reflectionEnable 1 -separate_reflection_viewport 1 -shadowMode 0' -Json
```

The fixed camera override was temporary and removed after the paired captures;
the file-backed registry shadow settings were also restored (`shadowmode=1`,
`shadowmapshader=0`). The successful runtime trace showed:

- `waterMode=4`, `reflectionEnable=1`, candidate score `8.044293e-02`;
- `doWater=1`, `requested=1`, `separate=1`, and a non-null reflection pbuffer;
- `BLENDMODE_WATER_PLANAR`, fragment `118`, vertex `234`, kind
  `bump_dual_multi`;
- clean capture completion and client exit.

The shadow safeguard was also exercised with the shipped static-water family:
fragment `117`, `blendBits=2`, `bump_dual_multi`, clean capture and exit. The
temporary shadow registry values used for that check were not retained.

## Parity results

For the same fixed view and explicit planar recipe, ARB/Cg control versus GLSL
pilot passed with the existing comparison policy:

| Metric | Result | Threshold |
| --- | ---: | ---: |
| Changed pixels | 0.1554% | 6.0% hard limit |
| Mean delta | 0.6548 | 3.0 advisory |
| Max delta | 163 | report only |

A repeated same-view base-water safeguard also passed: `0.1536%` changed
pixels, mean delta `0.8744`, max delta `164`. The committed Founders baseline
was not used as a shader verdict because its camera framing differs from the
fixed runtime view; that comparison correctly reported a framing mismatch.

The representative GLSL pilot Atlas suite passed 4/4:

- `AtlasPlaza_CityHall_03`: `0.2066%` changed pixels
- `AtlasPlaza_East_01`: `2.0321%`
- `AtlasPlaza_North_01`: `0.0141%`
- `AtlasPlaza_West_01`: `1.8415%`

The suite’s mean-delta advisories were reported by the harness and are
consistent with the repository’s documented exposure/weather noise policy.

## Build and smoke

- `agent/doctor.ps1 -Json`: passed.
- `agent/build.ps1 -Configuration Release -Platform x86`: passed after the
  change using the verified v145 fallback.
- `agent/start-shard.ps1 -StartupWaitSeconds 60`: passed.
- `agent/smoke.ps1 -ExerciseCharacter -AccountName Dummy00009
  -TimeoutSeconds 180 -Json`: passed direct-DB character creation and
  MapServer entry with TestClient exit code 0.
