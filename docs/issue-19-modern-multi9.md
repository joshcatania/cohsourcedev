# Issue #19 — modern Multi9 world materials

## Scope

This follow-up keeps the original issue #19 boundary: the opt-in
`modernMaterials` switch is extended only to the five existing native-GLSL
Multi9 fragment rows:

- Full LQ and Full HQ
- Single LQ and Single HQ
- Building

No shader pairing, sampler, material texture, water, shadow, post-processing,
Bump Multiply, fallback, mask, tint, selector, scroll, scale, or add-glow
behavior was changed. The existing Multi9 data remains authoritative:
normal RGB, gloss alpha/constants, authored specular RGB/exponent, and the
existing ambient/diffuse inputs.

## Final material response

`Game/src/render/thread/rt_glslpilot.c` preserves the original helper equations
before the existing `modernMaterials` branch. With `modernMaterials 0`, the
legacy Multi9 output and legacy half-vector remain unchanged.

For `modernMaterials 1`, each sub-material uses:

```text
glossStrength = clamp(normalGloss.a * max(glossConstant, 0), 0, 1)
matteGuard    = smoothstep(0.08, 0.20, glossStrength)
glossResponse = sqrt(glossStrength) * matteGuard

roughness    = clamp(sqrt(2 / (legacyExponent + 2)), 0.40, 0.90)
lobeExponent = clamp(2 / roughness^2 - 2, 1, 128)
NoL          = saturate(dot(N, L))
NoV          = saturate(dot(N, V))
NoH          = saturate(dot(N, normalize(V + L)))
D            = pow(NoH, lobeExponent) * (lobeExponent + 2) / pi

F0 = clamp(authoredSpecularRGB
           * (0.04 + 0.16 * glossResponse)
           * glossResponse, 0, 0.75)
F  = mix(F0, 1, pow(1 - NoV, 5) * 0.35)
E  = clamp(D * (0.70 + 0.30 * glossResponse)
           * glossResponse * NoL, 0, 1)
gloss = F * E
result = colorIn * (ambient + diffuse) + gloss
```

The bounded square-root remap preserves useful response on the truck's
mid/high-gloss painted and metal regions. The single bounded matte guard
ramps the response from zero at authored gloss `0.08` to full response at
`0.20`; therefore authored zero gloss has `glossResponse = 0`, `F0 = 0`,
`E = 0`, and exactly zero modern specular. The guard also prevents a low-gloss
material from gaining an artificial wet highlight. No diffuse/global-lighting
term is modified by this follow-up.

Full evaluates material 1 and material 2 independently before the existing
mask composition. Single evaluates material 1 only. Building lights material 1
then preserves the existing multiply-2 derivation for material 2.

The #17 Bump ColorBlendDual branch was not changed. Its accepted math remains
outside this Multi9-only tuning hunk.

## Visual evidence

The confirmed scene is `FoundersCanal_01` on map 10. The final A/B used the
same temporary pinned camera override as the original issue evidence:

```text
4494.28 0.00 992.15 0.2500 0.7854 0.0000
```

The override was removed after capture. Both runs used
`modernPresentation 0`, `modernBloom 0`, frozen time 16, and reported
`waterFeature=1 multiFeature=1`. The modern-on log confirms the verified
Full LQ, Single LQ, and Building LQ rows compiled/activated through the
existing `bump_dual_multi` vertex variant. Full HQ and Single HQ remain
implementation-consistent but runtime-unverified in their intended static HQ
pairing, as required by issue #5.

Named surfaces in the same pinned case:

- Matte/low-gloss guard: the foreground painted truck side panel remains broad
  and subdued under modern-on, with no isolated wet/plastic highlight.
- Visible improvement: the same side panel has clearer seam/normal relief and
  a less purple response.
- Glossy contrast: the wheel rim gains a clearer edge response while the tire
  remains matte.

The updated reviewer contact sheet is
[`docs/evidence/issue-19-modern-multi9-contact-sheet.jpg`](evidence/issue-19-modern-multi9-contact-sheet.jpg).

For the stable pinned crops, modern-on versus modern-off measured:

| Region | Mean RGB delta | Channels with delta > 5 | Observation |
| --- | ---: | ---: | --- |
| Painted side panel / matte guard | 8.1083 | 64.34% | Broad subdued lift; no wet spot/coating |
| Wheel | 23.1132 | 82.89% | Stronger readable rim response; tire stays matte |

These are informational A/B metrics, not regression thresholds.

## Validation

- `agent/doctor.ps1` — READY; the documented v142 probe warning remains and
  the tested v145 fallback is used.
- `agent/build.ps1 -Configuration Release -Platform x86` — PASS in 24.3s;
  log: `agent/logs/build-Release-x86-20260817-212355.log`.
- `agent/smoke.ps1 -ExerciseCharacter -AccountName Dummy00009` — PASS after
  fresh-shard warm-up; character creation and MapServer entry verified in
  14.1s; log: `agent/logs/smoke-directdb-20260817-212835.json`.
- `modernMaterials 0` reference regression — 4/4 PASS, 0 regressions under
  the unchanged policy; changed-percent values were 2.0198%, 0.0636%, 0%,
  and 1.5643%; mean-delta advisories remain report-only exposure/weather
  noise; summary: `agent/logs/regression-20260817-213541.json`.
- Founders modern-off reference — PASS, pinned camera and
  `waterFeature=1 multiFeature=1`; log:
  `agent/logs/capture-FoundersCanal_01-20260817-212900.json`.
- Founders modern-on evidence — PASS, pinned camera, verified Full/Single/
  Building LQ activation, and clean exit; log:
  `agent/logs/capture-FoundersCanal_01-20260817-213254.json`.
- Explicit legacy control
  `-glslPilot 0 -modernMaterials 1` — PASS with clean exit and no native
  GLSL material path; log:
  `agent/logs/capture-FoundersCanal_01-20260817-213435.json`.
- #17 character guard — `AtlasPlaza_Closeup_01` with
  `-glslPilot 1 -modernMaterials 0` — PASS; the log shows the existing
  Bump ColorBlendDual variants active; log:
  `agent/logs/capture-AtlasPlaza_Closeup_01-20260817-213502.json`.

The added response uses only existing inputs and adds no texture sample or
render pass. The shared helper adds a bounded `sqrt`/`smoothstep` remap and
the associated scalar/vector gating per lit sub-material; the exact response
is opt-in and the legacy path remains untouched. The final implementation SHA
is posted in the issue completion comment after push.
