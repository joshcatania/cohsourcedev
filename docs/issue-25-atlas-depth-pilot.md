# Issue #25 — Atlas depth pilot

This pilot starts from `ea1e9414f839a5bb949aa826da4175d54dd47b5b` on `agent/atlas-depth-pilot`. It evaluates the existing shadow-map and SSAO/ambient systems at runtime; it does not change textures, material equations, tone, bloom, water, regression cameras, regression baselines, or implement a new large shadow/AO system.

The durable visual evidence is the [four-state City Hall matrix](evidence/issue25-atlas-depth-matrix.jpg) and the [four-view current-versus-combined-winner contact sheet](evidence/issue25-atlas-depth-contact-sheet.jpg). The complete machine-readable settings, candidate decisions, performance samples, and capture paths are in the [metrics record](evidence/issue25-atlas-depth-pilot-metrics.json).

## Test design

All captures used the existing `AtlasHero_CityHall_01`, `AtlasHero_East_01`, `AtlasHero_North_01`, and `AtlasHero_West_01` fixed authoring views at 1280x720 with:

```text
-glslPilot 1 -modernMaterials 1 -modernPresentation 0 -modernBloom 0
```

The developer-only `agent/depth-pilot.ps1` helper writes the file-backed graphics settings, runs the existing `agent/capture.ps1` path, records the exact requested settings and capture result, copies the screenshot into the issue evidence directory, and restores every setting in a `finally` block. It pins the existing capture-harness values `shaderdetail=3` and `usewater=2` equally for every state; those are not part of the depth decision.

The current/reference state was the checked-in runtime state observed before the matrix:

| Setting | Current/reference |
| --- | ---: |
| Shadow mode | `1` — stencil |
| Shadow shader | `0` |
| Shadow map size | `0` — 512 when applicable |
| Shadow distance | `-2` — off/current |
| Ambient strength | `3` — high |
| Ambient resolution | `2` — quality |
| Ambient blur | `6` — trilateral |

The matrix then separated the systems into current, shadows-only, AO-only, and combined states. Each candidate was judged from the fixed City Hall view first; the selected combined state was rerun after a discarded warmup and cross-checked in all four views.

## Runtime findings

### Shadow maps

Shadow maps are active and materially improve scene-wide depth. They put the hero on the ground, add readable cast shadows to the paving and planter, and give the stairs, low walls, rails, and nearby trees a useful light direction. The current stencil state is comparatively flat and has little useful cast-shadow coverage in these views.

The best balance was:

```text
shadowmode=4       high shadow-map mode
shadowmapshader=3  high-quality shader
shadowmapsize=1    1024
shadowmapdistance=1 middle
```

The 512/close candidate was visibly more aliased and lost useful coverage. The medium/1024/middle candidate was usable but softer and weaker. The 2048/far candidate extended coverage but made the scene more aggressive/dark without a proportional improvement in the fixed gameplay-distance framing; it was not selected.

The remaining defects are characteristic of the existing path: hard or aliased edges in some areas, imperfect bias/contact behavior, and visible cascade/distance limits. They are quality limits, not evidence that the system is inactive or unusable.

### SSAO / ambient occlusion

AO contributes subtler depth than the shadow map. The useful result is local contact darkening at the hero's feet, planter and wall intersections, and paving seams; AO alone does not establish the scene's light direction or replace cast shadows.

The best AO-only balance was:

```text
ambientstrength=3     high
ambientresolution=3   high quality
ambientblur=5         bilateral depth
```

Gaussian depth was usable, but bilateral depth held contact structure more cleanly. Trilateral at the same strength started to darken crevices more than it clarified them. Ultra strength plus super-high resolution/trilateral was visibly over-processed for the small additional return. The combined winner keeps the high/bilateral-depth AO state over the selected 1024/middle/high-quality shadow map.

### Cross-view winner check

The combined winner passed a clean capture in all four authoring views. The improvement is strongest where the camera sees the hero, paving, planters, and architectural edges together; it remains useful in the longer North view, where the middle shadow distance avoids the excessive/distant darkness of the far candidate. The West close framing shows the grounding/contact benefit without requiring a texture or material change. The East and City Hall views show the clearest scene-wide cast-shadow improvement.

The contact sheet is visual evidence rather than a new pixel baseline: weather, exposure convergence, idle animation, and runtime entity population remain live-state variation documented by the repository guidance.

## Rough performance cost

The helper collected 20 GPU samples and approximately 76–78 CPU samples during each deterministic capture. PresentMon was also attempted, but it emitted no frame records for this legacy OpenGL client; therefore this pilot reports the available repeatable CPU/GPU sample bands rather than claiming an FPS number.

| State | CPU mean / max (% of machine) | GPU mean / max (%) | Capture |
| --- | ---: | ---: | --- |
| Current | 9.29 / 24.51 | 12.55 / 36 | pass, exit 0 |
| Shadows-only winner | 9.60 / 23.93 | 13.65 / 40 | pass, exit 0 |
| AO-only winner | 9.46 / 22.43 | 11.40 / 30 | pass, exit 0 |
| Combined winner | 9.09 / 21.79 | 14.35 / 39 | pass, exit 0 |

These are rough live-process samples, not a benchmark harness. They show no meaningful CPU increase and a modest combined GPU increase of about 1.8 percentage points over the current sample (roughly 14% relative to that sample). The result is compatible with retaining the systems for a bounded modernization pass; it does not justify blindly selecting 2048/far shadows or ultra AO.

## Recommendation

Overall disposition: **B — promising but visibly dated; salvageable with a narrow modernization pass.**

- **Shadow maps:** salvageable, not ready to call “good” as a modern default. The existing buffers, high/1024/middle configuration, and active cast-shadow behavior are valuable; edge quality, bias, cascade behavior, and distance handling need focused follow-up.
- **SSAO:** salvageable and useful at high/high-quality/bilateral-depth. It is not a replacement for lighting and becomes unattractive when pushed to ultra/trilateral.
- **Replacement:** not warranted by this pilot. Both systems produce real scene-wide value at reasonable rough cost, so a large new shadow/AO system would be premature.

The combined winner produces a materially larger scene-wide improvement than another conventional texture cleanup pass would provide in these views: it simultaneously clarifies the hero's grounding, paving/planter contact, architectural edges, and directional cast-shadow relationships. That does not make texture work irrelevant; it makes depth the higher-return next investigation for this Atlas slice.

## Verification and scope guardrails

- `agent/doctor.ps1 -Json` passed with the repository's known non-blocking v142 probe warning and verified v145 fallback.
- Direct-DB `agent/smoke.ps1 -ExerciseCharacter -AccountName Dummy00009 -TimeoutSeconds 180` passed before the pilot.
- All four reference captures and all four combined-winner captures passed with exit code 0; the failed overlapping launches were harness timing races and were rerun serially before evidence selection.
- `Release|x86` rebuilt successfully with the verified v145 fallback after stopping the disposable local shard. The first post-restart formal regression had one invalid East capture caused by the documented warm-up/readiness race; after the shard warmed, the unchanged formal suite passed 4/4 on the rerun (`changedPercent` 2.30%, 0.02%, 0%, and 0.03%), with zero baseline adoption. No baseline adoption is part of this issue.
- The durable deliverables are limited to the reversible capture helper, contact-sheet generator, report, compact metrics, and compressed evidence. No renderer source, textures, material equations, tone/bloom, water, regression cameras, or regression baselines are changed.

The final published commit SHA is recorded in the issue #25 completion comment and in the task handoff after push.
