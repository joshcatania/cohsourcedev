# Issue #23 — Atlas visual textures v2

This pass starts from `21670743151bfd59038e6dcde565834a9e95e4d2` on `agent/atlas-visual-textures-v2` and remains stock-by-default. It adds a manifest-driven, reversible v2 pilot for five remaining City Hall material/prop groups. No renderer, material, post-processing, character, camera, or regression-baseline changes are included.

The visual evidence is [the runtime contact sheet](evidence/issue23-atlas-visual-textures-v2-contact-sheet.jpg). Machine-readable capture and control results are in [the metrics record](evidence/issue23-atlas-visual-textures-v2-metrics.json).

## Runtime ranking and selection

The ranking used actual `TEXTUREPILOT` bind observations from the four dedicated `AtlasHero_CityHall_01`, `AtlasHero_East_01`, `AtlasHero_North_01`, and `AtlasHero_West_01` views. The already accepted #21 groups were excluded before ranking: `Plaza_Concrete_Trims02`, `AP_Asphalt_01`, `AP_CityHall_Border_01`, `AP_CityHall_Tiles_01`, and `deco_building_03_concrete_01`.

| Rank | Runtime weakness | Evidence | Decision |
| --- | --- | --- | --- |
| 1 | `AP_CityHall_Concrete_01_D/NS` — soft 1024px concrete on the main stair/facade | `AP_Plaza_Main`, submesh 0; close and repeated in the City Hall view | Selected; same-resolution restoration |
| 2 | `AP_CityHall_Details_01/NS/M` — soft door/window/arch relief | `AP_Plaza_Main`, submesh 6; stable close bind | Selected; 2x |
| 3 | `AP_CityHall_Dome_01/NS` — low-resolution radial dome ribs | `AP_CityHall_Makeover_01`, submesh 1; prominent in the close view | Selected; 2x |
| 4 | `AP_CityHall_Plaque_01_D/NS` — readable sign limited by 512x128 source | `AP_CityHall_Makeover_01`, submesh 11; the City Hall plaque is visible in all close comparisons | Selected; 2x |
| 5 | `AP_CityHall_Border_02_D/NS` — soft architectural trim | `AP_Plaza_Main`, submesh 3; visible above the stairs | Selected; 2x |
| 6 | `Plaza_Concrete_Wall_FoundationStained_01_D` — soft 1024x512 foundation | Observed on a far `_ap_blockfoundation_07_LOD`; not a stable close subject | Deferred |
| 7 | `AP_CityHall_Border_03_D/NS` — lower visible area than Border 02 | Runtime bind observed, but less coverage in the dedicated views | Deferred |
| 8 | `AP_CityHall_Pillar_01_D/NS` — narrow columns | Small projected area and repetition makes aliasing/shimmer more likely | Deferred |
| 9 | `AP_CityHall_Gold_D`, `AP_CityHall_Marble_D` — small trim materials | Visible, but each has lower projected coverage than the selected groups | Deferred |
| 10 | `street_sidewalk1`, `Clean_PKlot`, kiosks, benches, and `ironfence3` | Repeated or distant binds without a stable close subject in the four authoring views | Deferred |

This keeps the pass to five high-return groups, within the requested 3–6 range. The City Hall stairs/concrete, dome, plaque, details, and border are visibly cleaner in the stock/v2 side-by-side contact sheet at the established gameplay distance; the dome and plaque are the clearest named improvements, with the stairs/concrete and facade details also showing reduced softness.

## Treatments

The source and output paths, packed-source hashes, stock hashes, and expected output hashes are pinned in [`agent/atlas-visual-textures-v2.json`](../agent/atlas-visual-textures-v2.json). The manifest contains 11 runtime texture entries across the five groups:

| Group | Runtime layers | Stock → output | Treatment |
| --- | --- | --- | --- |
| `cityhall-concrete-01` | `AP_CityHall_Concrete_01_D`, `_NS` | 1024x1024 → 1024x1024 | Deterministic same-resolution base restoration; normal/gloss vector renormalization with gloss alpha preserved |
| `cityhall-details-01` | `AP_CityHall_Details_01`, `_NS`, `_M` | 512x512 → 1024x1024 | Conventional 2x LANCZOS base/normal upscale; nearest-neighbor mask |
| `cityhall-dome-01` | `AP_CityHall_Dome_01`, `_NS` | 512x512 → 1024x1024 | Conventional 2x LANCZOS base/normal upscale |
| `cityhall-plaque-01` | `AP_CityHall_Plaque_01_D`, `AP_CityHall_Plaque_NS` | 512x128 → 1024x256 | Conventional 2x LANCZOS base/normal upscale |
| `cityhall-border-02` | `AP_CityHall_Border_02_D`, `_NS` | 256x128 → 512x256 | Conventional 2x LANCZOS base/normal upscale |

The v2 tooling changes are limited to `agent/texture_pilot.py`: it adds explicit `restore_same_resolution` and `upscale_2x` manifest modes, nearest-preserving masks, deterministic same-size base restoration, and a per-entry normal/gloss alpha-range tolerance for codec verification. The default behavior and accepted v1 manifest remain compatible; no generative repainting or AI-generated pixels are used.

The generated selected outputs total 7,606,111 bytes, versus 3,477,343 bytes for the selected stock files, a 4,128,768-byte increase. The maximum output dimension is 1024px; no texture exceeds the cap.

## Controls and verification

- Python syntax check passed for `agent/texture_pilot.py`.
- v2 generation passed twice with the same output hashes recorded in the manifest.
- Existing #21 v1 manifest regeneration passed with the updated tooling.
- Install produced 11 selected loose overrides; all four dedicated captures exited 0.
- Stock/v2 comparisons used the existing comparison harness at pixel tolerance 12 and compare width 320. Changed pixels were 1.4795%, 2.4823%, 0.3849%, and 0.4926% for City Hall, East, North, and West respectively; these are visual A/B evidence, not new baselines.
- `-modernMaterials 0` capture passed while v2 was installed.
- Explicit restore removed all selected loose overrides; the packed stock paths were confirmed active afterward.
- `Release|x86` build passed using the repository’s verified v145 fallback after stopping the disposable local shard.
- Direct-DB character/map smoke passed after the documented cold-shard warm-up; the first immediate post-restart attempt timed out without a login or fatal error, and the warmed rerun created the character, reached MapServer, and exited 0.
- Formal existing AtlasPlaza regression passed 4/4 with `Dummy00018`, baseline adoption count 0, and no camera or baseline changes. Changed-percent values were 0.5544%, 0.0230%, 0%, and 4.7581%, all below the existing 6% hard limit.

The worktree is stock-by-default after restore: the committed deliverables are the v2 manifest, the deterministic tooling extension, and the evidence/report files. Runtime overrides are not committed or left active.
