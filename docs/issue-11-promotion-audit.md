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
| Modulate, multiply, color-blend dual, add-glow, alpha-detail | `BMB_DEFAULT` | dualtex | Formally verified where covered; add-glow is runtime-observed in the audit but still lacks a dedicated visual baseline. |
| Bump color-blend dual | `BMB_DEFAULT`, `BMB_HIGH_QUALITY` | bump-dual and skinned-bump, including HQ | Formally verified in the preflight suite. Other vertex families stay on synchronized ARB fallback. |
| Bump multiply | `BMB_DEFAULT` | model-space bump / RGBS model-space variants | Formally verified; unrelated pairings remain ARB. |
| Water | default, shadow, planar, and combined shadow+planar | bump-dual-multi LQ/HQ | Formally verified, including the high-settings Founders canal run. Unsupported water/vertex pairings are intentional ARB declines. |
| Multi9 | default, HQ, single, single-HQ, building | static bump-dual-multi LQ/HQ | Static-map coverage is verified. RGBS/baked-lighting, skinned multitex, and other reflection/shadow/cubemap combinations remain intentional ARB coverage. |
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

Observed: 124 distinct fallback pairs across the Atlas day suite, Talos,
Founders high settings, and Atlas night. All 124 were:

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
