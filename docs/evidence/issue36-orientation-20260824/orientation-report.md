# Issue 36 orientation/roll localization

Diagnostic-only A/B for `swinginganimations/Swinging.fbx`, action `Armature|mixamo.com|Layer0`, frames 18/20/22.

A is the current direction-plus-plane diagnostic path. B is an explicit semantic rest-basis transfer: it applies the evaluated source pose delta to the exact CoH Male target rest basis. No production animation, exporter, runtime asset, or Web Swing code is changed by this tool.

The old source→control pass checks joint positions and segment directions only. It does not compare a full orthonormal basis, axial roll, or sign continuity; therefore `pass=true` can accept a corkscrewed pose.

## Math

For each mapped source/target bone, B uses `sourceDelta = sourcePoseWorld * inverse(sourceRestWorld)` and `targetPoseWorld = sourceDelta * targetRestWorld`, then converts the desired target world matrix back through Blender's parent/rest relation with zero local translation and unit scale.

`direction` compares primary Y axes. `roll-only` aligns the two primary axes with the shortest arc and measures the remaining signed angle between secondary Z axes. `basis` is the complete quaternion angle. Cells below are `direction / roll-only / full-basis` degrees; the complete X/Y/Z basis vectors, quaternions, handedness, and source pose deltas are in `orientation-report.json`.

## First-divergence summary

### source→control

| frame | first basis error > 1° | first roll error > 1° | max direction | max roll-only | max basis |
|---:|---|---|---:|---:|---:|
| 18 | head | shoulder_r | 12.888° | 168.800° | 168.800° |
| 20 | head | shoulder_r | 12.888° | 150.186° | 150.186° |
| 22 | head | shoulder_r | 12.888° | 150.009° | 150.009° |

### control→CoH current vs rest-basis reference

| frame | first basis error > 1° | first roll error > 1° | max direction | max roll-only | max basis |
|---:|---|---|---:|---:|---:|
| 18 | HIPS | HIPS | 26.893° | 179.904° | 179.905° |
| 20 | HIPS | HIPS | 26.893° | 179.904° | 179.905° |
| 22 | HIPS | HIPS | 26.893° | 179.904° | 179.905° |

## Boundary reading

The first axial-roll failure in the source→control traversal is the right shoulder (`shoulder_r`, mapped to `COL_R`); its segment direction remains 0° while its roll error is already large. At the downstream control→CoH boundary, the first target bone with a full-basis error over 1° is `HIPS`, with an approximately 90° axial-roll error in all three frames. The `head` basis row appears earlier in the source list because the terminal `head_end` geometry is not the same as the target's `CRANIUM` terminal; it is recorded separately and is not the first shoulder/limb roll cause.

The B rows are zero (within floating-point/render-rig rounding), while A retains repeated 90°/180° axial errors. This localizes the primary defect to rest-basis orientation conversion, not a source frame-map or joint-position failure.

## Source→control basis metrics

These rows compare the evaluated control basis against the source pose delta expressed through the control rest basis. They are separate from the downstream target test.

