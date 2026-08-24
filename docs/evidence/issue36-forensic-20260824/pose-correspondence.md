# Issue 36 — raw Mixamo / Blender Male / CoH runtime correspondence

This is the frame-locked diagnostic requested for the corrected `Swingv3`
Male BOTTOM gate. It compares the raw source action, the production Blender
Male target, and the actual CoH runtime capture at the same authored frames.
It does not change physics, ASCEND, the source FBX, or the production runtime
asset.

## Exact inputs and frame mapping

| Representation | Exact input |
|---|---|
| Raw source | `swinginganimations/Swinging.fbx`, action `Armature|mixamo.com|Layer0` |
| Source timing | 30 fps, authored action frames `1..60`; no offset or resampling was introduced |
| Blender target | `agent/work/issue36-mixamo-full60-20260822/COHSOURCEDEV_RETARGET_POSE_PROOF.blend`, armature `CoH_Male_Exact_Export_Rig`, production full-60 runtime-FK action |
| CoH runtime | `MALE/COHSOURCEDEV_RETARGET_SWING_FULL`, length `60`, runtime frame `0` bind/reference and authored samples `1..60` |
| Runtime identity | `Dummy00009` / `Swingv3`, `TypeGfx=male`, `is_male=1` |

The proven mapping is source frame `N` → Blender target frame `N` → runtime
sample `N`. The diagnostic frames are exactly `18`, `20`, and `22`; no frame
mapping ambiguity remains.

## Visual comparison

Each sheet uses the same Blender display transform and matching front,
three-quarter, and side views for the two diagnostic skeletons. The runtime
column is the existing exact-character capture, cropped and enlarged only for
inspection; its game camera and scale are intentionally preserved.

| Frame | Comparison sheet |
|---:|---|
| 18 | [raw Mixamo → Blender Male → CoH runtime](pose-correspondence/FRAME18_raw-vs-retarget-vs-runtime.jpg) |
| 20 | [raw Mixamo → Blender Male → CoH runtime](pose-correspondence/FRAME20_raw-vs-retarget-vs-runtime.jpg) |
| 22 | [raw Mixamo → Blender Male → CoH runtime](pose-correspondence/FRAME22_raw-vs-retarget-vs-runtime.jpg) |

The raw action is itself a strongly compressed swing tuck, so it is not a
neutral anatomical reference. However, the raw front/three-quarter/side
silhouettes and branch layout are not the same as the more corkscrewed,
tangled Blender Male target seen in the runtime skin. The first visual
divergence cannot be assigned to one isolated joint from stills alone; it is
already present in the torso/hip scaffold and propagates into the shoulder/arm
and leg branches.

## Numeric Blender → ANIMX → runtime check

The production target was extracted at the exact three frames and compared to
the checked-in `full.json`. The exporter contract is rotation-only local FK
with fixed bind translations; ANIMX uses the established game-to-source axis
conversion.

| Frame | Max exporter-local → runtime-local | Max exporter ANIMX world residual | Max runtime-recomposed world residual | First local divergence > 0.1° |
|---:|---:|---:|---:|---|
| 18 | 0.083516283° | 0.000019014° | 0.130850145° | none |
| 20 | 0.074867470° | 0.000022394° | 0.156138179° | none |
| 22 | 0.073373143° | 0.000034659° | 0.147203682° | none |

All focused local channels remain below the established `0.1°` runtime
rotation tolerance, and no first divergent exporter/runtime bone occurs in
these frames. The larger recomposed-world residual is cumulative runtime
quantization across parent chains, not a new per-bone discontinuity. The
focused rows are preserved in the comparator output generated from the
production full-60 blend; the maximum local residuals occur at `COL_L` (18),
`LLEGL` (20), and `LLEGR` (22), all still below tolerance.

Focus hierarchy checked:

```text
HIPS → WAIST → CHEST → NECK → HEAD
CHEST → COL_L → UARML → LARML → HANDL
CHEST → COL_R → UARMR → LARMR → HANDR
HIPS → ULEGL → LLEGL
HIPS → ULEGR → LLEGR
```

## Classification

**Case C — source-to-Male retarget divergence is the first actionable
boundary.** The CoH target is faithfully carried through the existing
exporter, ANIMX, compiler/runtime decode, and `Swingv3` selection path. That
pass does not validate the source-to-Male anatomical transfer: the retarget
construction is where the raw source pose and the tangled runtime target stop
agreeing visually.

This evidence does not yet identify whether the retarget defect is a specific
rest-axis basis, bend-plane choice, handedness convention, or another
source-to-target solve detail. It does rule out a frame offset at `18/20/22`,
a runtime-local quaternion transport failure, a bind-translation mismatch,
and a first divergence in the compiled runtime path. No speculative
production animation fix is justified by this pass.

## Durable diagnostic tooling

| Path | Purpose |
|---|---|
| `agent/animation/render_issue36_production_correspondence.py` | frame-locks and renders clean raw/target skeletons with stale proof proxies hidden and source/target display alignment applied |
| `agent/animation/make_issue36_comparison_sheets.py` | builds the three-frame, three-view comparison sheets |
| `agent/animation/extract_issue36_correspondence.py` | extracts production target local/game/ANIMX rotations |
| `agent/animation/compare_issue36_correspondence.py` | compares target rotations against runtime samples and reports the first local boundary over tolerance |

ASCEND remains untested and held. Physics was not changed.
