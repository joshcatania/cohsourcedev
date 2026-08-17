# Issue #11 GLSL pilot promotion-readiness audit

Audit date: 2026-08-17
Pre-audit tip: `4446c8533` (`Port combined shadow-planar water to GLSL pilot`)

## Scope and decision

This is a promotion-readiness audit for the native GLSL pilot. No shader
material, permutation, default-on flag, fallback threshold, or legacy ARB
path was changed. The only code change adds pilot-gated symbolic telemetry to
the existing fallback hooks.

Recommendation: **READY FOR HYBRID DEFAULT-ON** (issue option 1).

The bounded run produced no bucket-A fallback. The observed fallbacks either
used a supported pilot fragment with an intentional/non-canonical vertex
pairing, or were state/reset bookkeeping. The existing ARB path remained the
fallback and every requested capture exited cleanly. The unported families
listed below remain explicit follow-up inventory, not promotion blockers for
the hybrid mode.

## Symbolic support matrix

| Family | Pilot coverage | Vertex pairing | Status / intentional boundary |
| --- | --- | --- | --- |
| Modulate, multiply, color-blend dual, add-glow, alpha-detail | `BMB_DEFAULT` | dualtex | Formally verified where covered. AddGlow has a dedicated Talos parity baseline: two same-control runs passed at 0.0000% and 2.3994% changed pixels, with 487/491 activations and 0 declines; see [issue #8 evidence](https://github.com/joshcatania/cohsourcedev/issues/8#issuecomment-5315268516). |
| Bump color-blend dual | `BMB_DEFAULT`, `BMB_HIGH_QUALITY` | bump-dual and skinned-bump, including HQ | Formally verified in the preflight suite. Other vertex families stay on synchronized ARB fallback. |
| Bump multiply | `BMB_DEFAULT` | model-space bump / RGBS model-space variants | Formally verified; unrelated pairings remain ARB. |
| Water | default, shadow, planar, and combined shadow+planar | `kPilotBumpMultiKindMask` only: static `bump_dual_multi` LQ | Formally verified, including the high-settings Founders canal run. The water rows do not use `kPilotBumpMultiHQKindMask`; unsupported water/vertex pairings are intentional ARB declines. |
| Multi9 | default, single, building; Full HQ and Single HQ targets are implemented but unverified in the static pairing | static `bump_dual_multi` LQ for verified rows; static HQ pairing is `kPilotBumpMultiHQKindMask` | Founders static-map coverage verified Full LQ, Single LQ, and Building. Full HQ and Single HQ reached fragments 121/137 only through skinned-multitex vertex 255 and declined to ARB; the intended static HQ vertex 253 was not reached. See [issue #5 evidence](https://github.com/joshcatania/cohsourcedev/issues/5#issuecomment-5311477268). RGBS/baked-lighting, skinned multitex, and other reflection/shadow/cubemap combinations remain intentional ARB coverage. |
| Effects/post-processing | registered effect IDs, fixed-function pbuffer pairing, sprite dualtex final composite | fixed-function and sprite dualtex | The verified effects chain is ported. Sunflare adaptation and performance-test special effects remain intentionally unported. |
| Sunflare material family | no native GLSL target | legacy path | Not observed in the bounded representative suite; no promotion claim is made for it. Targeted follow-up is only needed if normal-gameplay telemetry shows it materially. |

“Formally verified” means a prior harness-verified capture or the current
bounded run exercised the canonical pairing. “Runtime incomplete” means the
family is present in the pilot but the exact permutation/pairing was not
exercised by this bounded suite. “Not ported” means it remains intentionally
on ARB; it is not being implemented under this issue.

## Bounded fallback inventory

The post-build runs used pilot mode and the existing ARB fallback. Telemetry
deduplicates fragment/vertex pairs and reports symbolic blend family,
`BMB_*` permutation, vertex program/kind, and classification.

Observed: 124 logged fallback-pair events, representing 34 unique symbolic
pair identities across the Atlas day suite, Talos, Founders high settings,
and Atlas night. The telemetry deduplicates within each client process. The
34 identities below are the complete `fallback pair` stream; every row was
classified as `pilot-target-declined-on-vertex-pairing`, and the legacy
fragment request was synchronized before draw. The separate fragment-bind
coverage stream also recorded six reset/disable bookkeeping identities,
including `vertex=-1`; those are Bucket C state records, not rendered
material pair identities, and are therefore excluded from the 34-row count.

| Fragment family / `BMB_*` | Vertex identity / kind / draw mode | Classification | Scenes observed | Bucket |
| --- | --- | --- | --- | --- |
| AddGlow / `BMB_DEFAULT` | 228 / `skin_bump` | pilot-target-declined-on-vertex-pairing | Atlas City Hall, Founders canal | B |
| AddGlow / `BMB_DEFAULT` | 247 / `skin_bump HQ` | pilot-target-declined-on-vertex-pairing | Atlas City Hall, Atlas night, Founders canal | B |
| Bump ColorBlendDual / `BMB_DEFAULT` | 223 / dualtex lit 5 | pilot-target-declined-on-vertex-pairing | Founders canal | B |
| Bump ColorBlendDual / `BMB_DEFAULT` | 227 / dualtex lit 5 | pilot-target-declined-on-vertex-pairing | Atlas City Hall/East/North/West, Atlas night | B |
| Bump ColorBlendDual / `BMB_DEFAULT` | 229 / `DRAWMODE_HW_SKINNED` LQ | pilot-target-declined-on-vertex-pairing | Atlas City Hall | B |
| Bump ColorBlendDual / `BMB_DEFAULT` | 236 / `DRAWMODE_BUMPMAP_SKINNED_MULTITEX` LQ | pilot-target-declined-on-vertex-pairing | Atlas City Hall/East/North/West, Atlas night, Founders canal | B |
| Bump ColorBlendDual / `BMB_DEFAULT` | 247 / `skin_bump HQ` | pilot-target-declined-on-vertex-pairing | Atlas City Hall/East/North/West, Atlas night, Founders canal | B |
| Bump ColorBlendDual / `BMB_HIGH_QUALITY` | 223 / dualtex lit 5 | pilot-target-declined-on-vertex-pairing | Atlas City Hall, Atlas night, Talos | B |
| Bump ColorBlendDual / `BMB_HIGH_QUALITY` | 224 / dualtex lit 5 | pilot-target-declined-on-vertex-pairing | Founders canal, Talos | B |
| Bump ColorBlendDual / `BMB_HIGH_QUALITY` | 227 / dualtex lit 5 | pilot-target-declined-on-vertex-pairing | Atlas City Hall/East/North/West, Atlas night, Founders canal, Talos | B |
| Bump ColorBlendDual / `BMB_HIGH_QUALITY` | 228 / `skin_bump` | pilot-target-declined-on-vertex-pairing | Atlas City Hall, Atlas night, Founders canal, Talos | B |
| Bump ColorBlendDual / `BMB_HIGH_QUALITY` | 229 / `DRAWMODE_HW_SKINNED` LQ | pilot-target-declined-on-vertex-pairing | Atlas City Hall | B |
| Bump ColorBlendDual / `BMB_HIGH_QUALITY` | 236 / `DRAWMODE_BUMPMAP_SKINNED_MULTITEX` LQ | pilot-target-declined-on-vertex-pairing | Atlas City Hall/East/North/West, Atlas night | B |
| Bump ColorBlendDual / `BMB_HIGH_QUALITY` | 255 / `DRAWMODE_BUMPMAP_SKINNED_MULTITEX` HQ | pilot-target-declined-on-vertex-pairing | Atlas City Hall/East/North/West, Atlas night, Founders canal, Talos | B |
| Bump Multiply / `BMB_DEFAULT` | 224 / dualtex lit 5 | pilot-target-declined-on-vertex-pairing | Talos | B |
| Bump Multiply / `BMB_DEFAULT` | 228 / `skin_bump` | pilot-target-declined-on-vertex-pairing | Founders canal, Talos | B |
| Bump Multiply / `BMB_DEFAULT` | 247 / `skin_bump HQ` | pilot-target-declined-on-vertex-pairing | Founders canal, Talos | B |
| ColorBlendDual / `BMB_DEFAULT` | 228 / `skin_bump` | pilot-target-declined-on-vertex-pairing | Atlas night, Founders canal | B |
| ColorBlendDual / `BMB_DEFAULT` | 247 / `skin_bump HQ` | pilot-target-declined-on-vertex-pairing | Founders canal, Talos | B |
| Modulate / `BMB_DEFAULT` | 228 / `skin_bump` | pilot-target-declined-on-vertex-pairing | Atlas East/North/West, Atlas night, Founders canal | B |
| Modulate / `BMB_DEFAULT` | 247 / `skin_bump HQ` | pilot-target-declined-on-vertex-pairing | Atlas City Hall/North, Atlas night, Founders canal | B |
| Multi9 / `BMB_BUILDING` | 224 / dualtex lit 5 | pilot-target-declined-on-vertex-pairing | Atlas City Hall, Atlas night, Founders canal | B |
| Multi9 / `BMB_DEFAULT` | 224 / dualtex lit 5 | pilot-target-declined-on-vertex-pairing | Atlas East, Atlas night | B |
| Multi9 / `BMB_DEFAULT` | 228 / `skin_bump` | pilot-target-declined-on-vertex-pairing | Atlas City Hall, Atlas night | B |
| Multi9 / `BMB_DEFAULT` | 229 / `DRAWMODE_HW_SKINNED` LQ | pilot-target-declined-on-vertex-pairing | Atlas night | B |
| Multi9 / `BMB_DEFAULT` | 236 / `DRAWMODE_BUMPMAP_SKINNED_MULTITEX` LQ | pilot-target-declined-on-vertex-pairing | Founders canal | B |
| Multi9 / `BMB_DEFAULT` | 255 / `DRAWMODE_BUMPMAP_SKINNED_MULTITEX` HQ | pilot-target-declined-on-vertex-pairing | Atlas City Hall/East/North, Atlas night, Founders canal | B |
| Multi9 / `BMB_SINGLE_MATERIAL` | 224 / dualtex lit 5 | pilot-target-declined-on-vertex-pairing | Atlas East/North, Atlas night, Founders canal | B |
| Multi9 / `BMB_SINGLE_MATERIAL` | 228 / `skin_bump` | pilot-target-declined-on-vertex-pairing | Founders canal | B |
| Multiply / `BMB_DEFAULT` | 228 / `skin_bump` | pilot-target-declined-on-vertex-pairing | Atlas City Hall/East/North/West, Atlas night, Founders canal, Talos | B |
| Multiply / `BMB_DEFAULT` | 229 / `DRAWMODE_HW_SKINNED` LQ | pilot-target-declined-on-vertex-pairing | Atlas City Hall | B |
| Multiply / `BMB_DEFAULT` | 247 / `skin_bump HQ` | pilot-target-declined-on-vertex-pairing | Atlas City Hall/East/North/West, Atlas night, Founders canal, Talos | B |
| Water / `BMB_DEFAULT` | 224 / dualtex lit 5 | pilot-target-declined-on-vertex-pairing | Atlas City Hall, Atlas night, Talos | B |
| Water / `BMB_PLANAR_REFLECTION` | 224 / dualtex lit 5 | pilot-target-declined-on-vertex-pairing | Founders canal | B |

No genuinely unported rendered fragment was observed in the 34-row
inventory; Bucket C remains limited to the uncharacterized bookkeeping and
intentionally out-of-scope families described below.

### Bucket B — expected or bounded fallback

- Supported simple and bump fragments requested with unrelated vertex
  pairings, including `skin_bump`, HQ skin-bump, dualtex lit modes, model-space
  bump, and skinned-multitex draw modes.
- Multi9 default, HQ, single, and building variants requested with non-static
  vertex families, including `DRAWMODE_BUMPMAP_SKINNED_MULTITEX`.
- Water planar-reflection material requested with a non-canonical dualtex or
  skinned vertex pairing in the high-settings scene.
- These were reported as
  `classification=pilot-target-declined-on-vertex-pairing`; the legacy
  fragment request was synchronized before draw.

These are common enough to track, but they are not missing shader math: the
pilot is deliberately gated to known-safe fragment/vertex pairings and the
existing ARB path remains deterministic for the other pairings.

### Bucket C — non-blocking / not characterized by this suite

- Vertex-program reset/disable notifications, including the sentinel
  `vertex=-1`, are state bookkeeping rather than a rendered material.
- Sunflare adaptation and performance-test effect IDs were not observed in
  the bounded representative captures.
- Legacy R200/NV/TEX_ENV_COMBINE branches are dead-path candidates on the
  modern target and were not exercised.

### Bucket A — promotion blockers

None observed. No common, visually important material was found that both
lacked a native pilot target and failed to retain a clean ARB fallback.

## Tests and evidence

- `agent/doctor.ps1 -Json`: ready; the known v142 probe warning was handled by
  the tested v145 fallback.
- `agent/build.ps1 -Configuration Release -Platform x86`: **PASS**, log
  `agent/logs/build-Release-x86-20260817-080740.log`.
- Direct-DB smoke with character creation: **PASS**, `Dummy00009`, clean
  TestClient exit, character creation and MapServer connection confirmed in
  `agent/logs/smoke-directdb-20260817-081504.json`.
- Atlas day regression: four requested shots. Three passed on the first suite
  run; City Hall’s first comparison was a transient 50.21% weather/exposure
  outlier. The isolated rerun passed at 3.40% changed pixels; no comparison
  thresholds changed. East/North/West passed at 2.07%/1.98%/2.13% changed
  pixels. All captures exited cleanly.
- Talos arrival: **PASS**, `agent/logs/capture-TalosArrive_01-20260817-081919.json`.
- Founders Falls canal with `-useWater 4 -reflectionEnable 1
  -separate_reflection_viewport 1 -shadowMode 2 -shadowmapshader 1
  -shadowmapdistance 0 -shadowmapsize 0`: **PASS**, water state 4 and
  multitex feature active, `agent/logs/capture-FoundersCanal_01-20260817-081958.json`.
- Atlas night: **PASS**, `agent/logs/capture-AtlasPlaza_NightEast_01-20260817-082024.json`.

## Diagnostic change and follow-ups

The telemetry reuses the live fragment-variant registry and effect-ID table;
it does not hard-code material IDs. It is active only when `-glslPilot 1` is
enabled, bounded to a fixed number of distinct pairs, and distinguishes a
pilot-target vertex decline from an actually unported fragment variant.

Recommended follow-up order:

1. Add targeted Multi9 vertex-family coverage only if hybrid telemetry shows
   the intentional skinned/RGBS/reflection fallback is a migration priority.
2. Capture and classify Sunflare in representative normal gameplay before
   considering a native port.
3. Exercise the unobserved special effects only when their production use is
   established.
4. Delete legacy dead paths later as bounded renderer hygiene, independently
   of this promotion decision.

The shard, registry settings, and capture artifacts were restored after the
audit. The final diagnostic commit and pushed branch are recorded in the
issue comment.
