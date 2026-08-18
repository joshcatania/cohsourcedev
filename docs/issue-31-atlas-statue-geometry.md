# Issue #31 — Atlas statue geometry v1

Status: PASS for the Phase A round-trip gate and Geometry v1 visual gate. The work starts from accepted #29 SHA `361a96eb3600d61dc4b312a5eced7d8a89dbff56` on `agent/atlas-statue-geometry-v1`.

The Sol pre-start correction was applied: this is issue #31, and the evidence directory is `docs/evidence/issue31-atlas-statue-geometry-v1/`.

## Scope and controls

The exact runtime asset is:

`object_library/City_Zones/Elements/Hero_Statues/Male_Statue_Atlas/Male_Statue_Atlas.geo`

The packed runtime source was identified from the accepted #29 runtime trace as `bin/piggs/stage3c.pigg`, SHA256 `521D63B6817A313C5C6C03DCB25348B6D34A537698B69829BBF7F1F9B2BFCE69`. The packed extracted stock geo is SHA256 `CC3A946DF13F1AE6FE9CF602EB44004CDB52EB03B8530DD4DCF2E52B017FD9EE`.

The accepted #29 texture manifest was used unchanged. It contains the seven accepted texture overrides and the original packed source hashes for `stage2c.pigg` and `stage3g.pigg`; it is copied into [accepted-texture-manifest.json](evidence/issue31-atlas-statue-geometry-v1/accepted-texture-manifest.json). No texture, material shader, lighting, post-processing, or presentation change is part of this issue.

## Phase A gate: untouched round trip

The deterministic editable VRML export is [Male_Statue_Atlas.phase-a.wrl](evidence/issue31-atlas-statue-geometry-v1/phase-a/Male_Statue_Atlas.phase-a.wrl), SHA256 `691CCA0FB744623118234706366970A59293AF157EE63317E8748419ACC9234E`.

The source export was generated with:

```text
Push-Location bin
..\Utilities\GetVrml\bin\x86\Release\GetVrml.exe -geo2wrl ./piggs/stage3c.pigg:/object_library/city_zones/Elements/Hero_Statues/Male_Statue_Atlas/Male_Statue_Atlas.geo -wrlout <work>\source-stage3c\object_library\city_zones\Elements\Hero_Statues\Male_Statue_Atlas\Male_Statue_Atlas.wrl -factsout <work>\source-stage3c\source-facts.json -nogui -nocheckout -onlypig
Pop-Location
```

The unmodified VRML was regenerated with the offline authoring path:

```text
Utilities\GetVrml\bin\x86\Release\GetVrml.exe bin\data\object_library\city_zones\Elements\Hero_Statues\Male_Statue_Atlas\Male_Statue_Atlas.wrl -f -nogui -atlasoffline -nopig -nolod -nomeshmend -outputdir <work>\roundtrip-reduced
```

The generated round-trip geo is SHA256 `634FB0855B76E7CFDBD16ACD70B1E948CA04AB5761C7C22FC01B6BBAB6F4762D`. The source and round-trip facts are [source-facts.json](evidence/issue31-atlas-statue-geometry-v1/phase-a/source-facts.json) and [roundtrip-facts.json](evidence/issue31-atlas-statue-geometry-v1/phase-a/roundtrip-facts.json).

The authored identity check is exact for all three model/LOD records:

| model | source | round trip | authored hashes |
| --- | --- | --- | --- |
| `_H_M_Statue_Atlas_Giant` | 2,955 verts / 4,608 tris | 2,955 / 4,608 | position `08DEACA8`, normal `7A1C42FC`, UV `C1A032BD`, triangle `154A2571` |
| `_H_M_Statue_Atlas_Giant_LOD_01` | 1,694 / 2,442 | 1,694 / 2,442 | position `DFE9A46F`, normal `D1AF8902`, UV `F5662C75`, triangle `CC5A03DC` |
| `_H_M_Statue_Atlas_Giant_LOD_02` | 353 / 380 | 353 / 380 | position `B9036560`, normal `0D2AD239`, UV `EDDB8957`, triangle `F61F5154` |

The named material runs are unchanged: Globe `960/128/80` triangles and body `3,648/2,314/300` triangles for the high, LOD1, and LOD2 records respectively. Runtime traces prove both existing bindings remain active: `X_Male_Statue_Atlas_Globe_01` on submesh 0 and `X_Male_Statue_Atlas_01` on submesh 1.

The VRML importer canonicalizes the texture table order from `white, body, globe` to `white, globe, body`, but the binding is name-based and the per-material triangle ownership/counts and all authored geometry hashes remain exact. No material identity or visual binding changed. The only geo differences are derived reduction instructions: they are rebuilt from a sanitized copy so the runtime retains `lodCount=3`; authored positions, normals, UVs, winding, model names, and submesh ownership are untouched.

The reversible loose override was installed at the exact `bin/data/.../Male_Statue_Atlas.geo` path. The packed and loose captures were taken in the same runtime window and passed the capture harness:

