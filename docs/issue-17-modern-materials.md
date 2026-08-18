# Issue #17 — Modern Bump ColorBlendDual material response

This report records the opt-in native GLSL material experiment requested by
issue #17. The committed visual artifact is the [three-scene off/on contact
sheet](evidence/issue-17-modern-materials-contact-sheet.jpg). Full-resolution
captures and per-run logs remain local under `agent/captures/issue17/` and
`agent/logs/`.

## Scope and compatibility

- `game_state.modernMaterials` is command-backed and defaults to `0`.
- `modernMaterials 0` keeps the existing Bump ColorBlendDual LQ/HQ equation
  and output path unchanged.
- `modernMaterials 1` is consumed only by the two canonical native GLSL
  Bump ColorBlendDual programs: default/LQ and `BIT_HIGH_QUALITY`/HQ.
- `-glslPilot 0` remains the legacy ARB/Cg control path regardless of the
  modern-material flag.
- Primary captures explicitly used `modernPresentation 0 modernBloom 0`.
- No Multi9, source textures, Bump Multiply, shadows, water, post-processing,
  fallback, or material-table migration was included.

## Modern response

Both variants retain the existing three texture samples and base/tint
construction. The sampled normal RGB is expanded and normalized as
`N = normalize(normalRGB * 2 - 1)`, and sampled normal alpha remains the gloss
mask. The existing gloss constant `.w` remains the scalar gloss input, the
existing specular RGB remains the authored specular color, and the existing
exponent remains the lobe-width authoring input.

For `modernMaterials 1`, the shared response is:

```text
L = normalize(light)
V = normalized view direction
H = normalize(V + L)
NoL = saturate(dot(N, L))
glossStrength = saturate(normalGloss.a * max(glossConstant, 0))
roughness = clamp(sqrt(2 / (legacyExponent + 2)), 0.18, 0.95)
lobeExponent = clamp(2 / roughness^2 - 2, 1, 128)
F0 = saturate(specularRGB * (0.04 + 0.16 * glossStrength))
F = lerp(F0, 1, pow(1 - saturate(dot(N, V)), 5))
D = pow(saturate(dot(N, H)), lobeExponent) * (lobeExponent + 2) / (2 * pi)
specularEnergy = saturate(D * 0.25 * glossStrength * NoL)
gloss = F * specularEnergy
diffuse *= clamp(1 - max(F) * glossStrength * 0.5, 0.65, 1)
output.rgb = base.rgb * (ambient + diffuse) + gloss
```

The diffuse path remains ambient/Lambert first, with the bounded energy
adjustment above. LQ uses the existing tangent-space light and view
interpolants. HQ reconstructs the view direction from view-space position and
the interpolated tangent basis, and uses `g_LightDirFP` for the view-space
light direction. No extra texture sample, render pass, or material family was
added. The optional `g_ModernMaterialParamsFP` uniform is resolved and
uploaded only for these two programs.

## Visual observations

The character row shows a localized change in costume response: the
modern branch produces a broader, Fresnel-weighted highlight/falloff response
while retaining authored tint colors and normal-map detail. The low-light row
remains intentionally restrained; its global sky/exposure differs between
process launches because the capture freeze locks eye adaptation at different
convergence points. The close-up also has idle-animation phase variance, so it
is visual evidence rather than a pixel-regression target.

The earlier day off/on comparator measured 49.22% changed pixels and meanDelta 26.76;
the close-up measured 7.43% and meanDelta 8.09; and the low-light pair
measured 48.71% and meanDelta 31.57. These are reported as visual A/B
observations, not parity claims: the known capture exposure/weather and idle
phase variance dominate the global metrics. All captures exited cleanly.

## World/environment coverage follow-up

The bounded search did not find a world surface with a material change as
strong as the character response. The best reproducible non-character case is
`FoundersCanal_01`, using only the foreground truck side panel, door hardware,
panel rivets, tire tread, and wheel. The map is used as an environment probe;
water is outside this evaluation and was not inspected or changed.

Both runs used the same deterministic shot-table view (`map 10`, fixed
camera/time setup, `timeset 16`, `timescale 0`) and explicitly launched with:

```text
-glslPilot 1 -modernMaterials 0|1 -modernPresentation 0 -modernBloom 0
```

The OFF run is recorded in
`agent/logs/capture-FoundersCanal_01-20260817-193804.json` and the ON run in
`agent/logs/capture-FoundersCanal_01-20260817-193923.json`; both exited cleanly.
Their stdout telemetry registers vertex program 251 as `bump_dual HQ` and
reports repeated
`BLENDMODE_BUMPMAP_COLORBLEND_DUAL_HQ active (bump_dual HQ vertex variant)`
events in both runs (`...193804.stdout.log` and `...193923.stdout.log`).

The full-frame OFF/ON comparison is 1.1758% changed pixels with meanDelta
2.5865. Visual review of the truck surface is intentionally conservative:

- Normal-map shape: panel rivets, door hardware, tire grooves, and wheel shape
  remain readable, but ON does not produce a clear additional relief cue.
- Highlight width/falloff: the panel highlight is effectively unchanged; only
  sparse rim/underbody pixels move, with no robust lobe-width conclusion.
- Tint preservation: the authored violet-gray truck tint is preserved.
- Plastic/wet appearance: the truck and asphalt do not become artificially
  plastic or wet; the water surface is not part of this test.

This means the telemetry proves the canonical HQ pairing is active during the
world capture, but the named truck surface does not show a material response
that is materially distinguishable from the controlled OFF image. A fixed
Talos arrival wall probe was also clean but measured 0.0% changed pixels. The
bounded result is therefore that representative world coverage is effectively
character-dominant for this opt-in response; no shader-math change is justified
and the scope is not broadened into Multi9, textures, Bump Multiply, shadows,
water, post-processing, or fallback work. The updated contact sheet keeps this
world control beside the character evidence so the limitation is visible.

## Validation

The accepted infrastructure commit was verified as a direct child of the
original branch tip and cherry-picked cleanly. The resulting infrastructure
commit is `98819fca6038a15f875f7503e1ad47c519904617`; its five-file diff is
limited to the capture/direct-DB configuration and agent-status documentation.
The worktree was clean immediately after that cherry-pick.

Mandatory pre-renderer gate:

- Direct-DB character/map smoke: PASS —
  `agent/logs/smoke-directdb-20260817-184921.json`.
- Normal Atlas deterministic capture: PASS —
  `agent/logs/capture-AtlasPlaza_CityHall_03-20260817-184940.json`.

Post-change controls:

- Release/x86 build with the repository’s v145 fallback: PASS —
  `agent/logs/build-Release-x86-20260817-185729.log`.
- Direct-DB character/map smoke: PASS —
  `agent/logs/smoke-directdb-20260817-190720.json`.
- Explicit hybrid reference, `-glslPilot 1 -modernMaterials 0
  -modernPresentation 0 -modernBloom 0`: PASS —
  `agent/captures/issue17/reference/`.
- Legacy control, `-glslPilot 0 -modernMaterials 1
  -modernPresentation 0 -modernBloom 0`: PASS — clean exit and no GLSL pilot
  activation/compile/link/uniform diagnostics — `agent/captures/issue17/legacy/`.
- Modern LQ/HQ day, close-up, and low-light captures: PASS — clean exits;
  their command lines explicitly set `modernMaterials 1` and both presentation
  controls to `0`.

The final branch SHA is posted with the issue follow-up after push.
