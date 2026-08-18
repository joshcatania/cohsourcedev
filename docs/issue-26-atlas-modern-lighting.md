# Issue #26 — Atlas remaster lighting v1

Status: visual gate passed; implementation and validation complete on `agent/atlas-remaster-lighting-v1`.

Base: `ba16c9170d624400767af7832e40e6b090aae8e`.

The latest #26 scope-amendment comment was read before work began. Phase 0 was kept bounded to a warmed-shard/client-only loop; no build or server architecture was redesigned.

## Phase 0

### 0A — persistent warmed shard

The existing `start-shard.ps1` + direct-DB `smoke.ps1 -ExerciseCharacter` workflow was used. After the expected fresh-shard warm-up, the smoke passed and the shard was left running while successive client rebuild/capture cycles ran. The ServerMonitor/DbServer/Launcher/MapServer process set stayed up across those cycles; only `Ouroboros.exe` was relaunched.

The fresh post-validation restart reproduced the documented warm-up behavior: the first 180-second smoke attempt timed out, then the retry passed in 44.8 seconds and verified character creation and MapServer entry. This is recorded rather than treated as a renderer failure.

### 0B — narrow client build

[`agent/build-client.ps1`](../agent/build-client.ps1) reuses the repository toolchain selection and the tested v145 fallback, builds only `Game.vcxproj`, disables the legacy post-build event, and copies only `Game.exe`/`Game.pdb` to `bin/Ouroboros.exe`/`bin/Ouroboros.pdb`.

It fails clearly if `Ouroboros.exe` is still running, is safe to repeat, and does not touch server binaries. The final run passed in 3.3 seconds while the warmed shard remained online. The full Release/x86 solution build remains the authoritative restart boundary because the normal post-build step copies shared PhysX/runtime files that are locked by a live shard; the full build passed in 35.2 seconds after stopping the disposable local shard.

## Lighting implementation

The new command is `modernLighting`, defaulting to `0` in [`Game/src/cmdparse/cmdgame.c`](../Game/src/cmdparse/cmdgame.c). The implementation is in [`Game/src/graphics/sun.c`](../Game/src/graphics/sun.c):

- `sunApplyValuesWithModernLighting` is selected only when native GLSL is enabled, `modernLighting` is enabled, and the scene is outdoors.
- The authored sky-file sun direction and sky/fog inputs remain the source of truth.
- Direct energy is taken from the authored diffuse peak and expressed as a restrained warm lobe (`0.95, 0.88, 0.58`).
- Ambient fill is reduced to `0.82` of the authored value and uses restrained sky chroma; Atlas’s black `bg_color` field falls back to authored ambient chroma.
- Player ambient/diffuse values and the existing no-angle light are rebuilt from those same inputs, keeping character and world response coherent.
- The existing material equations, texture inputs, shadow settings, AO settings, post-processing, global grading, and fog/atmosphere systems are unchanged.

`-glslPilot 0 -modernLighting 1` takes the original `sunApplyValues` branch. The new lighting therefore cannot silently alter the legacy control path.

## Hard early visual gate

The gate used 1280x720 captures with the #25 Stock Ultra profile:

`shadowmode=4`, `shadowmapshader=3`, `shadowmapsize=1`, `shadowmapdistance=2`, `ambientstrength=3`, `ambientresolution=2`, `ambientblur=6`.

The only comparison switch was `-modernLighting 0` versus `-modernLighting 1`; `modernPresentation` and `modernBloom` stayed off. The City Hall pair is immediately distinguishable at normal screenshot size: the remaster has warmer sunlit paving and highlights, cooler/deeper shade, and clearer light/shade separation without a grading or post-process effect.

- [City Hall — Stock Ultra](evidence/issue26-atlas-remaster-v1/CityHall_StockUltra.jpg)
- [City Hall — Modern Lighting v1](evidence/issue26-atlas-remaster-v1/CityHall_ModernLightingV1.jpg)
- [Full contact sheet](evidence/issue26-atlas-remaster-v1/contact-sheet.jpg)

