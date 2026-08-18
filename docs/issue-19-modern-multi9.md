# Issue #19 — modern Multi9 world materials

## Scope

This change starts from `7c92e98f00075419bf17c1a4e122891fd908c5b4` on
`agent/glsl-modern-world-materials` and extends the existing opt-in
`modernMaterials` switch to the five existing GLSL Multi9 fragment rows:

- Full LQ and Full HQ
- Single LQ and Single HQ
- Building

No new shader pairing, sampler, material texture, water, shadow,
post-processing, Bump Multiply, or fallback support was added. The existing
Multi9 material constants remain the source of the response: normal RGB,
gloss alpha/constants, authored specular RGB/exponent, ambient/diffuse, and
the existing tint/mask/add-glow/selector/scroll inputs.

## Implementation

`Game/src/render/thread/rt_glslpilot.c` keeps the pre-existing Multi9 lighting
equations as the base path. The new branch is selected only when the existing
`g_ModernMaterialParamsFP.x` mirror is `1`:

- derives a bounded roughness/lobe width from the authored exponent;
- uses the existing tangent-space view vector for a normalized half-vector;
- applies a view-aware Fresnel/specular response using the authored specular
  color and gloss product;
- leaves authored diffuse lighting intact after visual iteration so the
  improvement reads as material response rather than global darkening;
- preserves the existing material topology: Full lights materials 1 and 2
  independently, Single lights material 1, and Building lights material 1
  before its existing multiply-2 derivation.

With `modernMaterials 0`, the old Multi9 helper equations and old half-vector
expression remain the active output path. The #17 Bump ColorBlendDual source
and behavior were not changed by this issue.

The existing HQ rows are wired to the same opt-in uniform, but the existing
static HQ pairing remains unverified as scoped by issue #5; no HQ pairing was
introduced or broadened here.

## Visual evidence

The confirmed scene is `FoundersCanal_01` on map 10, which exercises the
verified static `bump_dual_multi` LQ rows. The final A/B used the repository's
temporary `capture_override.txt` camera pin (`4494.28 0 992.15 0.25 0.7854 0`)
and was removed after capture. Both runs froze the world at time 16 and
reported `waterFeature=1 multiFeature=1`.

The final clean captures are:

- `agent/logs/capture-FoundersCanal_01-20260817-205611.json` —
  `-glslPilot 1 -modernMaterials 0`
- `agent/logs/capture-FoundersCanal_01-20260817-205641.json` —
  `-glslPilot 1 -modernMaterials 1`

The modern-on log records compilation and activation of all three verified
LQ rows: `BLENDMODE_MULTI`, `BLENDMODE_MULTI single`, and
`BLENDMODE_MULTI building`, each using the `bump_dual_multi` vertex variant.

The visible target is the foreground truck's static side panel and wheel.
The modern-on crop is visibly brighter/less purple, with stronger panel-seam
relief and a clearer wheel-rim response. The moving truck door and player idle
phase are excluded from the stable crop; they are scene animation, not
material evidence. The reviewer contact sheet is committed at
`docs/evidence/issue-19-modern-multi9-contact-sheet.jpg`.

For the stable panel/wheel crop, modern-on versus modern-off measured:

| Region | Mean RGB delta | Pixels with channel delta > 5 |
| --- | ---: | ---: |
| Side panel | 8.9808 | 99.00% |
| Wheel | 12.5292 | 94.30% |

These are evidence metrics for the pinned A/B, not the regression-harness
thresholds.

## Validation

- `agent/doctor.ps1` — READY; the documented v142 probe warning remains, and
  the tested v145 fallback is used.
- `agent/build.ps1 -Configuration Release -Platform x86` — PASS after the
  final shader edit; log:
  `agent/logs/build-Release-x86-20260817-210200.log`.
- `agent/smoke.ps1 -ExerciseCharacter -AccountName Dummy00009` — PASS after
  the final build; character creation and MapServer entry verified in
  148.9 seconds on a freshly restarted shard; log:
  `agent/logs/smoke-directdb-20260817-210425.json`.
- `agent/capture-regression.ps1` with the default hybrid renderer and
  `-modernMaterials 0` — 4/4 PASS, 0 regressions; summary:
  `agent/logs/regression-20260817-204710.json`. Changed-percent results were
  0.0000%, 0.0388%, 1.5343%, and 2.2228%; mean-delta advisories are the
  documented exposure/weather noise policy.
- Explicit legacy control
  `-glslPilot 0 -modernMaterials 1` — PASS and clean exit; log:
  `agent/logs/capture-FoundersCanal_01-20260817-205857.json`.
- #17 character guard: `AtlasPlaza_Closeup_01` with
  `-glslPilot 1 -modernMaterials 0` — PASS and clean exit; the log shows the
  existing `BLENDMODE_BUMPMAP_COLORBLEND_DUAL`/HQ skin and bump variants
  still active:
  `agent/logs/capture-AtlasPlaza_Closeup_01-20260817-204623.json`.