```text
changedPercent = 0.0883%
meanDelta      = 0.8363
maxDelta       = 26
```

The packed and loose runtime traces report the same `2,955/4,608`, `lodCount=3`, bounds, and two named material bindings. See [packed runtime](evidence/issue31-atlas-statue-geometry-v1/phase-a/runtime-packed.txt), [loose runtime](evidence/issue31-atlas-statue-geometry-v1/phase-a/runtime-loose.txt), [packed image](evidence/issue31-atlas-statue-geometry-v1/phase-a/packed-stock.jpg), [loose image](evidence/issue31-atlas-statue-geometry-v1/phase-a/loose-roundtrip.jpg), and [parity.json](evidence/issue31-atlas-statue-geometry-v1/phase-a/parity.json). Phase B was started only after this gate passed.

## Phase B: bounded high-detail geometry

The geometry pilot is [agent/atlas-statue-geometry.py](../agent/atlas-statue-geometry.py). It changes only `_H_M_Statue_Atlas_Giant`; the two material shapes have independent edge caches, so a recovery edge cannot cross the Globe/body seam. Original vertices remain anchors. Each source triangle becomes four triangles, with UVs linearly interpolated and normals averaged from the authored endpoints.

The edge point is recovered from the authored endpoint normals by projecting the linear midpoint onto both endpoint tangent planes, averaging those projections, applying the bounded normal-recovery gain, and clamping the displacement to `min(edge * 0.32, 4.0)`. The final pilot uses gain `4.0`; this is a curvature-guided correction with an explicit absolute bound, not arbitrary triangle inflation.

Final high-detail result:

```text
vertices       2,955 -> 10,482
triangles      4,608 -> 18,432
triangle bound 20,000 (pass)
new edge verts 7,527
mean recovery displacement 1.3119
max displacement           4.0000
radius                     294.210 -> 297.373
```

The final editable pilot VRML hash is `27B3E07752A4394D3DF1FABA7F9D46DFCF0C70A0726636A5DE41E2ACDBDF8FD8`; the generated high-detail geo hash is `7EA60ED4BEECA0801AE6BA39DDD09FD9B0239E8C67CCD27BE178431FC290EF65`. The final fact manifest is [geometry-facts.json](evidence/issue31-atlas-statue-geometry-v1/geometry/geometry-facts.json), and the method/count report is [geometry-pilot-report.json](evidence/issue31-atlas-statue-geometry-v1/geometry/geometry-pilot-report.json).

LOD1 and LOD2 remain authored-array identical to the Phase A round trip, including their positions, normals, UVs, triangle hashes, names, and material runs. The final high-detail material runs are Globe `3,840` and body `14,592` triangles. The final runtime still reports `lodCount=3` and the same two texture bindings.

At normal screenshot size the final globe lower silhouette is visibly more continuous and rounded than the #29 control while retaining the statue’s proportions. The full comparison is [contact-sheet.png](evidence/issue31-atlas-statue-geometry-v1/contact-sheet.png); the silhouette/detail crop is [geometry-comparison-crop.png](evidence/issue31-atlas-statue-geometry-v1/geometry-comparison-crop.png). The same-window #29-versus-geometry harness result is [geometry/parity.json](evidence/issue31-atlas-statue-geometry-v1/geometry/parity.json): `2.1186%` changed pixels, `meanDelta=1.9906`, and PASS under the existing `6%` hard limit.

## Validation and cleanup

The final GetVrml project build passed with 0 warnings and 0 errors. The repository Release/x86 build passed using the verified v145 fallback in 17.7 seconds. The post-build direct-DB character/map smoke passed in 1.95 seconds with `characterCreated=true`, `mapConnected=true`, and TestClient exit code 0. The post-build Ouroboros capture loaded the loose final geo and exited cleanly. Details are in [runtime-postbuild.txt](evidence/issue31-atlas-statue-geometry-v1/geometry/runtime-postbuild.txt).

The final cleanup was hash-guarded and restored stock-by-default behavior. The geometry status is `packed-stock`, the exact loose geometry path is absent, the staged editable VRML is absent, and all seven manifest-selected texture loose paths are absent. See [operations.txt](evidence/issue31-atlas-statue-geometry-v1/operations.txt).

The requested post-build non-statue Atlas validation was also run with all loose statue geometry and #29 texture paths removed. The fresh-shard smoke retry passed (`characterCreated=true`, `mapConnected=true`), and all four formal Atlas regression captures completed. The latest formal comparison window was environmentally noisy: one shot passed and three exceeded the existing six-percent hard threshold because of broad sky/character-state drift; no baseline was adopted and no threshold changed. A prior same-stock control window passed all four shots. The compact record, including both windows and exact metrics, is [stock-regression.json](evidence/issue31-atlas-statue-geometry-v1/stock-regression.json). This does not alter the Phase A or Geometry v1 verdicts, which are established by the material-preserving facts and same-window statue comparisons above.