## Four-view generalization

All four AtlasHero views were captured after the final Release/x86 build. The A/B metrics below use `compare-captures.ps1 -PixelTolerance 12 -MaxChangedPercent 100`; they are descriptive because a successful lighting change is expected to move a large fraction of pixels.

| View | Changed pixels | Mean delta | Max delta |
| --- | ---: | ---: | ---: |
| City Hall | 44.1879% | 12.6125 | 137 |
| East | 38.3245% | 11.4518 | 117 |
| North | 44.2090% | 11.2066 | 218 |
| West | 63.6511% | 13.0260 | 81 |

The individual final pairs are included in [`docs/evidence/issue26-atlas-remaster-v1`](evidence/issue26-atlas-remaster-v1/), with the contact sheet providing the normal-size review surface.

## Character, world, and low-light guards

All guard captures exited cleanly with `modernLighting=1`:

- [Character close-up](evidence/issue26-atlas-remaster-v1/Character_Closeup_ModernLightingV1.jpg) — repeated once after warm-up to avoid the known close-up idle-animation phase issue.
- [Founders Falls outdoor/world guard](evidence/issue26-atlas-remaster-v1/FoundersCanal_ModernLightingV1.jpg).
- [Night City Hall guard](evidence/issue26-atlas-remaster-v1/NightCityHall_ModernLightingV1.jpg).
- [Night East guard](evidence/issue26-atlas-remaster-v1/NightEast_ModernLightingV1.jpg).

The night shots remain deliberately low-light and retain their existing dark-world behavior; no night-specific brightening or presentation effect was introduced.

## Controls and performance

The final legacy control pair used `-glslPilot 0` with `modernLighting=0` and `modernLighting=1`. Both captures passed; the comparison changed 0.2595% of pixels with mean delta 1.2509 and max delta 69, confirming that the modern-lighting switch is inert on the legacy renderer path.

The #25-style bounded performance sample used the same Stock Ultra profile and a 10-second runtime sample. CPU mean was 8.96% of machine capacity for Stock Ultra and 9.00% for Modern Lighting v1; CPU maxima were 22.64% and 22.48%. GPU means were 17.00% and 13.85%, with the same 38% maximum. PresentMon was available but emitted no frame-time CSV on this capture path, so no FPS number is claimed. The raw summaries are [Stock Ultra](evidence/issue26-atlas-remaster-v1/Performance_StockUltra.settings.json) and [Modern Lighting v1](evidence/issue26-atlas-remaster-v1/Performance_ModernLightingV1.settings.json).

## Validation record

- `agent/doctor.ps1 -Json`: ready; the known v142 probe warning fell back successfully to v145.
- Full `agent/build.ps1 -Configuration Release -Platform x86`: pass, 35.2 seconds; log `agent/logs/build-Release-x86-20260818-115035.log`.
- Final `agent/build-client.ps1 -Configuration Release -Platform x86 -Json`: pass, 3.3 seconds; log `agent/logs/build-client-Release-x86-20260818-120800.log`.
- Direct-DB character/map smoke after the final restart: retry pass; result `agent/logs/smoke-directdb-20260818-115450.json`.
- Formal native-GLSL, modern-lighting-off regression: `agent/logs/regression-20260818-113800.json`, 4/4 pass with no baseline adoption.
- A later fresh-shard formal run reported 3/4: East/North/West passed, while City Hall had a 99.95% changed-pixel sky/horizon exposure/weather shift against the older baseline. No baseline was changed. Two successive final off captures on the same warmed shard passed the normal 6% criterion at 2.4153% changed pixels; this isolates the failure to the documented live-weather/exposure variance rather than the new code.
- Legacy `-glslPilot 0` control: final off/on captures both passed and were effectively identical as reported above.

The generated exploratory captures and full runtime logs remain outside the commit; the curated contact sheet, final comparison pairs, guards, and performance summaries are the committed evidence set.