| source semantic | source bone | control bone | frame 18 d/r/b | frame 20 d/r/b | frame 22 d/r/b |
|---|---|---|---:|---:|---:|
| `hips` | `mixamorig:Hips` | `Ctrl_Hips` | 0.00°/0.00°/0.00° | 0.00°/0.00°/0.00° | 0.00°/0.00°/0.00° |
| `spine` | `mixamorig:Spine` | `Ctrl_Spine` | 0.00°/0.00°/0.00° | 0.00°/0.00°/0.04° | 0.00°/0.00°/0.04° |
| `spine1` | `mixamorig:Spine1` | `Ctrl_Spine1` | 0.00°/0.00°/0.00° | 0.00°/0.00°/0.00° | 0.00°/0.00°/0.00° |
| `spine2` | `mixamorig:Spine2` | `Ctrl_Spine2` | 0.00°/0.00°/0.00° | 0.00°/0.00°/0.00° | 0.00°/0.00°/0.00° |
| `neck` | `mixamorig:Neck` | `Ctrl_Neck` | 0.00°/0.00°/0.00° | 0.00°/0.00°/0.00° | 0.00°/0.00°/0.00° |
| `head` | `mixamorig:Head` | `Ctrl_Head` | 12.89°/0.00°/12.89° | 12.89°/0.00°/12.89° | 12.89°/0.00°/12.89° |
| `shoulder_r` | `mixamorig:RightShoulder` | `Ctrl_Shoulder_Right` | 0.00°/84.52°/84.52° | 0.00°/91.23°/91.23° | 0.00°/96.25°/96.25° |
| `arm_r` | `mixamorig:RightArm` | `Ctrl_Arm_FK_Right` | 0.00°/90.00°/90.00° | 0.00°/90.00°/90.00° | 0.00°/90.00°/90.00° |
| `forearm_r` | `mixamorig:RightForeArm` | `Ctrl_ForeArm_FK_Right` | 0.00°/90.00°/90.00° | 0.00°/90.00°/90.00° | 0.00°/90.00°/90.00° |
| `hand_r` | `mixamorig:RightHand` | `Ctrl_Hand_FK_Right` | 0.00°/23.16°/23.16° | 0.00°/13.67°/13.67° | 0.00°/3.96°/3.96° |
| `shoulder_l` | `mixamorig:LeftShoulder` | `Ctrl_Shoulder_Left` | 0.00°/50.44°/50.44° | 0.00°/68.94°/68.94° | 0.00°/70.17°/70.17° |
| `arm_l` | `mixamorig:LeftArm` | `Ctrl_Arm_FK_Left` | 0.00°/90.00°/90.00° | 0.00°/90.00°/90.00° | 0.00°/90.00°/90.00° |
| `forearm_l` | `mixamorig:LeftForeArm` | `Ctrl_ForeArm_FK_Left` | 0.00°/90.00°/90.00° | 0.00°/90.00°/90.00° | 0.00°/90.00°/90.00° |
| `hand_l` | `mixamorig:LeftHand` | `Ctrl_Hand_FK_Left` | 0.00°/168.80°/168.80° | 0.00°/150.19°/150.19° | 0.00°/150.01°/150.01° |
| `thigh_r` | `mixamorig:RightUpLeg` | `Ctrl_UpLeg_FK_Right` | 0.00°/80.54°/80.54° | 0.00°/80.08°/80.08° | 0.00°/78.89°/78.89° |
| `shin_r` | `mixamorig:RightLeg` | `Ctrl_Leg_FK_Right` | 0.00°/77.26°/77.26° | 0.00°/75.59°/75.59° | 0.00°/73.09°/73.09° |
| `foot_r` | `mixamorig:RightFoot` | `Ctrl_Foot_FK_Right` | 0.00°/92.57°/92.57° | 0.00°/90.11°/90.11° | 0.00°/85.42°/85.42° |
| `thigh_l` | `mixamorig:LeftUpLeg` | `Ctrl_UpLeg_FK_Left` | 0.00°/100.70°/100.70° | 0.00°/101.26°/101.26° | 0.00°/101.45°/101.45° |
| `shin_l` | `mixamorig:LeftLeg` | `Ctrl_Leg_FK_Left` | 0.00°/81.71°/81.71° | 0.00°/83.71°/83.71° | 0.00°/84.32°/84.32° |
| `foot_l` | `mixamorig:LeftFoot` | `Ctrl_Foot_FK_Left` | 0.00°/59.62°/59.62° | 0.00°/53.99°/53.99° | 0.00°/50.69°/50.69° |

## Control→CoH target basis metrics

A is the current production-like direction/plane result in the source/ANIMX comparison frame. B is the rest-basis reference driven from the same control pose delta. The runtime-convention clones are also recorded in the JSON and used for the visual renders.

