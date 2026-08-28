# Issue #34 — Atlas statue sculpt v1

Status: bounded pass complete, visual gate **NOT PASS**.

Branch: `agent/atlas-statue-sculpt-v1`  
Base: accepted #33 commit `42178dc149eb883e22324587def2ab36398550aa`

## What was implemented

The Blender bridge now has a narrow `export` command for an already-edited
Atlas `.blend`. It finds the two existing high-LOD Atlas mesh objects and
exports evaluated Blender `loop_triangles` with explicit per-loop UVs and
normals, so edited quads/ngons are triangulated before the existing splice
step. LOD1 and LOD2 remain sourced from the original WRL.

The final authored file is external to the repository:

`D:\temp\cohsourcedev-blender\issue34\atlas-sculpt-v1-final.blend`

Final high-LOD output: **2,955 vertices / 4,608 triangles** across the two
Atlas material identities. The final retained edit was selected-area upper
chest relaxation plus a controlled front-plane chest form change. A bounded
neck/shoulder attempt was rejected after it exposed a thin runtime seam at the
intersecting source shell boundaries; the globe was restored to normal size.

## Validation

- Existing `.blend` export completed through the bridge.
- Final edited WRL spliced and compiled through GetVrml with exit code 0.
- Loose `.geo` loaded in City of Heroes with `AtlasHero_Statue_01`, modern
  materials/lighting flags, and a clean client exit (exit code 0).
- Final side-by-side: [Accepted #29 Statue Remaster | Blender Sculpt v1](evidence/issue34-atlas-statue-sculpt-v1/Accepted29_vs_BlenderSculptV1.jpg)
- No broad Atlas regression suite was run.

## Visual verdict

**NOT PASS.** The chest-only safe revision survives the pipeline, but it is
not immediately and clearly better at normal screenshot size. The most
promising neck/shoulder edits were blocked by the original mesh's separate,
intersecting shell pieces: moving those boundaries created a visible runtime
seam. The exact accepted #29 generated loose texture set was also not present
in this checkout, so the final runtime capture used the packed statue textures
with the modern lighting flags rather than claiming an exact texture-identical
A/B.

This is the stopping point requested by the issue: the limitation is artistic
and source-topology-specific, not a reason to generalize the DCC pipeline.

Final commit SHA: reported in the issue #34 comment after push.
