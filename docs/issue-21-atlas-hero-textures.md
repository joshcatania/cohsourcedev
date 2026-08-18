# Issue #21 — Atlas hero textures v1

Branch: `agent/atlas-hero-textures-v1`

Base: `ca8c92a56d474ea4699326b2500f24a95da21b28`

## Runtime-ranked selection

The candidate list came from the bounded `TEXTUREPILOT` material-bind trace on four valid Atlas Park views, not from screenshot guessing. The corrected deterministic shot table now uses the verified in-zone camera `500.00 120.00 -800.00`, with fixed headings for City Hall, East, North, and West.

| Material group | BASE1 observations | Views | Stock dimensions | Decision |
| --- | ---: | ---: | --- | --- |
| `Plaza_Concrete_Trims02` | 8 | 4 | 512×256 base + normal | Selected; broad near-player plaza trim/wall coverage |
| `AP_Asphalt_01` | 7 | 4 | 512×512 base + normal/detail | Selected; repeated Atlas ground/road coverage |
| `deco_building_03_concrete_01` | 9 | 4 | 512×512 base + normal/detail | Selected; highest-reuse upgraded facade material |
| `AP_CityHall_Border_01` | 4 | 4 | 512×256 base + normal | Selected; named City Hall architectural surface |
| `AP_CityHall_Tiles_01` | 4 | 4 | 512×512 base + normal | Selected; named City Hall/plaza paving surface |

Rejected candidates were not silently processed: `AP_CityHall_Concrete_01` and `Plaza_Concrete_Wall_FoundationStained_01` already use 1024px sources; `Plaza_Grass_FreshCut_01` is already capped on its base; skyline LODs and tiny props were not useful hero targets.

## Processing and semantics

The accepted Issue #20 workflow is now manifest-driven through `agent/texture-pilot.ps1` and `agent/texture_pilot.py`. The committed manifest is [agent/atlas-hero-textures-v1.json](../agent/atlas-hero-textures-v1.json). It contains only selected logical paths, stock hashes, stored names, flags, semantic type, target dimensions, and expected output hashes; no extracted proprietary source dump is committed.

Each selected layer is processed exactly once at conservative 2×, capped at 1024px. Base/detail layers use LANCZOS plus the restrained Issue #20 sharpen and nearest alpha. Normal/gloss layers resize encoded vectors, renormalize RGB, and nearest-resize the authored alpha channel. GetTex runs through neutral temporary package names so `_N` filenames cannot trigger an unintended normal-map channel repack; the original stored DDS name and stock container flags are restored and verified afterward.

The generated set is 13 files across five material groups. All generated dimensions, stored names, flags, output hashes, normal-vector length, and alpha codec error bounds were verified. No character textures, renderer/material/post-processing code, pigg contents, or whole-library conversion were introduced.

## Reversible controls

After extracting only the selected files locally from `stage2c.pigg` and `stage3g.pigg`, use:

```powershell
.\agent\texture-pilot.ps1 -Action Generate `
  -ManifestPath .\agent\atlas-hero-textures-v1.json `
  -StockRoot C:\path\to\extracted-stock `
  -OutputRoot $env:TEMP\coh-issue21-atlas-pilot

.\agent\texture-pilot.ps1 -Action Install `
  -ManifestPath .\agent\atlas-hero-textures-v1.json `
  -OutputRoot $env:TEMP\coh-issue21-atlas-pilot

.\agent\texture-pilot.ps1 -Action Remove `
  -ManifestPath .\agent\atlas-hero-textures-v1.json

.\agent\texture-pilot.ps1 -Action Restore `
  -ManifestPath .\agent\atlas-hero-textures-v1.json `
  -OutputRoot $env:TEMP\coh-issue21-atlas-pilot
```

`Install` copies only the 13 manifest paths under `bin/data/texture_library`. `Remove` refuses to delete an unrelated file. `Restore` removes only matching generated overrides, confirms all selected loose paths are absent, and re-hashes both packed stock piggs. The verified final state has zero selected loose overrides and unchanged packed hashes.

## Visual evidence

The A/B contact sheet is [docs/evidence/issue21-atlas-hero-contact-sheet.jpg](evidence/issue21-atlas-hero-contact-sheet.jpg), with stock and manifest-pilot renders for the same four deterministic cameras. The useful-distance improvement is clearest on the repeated `Plaza_Concrete_Trims02` / `AP_CityHall_Tiles_01` paving and the `AP_CityHall_Border_01` wall/facade edges; the pilot produces cleaner tile boundaries and more stable fine surface detail without changing the material or lighting path. The raw comparison metrics are recorded in [docs/evidence/issue21-atlas-hero-metrics.json](evidence/issue21-atlas-hero-metrics.json); full-frame pixel drift is treated as supporting evidence only because the capture harness documents JPEG and exposure noise.

The visual loop included a failed East capture that was all black despite a technically clean client exit; it was rejected and rerun successfully before the final contact sheet was made.

## Verification

- `agent/doctor.ps1 -Json`: ready baseline passed.
- `agent/build.ps1 -Configuration Release -Platform x86`: passed after the final shot-table change.
- `agent/smoke.ps1 -ExerciseCharacter -AccountName Dummy00009 -TimeoutSeconds 180 -Json`: passed after the expected cold-shard warm-up retry; direct DbServer login, character creation, and MapServer entry were proven.
- Four installed pilot captures passed with clean client exit; one black East capture was discarded and rerun.
- One post-restore default-table capture passed with no temporary camera override and showed stock 512px runtime binds.
- Final restore passed: 13 loose overrides absent; `stage2c.pigg` and `stage3g.pigg` matched manifest SHA-256 values.
