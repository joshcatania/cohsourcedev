# Issue #28 — Atlas modern atmosphere v1

Status: accepted for continuation as an opt-in v1 pilot on `agent/atlas-remaster-atmosphere-v1`.

Base: accepted #26 tip `f66875cfab1c860fb190cf88cbe31f7da3e7da70`.

Issue: [#28 — Atlas remaster atmosphere / skyline depth](https://github.com/joshcatania/cohsourcedev/issues/28)

## Result

The cheapest existing-state approach was sufficient. Modern Atmosphere reshapes the already-authored outdoor fog state: it keeps the near edge no earlier than the authored onset, moves the far edge to 82% of its authored distance, places the near edge at 45% of that new far distance when needed, and mixes 22% of the authored background color into the fog color when the background is non-black.

This gives Atlas the requested aerial perspective without a new shader term or pass. Near plaza geometry, the player, planters, and foreground streets remain crisp; mid-distance structures gain restrained contrast/color falloff; distant towers sit more naturally in the sky instead of reading as a disconnected cyan layer.

The feature is command-backed and default-off:

```text
modernAtmosphere 0       # exact accepted #26 behavior
modernAtmosphere 1       # opt-in native GLSL outdoor atmosphere
-glslPilot 0             # legacy control; atmosphere code is bypassed
```

The implementation is outdoor-only and gated by `glslPilot`. No texture, geometry, sky replacement, shadow/AO, tone mapping, bloom, volumetric fog, ray marching, or broad lighting changes were added. No native GLSL distance-atmosphere term was needed.

## Hard early visual gate

The gate used the persistent warmed shard, 1280x720 captures, the #25 Stock Ultra shadow/AO profile, and the accepted #26 Current Remaster control. City Hall tested near-field protection; East tested the skyline-heavy aerial-perspective problem.

Candidate A (`far = 0.74`, `near = 0.34`) was rejected immediately: it read as too foggy/blue and washed the far field. Candidate B (`far = 0.82`, `near = 0.45`, background mix `0.22`) was clearly better at normal screenshot size and was carried forward. Relative to the same-window Current Remaster control, Candidate B changed 51.67% of City Hall pixels (mean delta 18.18) and 18.63% of East pixels (mean delta 7.52); these are descriptive visual-difference measures, not quality thresholds.

Early controls and the selected candidate are in [the early evidence directory](evidence/issue28-atlas-remaster-atmosphere-v1/early/) and [the candidate-B City Hall capture](evidence/issue28-atlas-remaster-atmosphere-v1/early-atmosphere-candidate-b-cityhall/AtlasHero_CityHall_01.jpg).

## Final visual matrix

All views use the same three references: Stock Ultra, Current Remaster (#26), and #26 + Modern Atmosphere. The winning profile is native GLSL with `modernMaterials 1`, `modernLighting 1`, `modernPresentation 0`, `modernBloom 0`, `modernAtmosphere 1`, and the Stock Ultra shadow/AO settings.

| View | Current → Atmosphere changed pixels | Mean delta | Read |
| --- | ---: | ---: | --- |
| City Hall | 23.58% | 9.69 | Near plaza/player stay crisp; distant buildings integrate better. |
| East | 29.86% | 12.35 | Strongest skyline-depth improvement; towers recede into the environment. |
| North | 18.89% | 8.77 | Mid/far separation improves without a fog wall. |
| West | 4.57% | 2.97 | Deliberately restrained; no material regression. |

Comparison settings were `pixelTolerance 12`, `maxChangedPercent 100`; the metrics above are evidence of change magnitude, not a pass/fail image-diff contract. The complete normal-size matrix is in [the contact sheet](evidence/issue28-atlas-remaster-atmosphere-v1/contact-sheet.jpg), with a focused [East skyline crop](evidence/issue28-atlas-remaster-atmosphere-v1/skyline-crop.jpg).

Final source images:

- [Stock Ultra](evidence/issue28-atlas-remaster-atmosphere-v1/final-stock-ultra/)
- [Current Remaster (#26)](evidence/issue28-atlas-remaster-atmosphere-v1/final-current-remaster/)
- [#26 + Modern Atmosphere](evidence/issue28-atlas-remaster-atmosphere-v1/final-modern-atmosphere/)

## Guards

- Near-character framing is covered by the West final view; foreground character/world separation remains sharp.
- City Hall night and East night captures completed. They retain the existing low-light appearance and show no atmosphere-specific cyan wash, banding, or silhouette crush.
- Founders Falls canal capture completed with water enabled; near truck/ground detail remains crisp and the background does not acquire a fog wall.
- The legacy pair was run with `-glslPilot 0`, including an atmosphere-on command. The source gate bypasses the atmosphere path in both cases, so the distant skyline is not altered by this feature. A normal pixel comparison of that live pair was noisy (`14.01%`, mean delta `8.56`) because of NPC/exposure variation; it is recorded as a diagnostic, not presented as a pixel-perfect pass.

Guard images are in [the issue #28 evidence directory](evidence/issue28-atlas-remaster-atmosphere-v1/guard-founders-falls/) and its `guard-night-*` and `legacy-control-*` siblings.

## Performance sanity

These are coarse process/GPU-utilization samples around the same City Hall capture, not a frame-time benchmark:

| Configuration | CPU samples / mean / max | GPU samples / mean / max |
| --- | ---: | ---: |
| Current Remaster | 78 / 8.91% / 22.90% | 20 / 8.8% / 37% |
| Modern Atmosphere | 78 / 9.28% / 24.37% | 20 / 9.2% / 37% |

The machine-readable records are [Current Remaster](evidence/issue28-atlas-remaster-atmosphere-v1/performance-current-remaster/performance-current-remaster.performance.json) and [Modern Atmosphere](evidence/issue28-atlas-remaster-atmosphere-v1/performance-modern-atmosphere/performance-modern-atmosphere.performance.json). The small difference is consistent with reusing existing fog state rather than adding a new draw or shader branch.

## Validation

- `agent/doctor.ps1 -Json`: ready; the known v142 probe warning falls back to the verified v145 toolchain.
- `agent/build-client.ps1`: passed during fast iteration; final selected-candidate build log: `agent/logs/build-client-Release-x86-20260818-125631.log`.
- `agent/build.ps1 -Configuration Release -Platform x86`: passed in 47.3s; log: `agent/logs/build-Release-x86-20260818-131503.log`.
- `agent/start-shard.ps1 -StartupWaitSeconds 60`: observed ServerMonitor and required shard processes after 6.9s.
- `agent/smoke.ps1 -ExerciseCharacter -AccountName Dummy00009 -TimeoutSeconds 180`: first fresh-shard attempt timed out without a connect/login error during the documented warm-up; the bounded retry passed in 9.2s with character creation and MapServer entry. Retry result: `agent/logs/smoke-directdb-20260818-132025.json`.
- `agent/capture-regression.ps1` with `modernAtmosphere 0`: passed 4/4 with no baseline adoption; result: `agent/logs/regression-20260818-130923.json`.

The formal regression remains off, preserving the accepted #26 baseline. Stock Ultra remains the absolute shipped benchmark, and the feature is default-off for normal users.
