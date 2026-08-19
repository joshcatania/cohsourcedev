# Issue #33 — Atlas statue Blender vertical slice

Status: PASS. The branch starts at accepted #31 commit `020a280103a5d96ca7ab5e672d3f7826e4153530`.

## Proof

The high LOD from the accepted #31 VRML was extracted into two Blender mesh objects:

- `_H_M_Statue_Atlas_Giant_0` — `X_Male_Statue_Atlas_Globe_01`
- `_H_M_Statue_Atlas_Giant_1` — `X_Male_Statue_Atlas_01`

Blender 4.5.3 LTS authored the proof edit by scaling only the Globe object `1.10x` around its local center. The edited `.blend` is intentionally repo-external:

`D:\temp\cohsourcedev-blender\issue33\atlas-proof-smooth.blend`

The edited high LOD was spliced into the original #31 WRL. LOD1 and LOD2 were retained byte-for-byte from the source WRL. GetVrml compiled the complete WRL successfully with exit code 0. The resulting loose geometry SHA256 was:

`98841E9414AF9965215D5189DADC2E18E2E2A851EDD47EFD6812808B7FBB5769`

The existing #31 loose-override pilot installed that `.geo`, and the existing capture path completed `AtlasHero_Statue_01` with client exit code 0. The runtime trace reported:

```text
GEOMETRYPILOT: ... kind=loose ... verts=13824 tris=4608
TEXTUREPILOT: bind=X_Male_Statue_Atlas_Globe_01 ... submesh=0
TEXTUREPILOT: bind=X_Male_Statue_Atlas_01 ... submesh=1
lodCount=3
```

Proof image: [AtlasHero_Statue_01.blender-proof.jpg](evidence/issue33-atlas-statue-blender-v1/AtlasHero_Statue_01.blender-proof.jpg)

The accepted #31 same-path control remains available as the before image: [loose-roundtrip.jpg](evidence/issue31-atlas-statue-geometry-v1/phase-a/loose-roundtrip.jpg). The proof image shows the globe enlarged around its own center while the original body/globe material identity remains attached.

## Bridge

The reusable narrow bridge is [agent/atlas-blender-bridge.py](../agent/atlas-blender-bridge.py). It supports only the deterministic #31 two-Shape Atlas high-LOD subset:

```text
py -3 agent/atlas-blender-bridge.py extract --wrl <source.wrl> --obj <atlas-high.obj>
blender.exe --background --python agent/atlas-blender-bridge.py -- author --obj <atlas-high.obj> --blend <atlas.blend> --edited-obj <atlas-edited.obj>
py -3 agent/atlas-blender-bridge.py splice --wrl <source.wrl> --edited-obj <atlas-edited.obj> --out <edited.wrl>
```

It is not a general DCC interchange layer and does not regenerate LOD1/LOD2.

## Cleanup

After capture, the existing pilot restored stock-by-default state. The loose Atlas `.geo` target is absent and the packed source remains unchanged. No broad Atlas regression suite was run.
