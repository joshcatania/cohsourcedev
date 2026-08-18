# Issue #29 — Atlas statue hero remaster v1

Status: complete on `agent/atlas-statue-hero-remaster-v1`. The statue remaster clears the normal-size visual gate and is hardened as a stock-by-default, reversible manifest; no loose overrides are left installed.

Base: accepted issue #26 at `f66875cfab1c860fb190cf88cbe31f7da3e7da70f`.

## Scope and preflight

The repository guide, issues #20, #21, #23, #26, and #28, their local reports/reviews, and both Sol pre-start comments on #29 were read before implementation. The work stayed inside the existing `TEXTUREPILOT` telemetry, capture, GetTex, loose-override, build, ServerMonitor, and TestClient paths. No broad asset archaeology or new mesh pipeline was introduced.

## Deterministic capture

Added exactly one table-driven capture identity: `AtlasHero_Statue_01`. Existing capture identities and regression baselines were not changed.

- Map: Atlas Park static map 1.
- Camera source: the additive shot row, `100.00 120.00 -650.00 0.2000 0.0000 0.0000`, `camdist 30`.
- Output: 1280×720 JPEG.
- Stabilization: `timeset 16`, `timescale 0`, one discarded warm-up capture after login/map readiness, then the final capture.
- Stock Ultra: `-modernMaterials 0 -modernLighting 0 -modernPresentation 0 -modernBloom 0`.
- Current Remaster (#26): `-modernMaterials 1 -modernLighting 1 -modernPresentation 0 -modernBloom 0`.
- Current Remaster + Statue Remaster: the #26 flags plus the seven manifest-selected loose overrides, installed only for the candidate run.

The final three-state contact sheets are [full-frame](evidence/issue29-atlas-statue-hero-remaster-v1/contact-sheet.jpg) and [statue-focus detail](evidence/issue29-atlas-statue-hero-remaster-v1/statue-detail-contact-sheet.jpg).

## Runtime identification

The existing `TEXTUREPILOT` bind telemetry identified the exact Atlas statue draw without scanning the asset library broadly:

```text
model=_H_M_Statue_Atlas_Giant
source=object_library/City_Zones/Elements/Hero_Statues/Male_Statue_Atlas/Male_Statue_Atlas.geo
verts=2955 tris=4608 texCount=2 radius=294.210
lodCount=3
binds=X_Male_Statue_Atlas_Globe_01, X_Male_Statue_Atlas_01
```

The minimal geometry extension records the selected model’s vertex/triangle counts, bounds, radius, world midpoint, source model, and reported LOD count once per distinct model. A separate authoring probe also saw `_H_M_Statue_Atlas_Giant_LOD_01` at 1694 verts / 2442 tris with `lodCount=2`. The runtime exposes `lodIndex=-1` and zero near/far/error thresholds for these draws, so the evidence records the reported counts without inventing unresolved LOD thresholds. The curated excerpt is [runtime-telemetry.txt](evidence/issue29-atlas-statue-hero-remaster-v1/runtime-telemetry.txt).

The exact material set is seven runtime-bound files:

| Material layer | Stock | Hero target | Treatment |
| --- | ---: | ---: | --- |
| `Statue_Globe_02_D` | 512×256 | 1024×512 | normal-guided albedo restoration, contrast/gain tuned for the globe |
| `Statue_Globe_02_N` | 256×128 | 512×256 | vector-aware normal restoration; gloss alpha nearest-preserved |
| `Statue_Globe_02_MK` | 256×128 | 512×256 | authored mask resize with nearest semantics |
| `Statue_Globe_02_M` | 512×512 | 1024×1024 | authored detail restoration with controlled unsharp pass |
| `Statue_Globe_01_D` | 256×256 | 1024×1024 | body-normal-guided albedo restoration |
| `Male_Statue_Atlas_AO` | 512×512 | 1024×1024 | authored detail restoration with controlled unsharp pass |
| `Male_Statue_Atlas_N` | 1024×1024 | 1024×1024 | vector-aware normal restoration; gloss alpha nearest-preserved |

The full source/target dimensions, flags, source hashes, and expected generated hashes are in [the manifest](../agent/atlas-statue-hero-remaster-v1.json).

## Bottleneck diagnosis

The dominant ceiling is a combination of low-resolution authored base/surface information in the globe material and the existing lighting/material presentation. Geometry is a secondary future ceiling, not the reason to create a new mesh pipeline for this issue.

- Base texture: confirmed bottleneck for the globe. The diffuse is only 512×256 and the body diffuse is 256×256; plain enlargement alone was visibly too soft.
- Normal/surface information: present, but the globe normal is only 256×128. The body normal is already 1024×1024. Vector-aware restoration and normal-derived albedo cues make the globe map/relief read at screenshot size.
- Gloss/spec/material data: the runtime binds an authored normal alpha/gloss channel and separate mask/multiply layers; telemetry reports the existing material bindings and gloss state. No new shader/material branch was needed. The #26 lighting path remains the controlled presentation baseline.
- UVs: no UV defect was found. The remaster reuses the exact runtime material binds and source UV layout; no UVs or mesh vertices are changed.
- Geometry/LOD: the selected draw is 2955 verts / 4608 tris with a reported three-entry LOD set, and a secondary observed LOD is 1694 verts / 2442 tris. The added telemetry cannot resolve the engine’s selected LOD index/threshold fields in this draw. No already-working end-to-end loose mesh/geo override path was proven quickly, so no geometry pipeline was built.

## Remaster and hardening gate

The candidate uses the existing reversible `texture-pilot.ps1` Generate/Install/Restore workflow and GetTex package path. It is capped at 1024 pixels, preserves source alpha/gloss semantics, and is not a generic 2× upscale: the seven entries use semantic transformations for albedo, vector normals, mask handling, and controlled detail restoration. The manifest is `stock_by_default: true`.

The initial restrained candidate failed the gate because it was effectively invisible against #26. Its output was discarded. The final candidate increased only the source-guided restoration strength and authored detail treatment; no geometry, UV, camera, or shader change was used to manufacture the comparison.

At normal 1280×720 size, the final candidate is immediately and clearly better than #26 in the globe: map/relief structure is legible rather than a dark soft mass. The [focus contact sheet](evidence/issue29-atlas-statue-hero-remaster-v1/statue-detail-contact-sheet.jpg) makes the same result easy to inspect. The comparison measurements are [recorded here](evidence/issue29-atlas-statue-hero-remaster-v1/comparison-metrics.json):

- Stock Ultra → #26: 44.631% changed pixels, mean delta 11.7808, max delta 140 at the 320-pixel comparison sampler. This is the expected large lighting/presentation change from #26.
- #26 → #26 + Statue Remaster: 0.5085% changed pixels, mean delta 1.5001, max delta 34 at the same sampler.
- Statue focus region `(470, 0)-(970,420)`: 1.9195% changed pixels, mean delta 2.3674, max delta 128. The region metric is supplementary; the hardening decision is the visible relief/map improvement in the normal-size capture.

The candidate overrides were installed for the final capture, then `Restore` removed all seven selected loose files and verified both packed pigg hashes. The working tree therefore retains the manifest and reproducible authoring code, not a machine-specific loose override state.

## Evidence and validation

The evidence directory contains the three final JPEGs, capture JSON, startup traces, the contact sheets, comparison metrics, and curated runtime telemetry:

`docs/evidence/issue29-atlas-statue-hero-remaster-v1/`

Final validation completed:

- `agent/doctor.ps1 -Json`: ready; the known non-blocking v142 probe warning falls back to the verified v145 toolset.
- `agent/build-client.ps1 -Configuration Release -Platform x86 -Json`: passed with the verified v145 fallback.
- `agent/smoke.ps1 -ExerciseCharacter -AccountName Dummy00009 -TimeoutSeconds 180 -Json`: passed direct-DB character/map smoke.
- `agent/texture-pilot.ps1 -Action Generate`: passed 1024-pixel and expected-output-hash validation.
- `agent/texture-pilot.ps1 -Action Restore`: passed; seven selected loose paths absent and packed sources match.
- Final stock, #26, and statue-remaster capture runs exited cleanly with exit code 0 after warm-up.
- Historical capture definitions, baselines, and unrelated work were not modified.

Full raw runtime/build/capture logs remain under the existing ignored `agent/logs/` paths; the committed evidence keeps the focused artifacts and avoids duplicating multi-megabyte stdout dumps.
