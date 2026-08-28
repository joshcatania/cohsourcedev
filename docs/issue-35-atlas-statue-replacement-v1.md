# Issue #35 — Atlas statue replacement v1

Status: bounded replacement experiment complete; runtime gate **PASS**, visual gate **NOT PASS**.

Branch: `agent/atlas-statue-replacement-v1`  
Base: `be9a2b613f270a6e70559d04f4009da7780b821c`

## Result

This experiment creates a genuinely new contiguous Blender body from fresh
overlapping primitive anatomy. The original high-LOD body remains in the
working scene as hidden reference geometry only. The normal-size original
globe is retained separately.

The candidate preserves the raised-globe composition, source placement,
pedestal relationship, and the expected material identities. It exports
through the #34 evaluated loop-triangle bridge, splices into the tracked
high-LOD WRL, compiles through GetVrml, and loads through a reversible loose
`.geo` override.

The result does not meet the issue's primary visual acceptance bar. The
neutral board is clearly new geometry, but the bounded primitive/remesh method
still reads as a simplified mannequin at this scale rather than a convincing
modern remake-quality Atlas sculpture. The runtime body is also too dark and
partly occluded when using the old texture donor. This is the requested stop
condition; further work should be an intentional art/sculpt pass, not more
incremental smoothing of the old mesh.

## Construction

- Fresh ribcage, pelvis, trapezius, pectorals, neck, head/jaw/brow/nose,
  deltoids, raised arms/hands, thighs, knees, calves, and feet were authored
  as new Blender primitives.
- The overlapping parts were joined and voxel-remeshed into one contiguous
  body, followed by a bounded surface-relax pass.
- The engine candidate uses a 2.20 Blender-unit voxel size to stay within the
  practical GetVrml compile envelope. The denser 1.55-unit candidate compiled
  indefinitely at reduce-instruction generation; a 1.35-unit smoothed pass
  returned `-1` without a `.geo`.
- Old body UVs were projected onto the new surface with nearest-reference
  triangle interpolation as a temporary runtime aid. The new body winding was
  flipped to match the CoH runtime convention, and the new object's transform
  was baked before bridge export.

## Counts and materials

Authored/exported high LOD:

| Part | Vertices | Triangles | Material | UVs |
| --- | ---: | ---: | --- | ---: |
| New body/head | 8,010 | 16,016 | `X_Male_Statue_Atlas_01` | projected donor UV |
| Globe | 559 | 960 | `X_Male_Statue_Atlas_Globe_01` | original UV |
| Total | 8,569 | 16,976 | two required identities | |

Blender file:

`D:\temp\cohsourcedev-blender\issue35\atlas-replacement-v1-final.blend`

Compiled loose geometry SHA-256:

`00109945A51D06345B2D2AA75266A897F90E8861B3A2980B9CDF3574310E8645`

## Evidence

- [Neutral original vs replacement](evidence/issue35-atlas-statue-replacement-v1/Original_vs_Replacement_v1_neutral.png)
- [Stock control capture](evidence/issue35-atlas-statue-replacement-v1/AtlasHero_Statue_01_control-stock.jpg)
- [Replacement v1 capture](evidence/issue35-atlas-statue-replacement-v1/AtlasHero_Statue_01_replacement-v1.jpg)
- [Machine-readable final counts](evidence/issue35-atlas-statue-replacement-v1/replacement-v1-final-stats.json)

The replacement capture passed the existing capture harness with exit code 0.
Runtime telemetry observed the loose geometry at the Atlas target and both
material bindings. LOD1/LOD2 remained sourced from the original WRL.

## Validation

- GetVrml compile: exit code 0; 15.35 seconds; loose `.geo` produced.
- Direct-DB character/map smoke: pass after shard warm-up.
- Replacement Atlas capture: pass; clean Ouroboros exit; both material
  identities observed.
- Stock geometry restored afterward: `packed-stock`; no loose override remains.
- No C/C++ or shared engine code changes were made.