| target | control semantic | A18 d/r/b | B18 d/r/b | A20 d/r/b | B20 d/r/b | A22 d/r/b | B22 d/r/b |
|---|---|---:|---:|---:|---:|---:|---:|
| `HIPS` | `hips` | 5.98°/90.00°/90.16° | 0.00°/0.00°/0.00° | 5.98°/90.00°/90.16° | 0.00°/0.00°/0.00° | 5.98°/90.00°/90.16° | 0.00°/0.00°/0.00° |
| `WAIST` | `spine` | 11.15°/0.95°/11.19° | 0.00°/0.00°/0.00° | 11.15°/0.56°/11.16° | 0.00°/0.00°/0.00° | 11.15°/0.25°/11.15° | 0.00°/0.00°/0.00° |
| `CHEST` | `spine2` | 0.35°/90.00°/90.00° | 0.00°/0.00°/0.00° | 0.35°/90.00°/90.00° | 0.00°/0.00°/0.00° | 0.35°/90.00°/90.00° | 0.00°/0.00°/0.00° |
| `NECK` | `neck` | 1.49°/90.00°/90.01° | 0.00°/0.00°/0.00° | 1.49°/90.00°/90.01° | 0.00°/0.00°/0.00° | 1.49°/90.00°/90.01° | 0.00°/0.00°/0.00° |
| `HEAD` | `head` | 26.89°/179.21°/179.24° | 0.00°/0.00°/0.00° | 26.89°/179.21°/179.24° | 0.00°/0.00°/0.04° | 26.89°/179.21°/179.24° | 0.00°/0.00°/0.00° |
| `COL_R` | `shoulder_r` | 16.92°/91.03°/92.25° | 0.00°/0.00°/0.00° | 16.92°/91.03°/92.25° | 0.00°/0.00°/0.00° | 16.92°/91.03°/92.25° | 0.00°/0.00°/0.00° |
| `UARMR` | `arm_r` | 2.17°/90.00°/90.02° | 0.00°/0.00°/0.00° | 2.17°/90.00°/90.02° | 0.00°/0.00°/0.00° | 2.17°/90.00°/90.02° | 0.00°/0.00°/0.00° |
| `LARMR` | `forearm_r` | 1.30°/90.00°/90.01° | 0.00°/0.00°/0.00° | 1.30°/90.00°/90.01° | 0.00°/0.00°/0.00° | 1.30°/90.00°/90.01° | 0.00°/0.00°/0.00° |
| `HANDR` | `hand_r` | 25.01°/90.49°/93.16° | 0.00°/0.00°/0.00° | 25.01°/90.49°/93.16° | 0.00°/0.00°/0.00° | 25.01°/90.49°/93.16° | 0.00°/0.00°/0.00° |
| `COL_L` | `shoulder_l` | 16.92°/91.03°/92.25° | 0.00°/0.00°/0.00° | 16.92°/91.03°/92.25° | 0.00°/0.00°/0.00° | 16.92°/91.03°/92.25° | 0.00°/0.00°/0.00° |
| `UARML` | `arm_l` | 2.17°/90.00°/90.02° | 0.00°/0.00°/0.00° | 2.17°/90.00°/90.02° | 0.00°/0.00°/0.00° | 2.17°/90.00°/90.02° | 0.00°/0.00°/0.00° |
| `LARML` | `forearm_l` | 1.30°/90.00°/90.01° | 0.00°/0.00°/0.00° | 1.30°/90.00°/90.01° | 0.00°/0.00°/0.00° | 1.30°/90.00°/90.01° | 0.00°/0.00°/0.04° |
| `HANDL` | `hand_l` | 25.00°/90.49°/93.16° | 0.00°/0.00°/0.00° | 25.00°/90.49°/93.16° | 0.00°/0.00°/0.00° | 25.00°/90.49°/93.16° | 0.00°/0.00°/0.00° |
| `ULEGR` | `thigh_r` | 6.66°/89.84°/90.03° | 0.00°/0.00°/0.00° | 6.66°/89.84°/90.03° | 0.00°/0.00°/0.00° | 6.66°/89.84°/90.03° | 0.00°/0.00°/0.00° |
| `LLEGR` | `shin_r` | 10.17°/179.90°/179.90° | 0.00°/0.00°/0.00° | 10.17°/179.90°/179.90° | 0.00°/0.00°/0.00° | 10.17°/179.90°/179.90° | 0.00°/0.00°/0.00° |
| `FOOTR` | `foot_r` | 3.07°/0.00°/3.07° | 0.00°/0.00°/0.00° | 3.07°/0.00°/3.07° | 0.00°/0.00°/0.00° | 3.07°/0.00°/3.07° | 0.00°/0.00°/0.00° |
| `ULEGL` | `thigh_l` | 7.21°/90.22°/90.44° | 0.00°/0.00°/0.00° | 7.21°/90.22°/90.44° | 0.00°/0.00°/0.00° | 7.21°/90.22°/90.44° | 0.00°/0.00°/0.00° |
| `LLEGL` | `shin_l` | 10.17°/179.90°/179.90° | 0.00°/0.00°/0.00° | 10.17°/179.90°/179.90° | 0.00°/0.00°/0.06° | 10.17°/179.90°/179.90° | 0.00°/0.00°/0.00° |
| `FOOTL` | `foot_l` | 3.07°/0.00°/3.07° | 0.00°/0.00°/0.00° | 3.07°/0.00°/3.07° | 0.00°/0.00°/0.00° | 3.07°/0.00°/3.07° | 0.00°/0.00°/0.00° |
## Torso mapping audit

