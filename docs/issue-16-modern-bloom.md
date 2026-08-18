# Issue #16 — Modern Bloom v1 evidence

Implementation commit: `4f03000a308a88cebfa41c996bf23cb3fc8421cb`

The branch remains `agent/glsl-modern-presentation`, based on the accepted
#13 tip `54bcd55af8aff888b2d9437c49b26e3f9b1d2376`.

## Configuration and scope

`game_state.modernBloom` is a command-backed, default-off flag. It is carried
through the existing `g_Effects_PresentationFP` mirror:

```text
.x = modernPresentation
.y = modernBloom
```

The four native GLSL bloom-capable final shaders use the new branch only when
`.y` is enabled:

- `FX_TONEMAP2`
- `FX_TONEMAP2_DESATURATE`
- `FX_DOF_BLOOM_FINAL`
- `FX_DOF_BLOOM_FINAL_DESATURATE`

Therefore `modernPresentation 0 modernBloom 0` retains the promoted reference
path, `modernPresentation 1 modernBloom 0` retains the accepted #13 image,
and `modernPresentation 1 modernBloom 1` is the Modern Bloom v1 experiment.
`-glslPilot 0` remains the legacy ARB/Cg control because the vector is only
mirrored to native GLSL programs.

The existing `brightPass()` call remains disabled. No blur kernel, pbuffer,
sampler, texture, render pass, adaptation, exposure, DOF, material, world, or
legacy shader path changed.

## Final bloom math

The branch thresholds the original, already-blurred full-scene RGB buffer
(`bloomSource`) using the renderer's existing luminance weights, then applies
the scalar gate to the tone-mapped blurred RGB. The final constants are:

```text
LUM             = (0.35, 0.45, 0.20)
threshold       = 0.62
soft-knee       = 0.22
energy scale    = 2.50
```

Equivalent shader math:

```text
sourceLum   = dot(max(bloomSource, 0), LUM)
soft        = clamp(sourceLum - 0.62 + 0.22, 0, 2 * 0.22)
bloomEnergy = (soft * soft) / (4 * 0.22)
bloomEnergy = max(bloomEnergy, max(sourceLum - 0.62, 0))
bloomGate   = clamp((bloomEnergy / max(sourceLum, 0.001)) * 2.50, 0, 1)
bloomWeight = clamp(ExpectedLum.z, 0, 1)
glow        = blurredTonemapped.rgb * bloomGate * bloomWeight
sample.rgb += glow * max(0, 1 - sample.rgb)
```

The threshold is near the old bloom onset around the upper-midrange, while the
`0.22` knee gives a gradual onset below it. The scalar gate preserves the
blurred highlight's RGB hue/chroma instead of thresholding channels separately.
The last line is an energy-limited screen-like composite: it adds glow where
the destination has headroom and naturally weakens near white. `ExpectedLum.z`
remains the scene-authored bloom weight.

The first runtime tuning pass used a narrower knee and lower energy scale; it
was visibly too quiet in the night skyline. The final values above made the
brightest skyline/cyan highlights more intentional while the day control did
not acquire a frame-wide glow.

## Reviewer-accessible evidence

The compressed contact sheet is committed at
[`docs/evidence/issue-16-modern-bloom-contact-sheet.jpg`](evidence/issue-16-modern-bloom-contact-sheet.jpg).
It compares `modernPresentation 1 modernBloom 0` on the left with
`modernPresentation 1 modernBloom 1` on the right. Full-resolution captures
remain local under `agent/captures/issue16/`.

| Scene | Presentation-only control | Modern Bloom v1 | Assessment |
|---|---|---|---|
| Atlas night / skyline | `agent/captures/issue16/presentation-on-modern-bloom-off/AtlasPlaza_NightEast_01_final.jpg` | `agent/captures/issue16/presentation-on-modern-bloom-on/AtlasPlaza_NightEast_01_final.jpg` | Bright cyan skyline bands receive a softer halo; the unlit foreground remains the #13 darker-night caveat. |
| Founders / emissive-cyan surfaces | `agent/captures/issue16/presentation-on-modern-bloom-off/FoundersCanal_01_final.jpg` | `agent/captures/issue16/presentation-on-modern-bloom-on/FoundersCanal_01_final.jpg` | Cyan highlights remain colored and the truck does not become a broad glowing fog; door/camera phase differs between runs. |
| Atlas day / pale-sky control | `agent/captures/issue16/presentation-on-modern-bloom-off/AtlasPlaza_East_01_final.jpg` | `agent/captures/issue16/presentation-on-modern-bloom-on/AtlasPlaza_East_01_final.jpg` | No obvious global daytime haze; exposure and player phase vary between process launches. |
| Talos arrival | `agent/captures/issue16/presentation-on-modern-bloom-off/TalosArrive_01_final.jpg` | `agent/captures/issue16/presentation-on-modern-bloom-on/TalosArrive_01_final.jpg` | Current arrival view is a side-lane probe rather than a strong AddGlow/building view, so it is not used as a bloom verdict. |
| Character close-up | `agent/captures/issue16/presentation-on-modern-bloom-off/AtlasPlaza_Closeup_01_final.jpg` | `agent/captures/issue16/presentation-on-modern-bloom-on/AtlasPlaza_Closeup_01_final.jpg` | Costume hue/detail remain intact; idle animation makes this unsuitable for pixel-level judgment. |

Informational comparison metrics below use the repository comparator at 320 px
with `pixelTolerance=12`. They are not parity failures; separate client
launches freeze eye adaptation at different convergence points, and some views
also move the player or door animation.

| Pair | changedPercent | meanDelta |
|---|---:|---:|
| Atlas night | 7.2617% | 4.5002 |
| Founders | 58.7253% | 35.1253 |
| Atlas day | 43.2609% | 29.2338 |
| Talos arrival | 84.5039% | 35.0824 |
| Character close-up | 16.6773% | 7.3778 |

The useful visual result is the localized night/emissive halo behavior, not
the large whole-image deltas in the unstable pairs. No black, full-white, or
NaN-style frame was observed. No hue rotation or obvious saturation break was
seen. The remaining downside is Atlas-night unlit foreground readability,
which is inherited from #13 and is a reason to keep both modern flags
opt-in/default-off.

## Validation

- Doctor: ready; the documented v142 MSBuild probe warning remains non-blocking
  because the v145 fallback succeeds.
- Release/x86 build: PASS, `agent/logs/build-Release-x86-20260817-133906.log`.
- Direct-DB character/map smoke: PASS in 2.2 seconds,
  `agent/logs/smoke-directdb-20260817-134431.json`.
- Explicit `modernPresentation 0 modernBloom 0` regression: 4/4 PASS using
  the existing 6% changed-pixel policy; CityHall 0.6674%, East 0.0230%, North
  1.5007%, West 0.0141%. Mean-delta advisories were report-only exposure
  noise. Summary: `agent/logs/regression-20260817-135241.json`.
- Legacy control: `-glslPilot 0 -modernPresentation 1 -modernBloom 1`
  captured and exited 0 with no GLSL activation/compile/link/uniform failure;
  result: `agent/logs/capture-AtlasPlaza_NightEast_01-20260817-135200.json`.
- Final modern captures: all selected ON/control captures exited 0. The
  final bloom branch adds no texture fetches or allocations and costs roughly
  12–18 scalar/vector ALU operations per final fragment, depending on driver
  lowering.

The final pushed branch SHA is reported with the evidence comment on issue
#16.