The current production compression is `Mixamo Hips → HIPS`, `Spine → WAIST`, `Spine2 → CHEST`, `Neck → NECK`, `Head → HEAD`; `Spine1` is not directly consumed by a target channel. The rest-basis table retains both `Spine → WAIST` and `Spine1 → WAIST` so that the skipped source segment remains explicit.

| frame | first current CoH basis divergence > 1° | current CHEST direction | current CHEST roll-only | B CHEST direction | B CHEST roll-only |
|---:|---|---:|---:|---:|---:|
| 18 | HIPS | 0.349° | 90.000° | 0.000° | 0.000° |
| 20 | HIPS | 0.349° | 90.000° | 0.000° | 0.000° |
| 22 | HIPS | 0.349° | 90.000° | 0.000° | 0.000° |

## Rest-basis offsets

| source | target | full offset | primary offset | roll offset after primary align |
|---|---|---:|---:|---:|
| hips | HIPS | 180.000° | 5.976° | 180.000° |
| spine | WAIST | 11.146° | 11.146° | 0.000° |
| spine1 | WAIST | 11.146° | 11.146° | 0.000° |
| spine2 | CHEST | 0.349° | 0.349° | 0.000° |
| neck | NECK | 180.000° | 1.489° | -180.000° |
| head | HEAD | 179.271° | 14.008° | 179.266° |
| shoulder_r | COL_R | 178.980° | 16.925° | -178.968° |
| arm_r | UARMR | 179.998° | 2.172° | 179.998° |
| forearm_r | LARMR | 180.000° | 1.299° | 180.000° |
| shoulder_l | COL_L | 178.980° | 16.924° | 178.969° |
| arm_l | UARML | 179.998° | 2.172° | -179.998° |
| forearm_l | LARML | 180.000° | 1.299° | -180.000° |
| thigh_r | ULEGR | 28.069° | 6.665° | 27.281° |
| shin_r | LLEGR | 179.905° | 10.173° | 179.904° |
| thigh_l | ULEGL | 35.010° | 7.206° | -34.283° |
| shin_l | LLEGL | 179.905° | 10.172° | -179.904° |

## Shared bend-plane reuse

The current solver reuses one bend plane for every bone in an arm or leg chain. The table compares that shared plane with each source bone's evaluated pose Z/roll axis after projection about that bone's primary segment. Large signed differences are the direct roll-loss measurement; this is not a frame-to-frame sign-flip test.

| chain | source bone | 18 signed° / dot | 20 signed° / dot | 22 signed° / dot |
|---|---|---:|---:|---:|
| `arm_r` | `shoulder_r` | +174.5° / -0.995 | -178.8° / -1.000 | -173.8° / -0.994 |
| `arm_r` | `arm_r` | +180.0° / -1.000 | +180.0° / -1.000 | +180.0° / -1.000 |
| `arm_r` | `forearm_r` | +180.0° / -1.000 | +180.0° / -1.000 | +180.0° / -1.000 |
| `arm_r` | `hand_r` | +113.2° / -0.393 | +103.7° / -0.236 | +94.0° / -0.069 |
| `arm_l` | `shoulder_l` | -39.6° / 0.771 | -21.1° / 0.933 | -19.8° / 0.941 |
| `arm_l` | `arm_l` | +0.0° / 1.000 | +0.0° / 1.000 | +0.0° / 1.000 |
| `arm_l` | `forearm_l` | +0.0° / 1.000 | +0.0° / 1.000 | +0.0° / 1.000 |
| `arm_l` | `hand_l` | +101.2° / -0.194 | +119.8° / -0.497 | +120.0° / -0.500 |
| `leg_r` | `thigh_r` | -80.5° / 0.164 | -80.1° / 0.172 | -78.9° / 0.193 |
| `leg_r` | `shin_r` | -77.3° / 0.221 | -75.6° / 0.249 | -73.1° / 0.291 |
| `leg_r` | `foot_r` | -92.6° / -0.045 | -90.1° / -0.002 | -85.4° / 0.080 |
| `leg_l` | `thigh_l` | -100.7° / -0.186 | -101.3° / -0.195 | -101.4° / -0.198 |
| `leg_l` | `shin_l` | -81.7° / 0.144 | -83.7° / 0.109 | -84.3° / 0.099 |
| `leg_l` | `foot_l` | -59.6° / 0.506 | -54.0° / 0.588 | -50.7° / 0.634 |

## Plane/roll sign continuity

A negative reference dot would be a frame-to-frame sign flip. None occurs in the tested 18→20 or 20→22 intervals; sign continuity is therefore not the first cause in this window.

| reference | 18→20 dot | 20→22 dot | flips |
|---|---:|---:|---|
| arm_l_plane | 0.944942 | 0.999790 | none |
| arm_r_plane | 0.989566 | 0.997058 | none |
| control_axis_arm_r | 0.989566 | 0.997058 | none |
| control_axis_leg_r | 0.990577 | 0.997273 | none |
| leg_l_plane | 0.997217 | 0.997523 | none |
| leg_r_plane | 0.990577 | 0.997273 | none |
| pelvis_plane | 0.994399 | 0.993536 | none |
| source_pose_roll_arm_l | 0.944943 | 0.999790 | none |
| source_pose_roll_arm_r | 0.989566 | 0.997057 | none |
| source_pose_roll_foot_l | 0.973273 | 0.989898 | none |
| source_pose_roll_foot_r | 0.981553 | 0.980367 | none |
| source_pose_roll_forearm_l | 0.944943 | 0.999790 | none |
| source_pose_roll_forearm_r | 0.989566 | 0.997057 | none |
| source_pose_roll_hand_l | 0.999329 | 0.998084 | none |
| source_pose_roll_hand_r | 0.793349 | 0.312338 | none |
| source_pose_roll_head | 0.997354 | 0.996344 | none |
| source_pose_roll_hips | 0.994399 | 0.993536 | none |
| source_pose_roll_neck | 0.996122 | 0.994198 | none |
| source_pose_roll_shin_l | 0.967897 | 0.997918 | none |
| source_pose_roll_shin_r | 0.964794 | 0.979559 | none |
| source_pose_roll_shoulder_l | 0.989297 | 0.998463 | none |
| source_pose_roll_shoulder_r | 0.983544 | 0.993771 | none |
| source_pose_roll_spine | 0.995565 | 0.995212 | none |
| source_pose_roll_spine1 | 0.995223 | 0.994459 | none |
| source_pose_roll_spine2 | 0.993403 | 0.994022 | none |
| source_pose_roll_thigh_l | 0.997093 | 0.997241 | none |
| source_pose_roll_thigh_r | 0.990618 | 0.998879 | none |

## A/B visual outputs

Each frame has raw Mixamo, current CoH Male, and rest-basis B renders from the front, three-quarter, and side views. The asymmetric roll fins make axial twist and handedness visible without relying on a skinned mesh.

| frame | raw | current CoH | rest-basis B |
|---:|---|---|---|
| 18 | [front](visual/frame18/raw/front.png), [3/4](visual/frame18/raw/threequarter.png), [side](visual/frame18/raw/side.png) | [front](visual/frame18/current/front.png), [3/4](visual/frame18/current/threequarter.png), [side](visual/frame18/current/side.png) | [front](visual/frame18/rest/front.png), [3/4](visual/frame18/rest/threequarter.png), [side](visual/frame18/rest/side.png) |
| 20 | [front](visual/frame20/raw/front.png), [3/4](visual/frame20/raw/threequarter.png), [side](visual/frame20/raw/side.png) | [front](visual/frame20/current/front.png), [3/4](visual/frame20/current/threequarter.png), [side](visual/frame20/current/side.png) | [front](visual/frame20/rest/front.png), [3/4](visual/frame20/rest/threequarter.png), [side](visual/frame20/rest/side.png) |
| 22 | [front](visual/frame22/raw/front.png), [3/4](visual/frame22/raw/threequarter.png), [side](visual/frame22/raw/side.png) | [front](visual/frame22/current/front.png), [3/4](visual/frame22/current/threequarter.png), [side](visual/frame22/current/side.png) | [front](visual/frame22/rest/front.png), [3/4](visual/frame22/rest/threequarter.png), [side](visual/frame22/rest/side.png) |

## Exporter/runtime scope

The diagnostic applies rotations only: every B application reports zero local translation and unit scale within the assertion tolerance. It does not invoke the exporter or change runtime assets. The A/B mismatch is therefore a source/control/target orientation-construction defect in this proof path; the existing exporter/runtime transport remains outside the localized failure.

| frame | B source→target max location | B source→target max scale | B control→target max location | B control→target max scale |
|---:|---:|---:|---:|---:|
| 18 | 5.62e-07 | 3.58e-07 | 5.46e-07 | 2.98e-07 |
| 20 | 9.83e-07 | 3.58e-07 | 4.92e-07 | 2.98e-07 |
| 22 | 5.46e-07 | 3.58e-07 | 4.95e-07 | 2.38e-07 |
