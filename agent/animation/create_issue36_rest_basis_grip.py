"""Derive a bounded left-hand grip from the accepted Issue 36 swing clip.

The accepted RestBasis full clip already owns the continuing ASCEND pose.  This
authoring step keeps that clip as the source of truth and changes only the
target's left ring-finger and thumb channels.  The source Mixamo finger
channels are used as the grip reference, but they are applied only in the
authored ASCEND window and are blended in/out at the window edges.

The input blend and source FBX are never opened for writing.  The output blend
is a separate derived asset, and the report records the exact source hashes,
finger mapping, frame profile, and rotation-only proof.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Quaternion, Vector


ROOT = Path(__file__).resolve().parents[2]
ANIMATION_DIR = ROOT / "agent" / "animation"
if str(ANIMATION_DIR) not in sys.path:
    sys.path.insert(0, str(ANIMATION_DIR))

import diagnose_issue36_orientation as diagnostic  # noqa: E402
import prove_mixamo_anatomical_pose as proof  # noqa: E402
from create_blender_canary import load_rig  # noqa: E402


ASSET_NAME = "COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL_GRIP"
BASE_ASSET_NAME = "COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL"
SOURCE_ACTION = "Armature|mixamo.com|Layer0"
FRAME_START = 1
FRAME_END = 60
GRIP_WINDOW = tuple(range(16, 31))
GRIP_BONES = ("F1_L", "F2_L", "T1_L", "T2_L", "T3_L")

# The target rig has one left finger chain (F2_L -> F1_L) and one three-bone
# thumb chain (T3_L -> T2_L -> T1_L).  These are the only target channels that
# the derived asset is allowed to alter.
SOURCE_FINGER_MAP = {
    "F2_L": "mixamorig:LeftHandRing1",
    "F1_L": "mixamorig:LeftHandRing2",
    "T3_L": "mixamorig:LeftHandThumb1",
    "T2_L": "mixamorig:LeftHandThumb2",
    "T1_L": "mixamorig:LeftHandThumb3",
}

# A short linear closure envelope keeps the derived channels continuous with
# the unchanged full clip at both ends of the ASCEND move.  Frames 16 and 30
# remain byte-equivalent in pose to the base clip; the authored frame range is
# still exactly 1..60.
CLOSURE_WEIGHTS = {
    16: 0.00,
    17: 0.30,
    18: 0.60,
    19: 0.85,
    20: 1.00,
    21: 1.00,
    22: 1.00,
    23: 1.00,
    24: 1.00,
    25: 1.00,
    26: 1.00,
    27: 0.85,
    28: 0.60,
    29: 0.30,
    30: 0.00,
}

# Blender stores pose channels as float32.  The accepted Issue 36 rest-basis
# proof observes up to 0.055952 degrees of harmless representation error when
# the same action is evaluated at a later frame, so keep the pre-export gate
# at the established 0.1-degree proof tolerance.  The decoded base-vs-grip
# verifier uses a tighter 0.01-degree delta gate for untouched runtime bones.
Q_TOLERANCE_DEGREES = 0.1
CHANNEL_TOLERANCE = 1.0e-7


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--blend", required=True, type=Path)
    parser.add_argument("--source-fbx", required=True, type=Path)
    parser.add_argument("--rig-json", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--base-runtime-sha256", required=True)
    parser.add_argument("--asset-name", default=ASSET_NAME)
    return parser.parse_args(argv)


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def root_relative(path):
    candidate = Path(path).resolve()
    try:
        return candidate.relative_to(ROOT).as_posix()
    except ValueError:
        return candidate.as_posix()


def q_values(q):
    q = q.normalized()
    if q.w < 0.0:
        q.negate()
    return [float(q.x), float(q.y), float(q.z), float(q.w)]


def v_values(value):
    return [float(value.x), float(value.y), float(value.z)]


def q_error_degrees(left, right):
    dot = max(-1.0, min(1.0, abs(left.normalized().dot(right.normalized()))))
    return math.degrees(2.0 * math.acos(dot))


def vector_error(left, right):
    return float((left - right).length)


def matrix_error(left, right):
    return max(
        abs(float(left[row][column]) - float(right[row][column]))
        for row in range(4)
        for column in range(4)
    )


def slerp(left, right, factor):
    """Spherical interpolation without depending on Blender API version."""
    left = left.normalized()
    right = right.normalized()
    dot = left.dot(right)
    if dot < 0.0:
        right = -right
        dot = -dot
    dot = max(-1.0, min(1.0, dot))
    if dot > 1.0 - 1.0e-7:
        result = Quaternion(
            (
                left.w + factor * (right.w - left.w),
                left.x + factor * (right.x - left.x),
                left.y + factor * (right.y - left.y),
                left.z + factor * (right.z - left.z),
            )
        )
        return result.normalized()
    theta = math.acos(dot)
    sine = math.sin(theta)
    left_weight = math.sin((1.0 - factor) * theta) / sine
    right_weight = math.sin(factor * theta) / sine
    return Quaternion(
        (
            left.w * left_weight + right.w * right_weight,
            left.x * left_weight + right.x * right_weight,
            left.y * left_weight + right.y * right_weight,
            left.z * left_weight + right.z * right_weight,
        )
    ).normalized()


def integral_frame_range(action):
    start_value, end_value = action.frame_range
    start = int(round(start_value))
    end = int(round(end_value))
    if abs(start_value - start) > 1.0e-6 or abs(end_value - end) > 1.0e-6:
        raise RuntimeError(f"Action has a non-integral frame range: {start_value}..{end_value}")
    return start, end


def action_fcurves(action):
    fcurves = list(getattr(action, "fcurves", []))
    if not fcurves and hasattr(action, "layers"):
        for layer in action.layers:
            for strip in layer.strips:
                for channelbag in strip.channelbags:
                    fcurves.extend(channelbag.fcurves)
    return fcurves


def curve_snapshot(action):
    return {
        (curve.data_path, curve.array_index): [
            (float(point.co.x), float(point.co.y))
            for point in curve.keyframe_points
        ]
        for curve in action_fcurves(action)
    }


def pose_snapshot(target):
    return {
        pose_bone.name: {
            "rotation": pose_bone.rotation_quaternion.copy(),
            "location": pose_bone.location.copy(),
            "scale": pose_bone.scale.copy(),
        }
        for pose_bone in target.pose.bones
    }


def validate_input(source, target, args):
    if args.asset_name != ASSET_NAME:
        raise SystemExit(f"The derived grip asset name is fixed to {ASSET_NAME}.")
    if args.source_fbx.name.lower() != "swinging.fbx":
        raise SystemExit("--source-fbx must be swinginganimations/Swinging.fbx")
    if not args.source_fbx.is_file():
        raise SystemExit(f"Source FBX not found: {args.source_fbx}")
    if not args.rig_json.is_file():
        raise SystemExit(f"Rig JSON not found: {args.rig_json}")
    if source.animation_data is None or source.animation_data.action is None:
        raise RuntimeError("Mixamo source has no active action")
    if source.animation_data.action.name != SOURCE_ACTION:
        raise RuntimeError(
            f"Expected source action {SOURCE_ACTION}, got {source.animation_data.action.name}"
        )
    if target.animation_data is None or target.animation_data.action is None:
        raise RuntimeError("Accepted target has no active full swing action")
    if not target.animation_data.action.name.startswith(BASE_ASSET_NAME):
        raise RuntimeError(
            f"Expected accepted target action based on {BASE_ASSET_NAME}, "
            f"got {target.animation_data.action.name}"
        )
    action_start, action_end = integral_frame_range(target.animation_data.action)
    if (action_start, action_end) != (FRAME_START, FRAME_END):
        raise RuntimeError(
            f"Accepted full action must cover {FRAME_START}..{FRAME_END}, "
            f"got {action_start}..{action_end}"
        )
    if bpy.context.scene.render.fps != 30:
        raise RuntimeError(f"Accepted full clip must be 30 fps, got {bpy.context.scene.render.fps}")
    if target.get("coh_export_fk") != "runtime-local-bind-translation":
        raise RuntimeError("Exact target is missing the runtime-local-bind-translation contract")
    if len(target.data.bones) != 68:
        raise RuntimeError(f"Expected the 68-bone Male target, got {len(target.data.bones)}")
    missing_target = sorted(set(GRIP_BONES) - {bone.name for bone in target.data.bones})
    missing_source = sorted(set(SOURCE_FINGER_MAP.values()) - {bone.name for bone in source.data.bones})
    if missing_target:
        raise RuntimeError(f"Target is missing grip bones: {missing_target}")
    if missing_source:
        raise RuntimeError(f"Source is missing mapped finger bones: {missing_source}")
    if set(CLOSURE_WEIGHTS) != set(GRIP_WINDOW):
        raise RuntimeError("Closure profile does not cover exactly frames 16..30")
    report, bones, _ = load_rig(args.rig_json)
    if report.get("boneCount", len(bones)) != 68 or len(bones) != 68:
        raise RuntimeError("Rig JSON is not the expected 68-bone SKEL_READY2 rig")
    return report


def source_world_rotation(obj, bone_name, posed):
    matrix = (
        obj.pose.bones[bone_name].matrix
        if posed
        else obj.data.bones[bone_name].matrix_local
    )
    return (obj.matrix_world.to_quaternion() @ matrix.to_3x3().to_quaternion()).normalized()


def target_runtime_local_for_source_finger(source, target, target_name, source_name):
    source_rest = source_world_rotation(source, source_name, posed=False)
    source_pose = source_world_rotation(source, source_name, posed=True)
    source_delta = (source_pose @ source_rest.inverted()).normalized()
    target_rest = source_world_rotation(target, target_name, posed=False)
    desired_source = (source_delta @ target_rest).normalized()
    desired_game_world = proof.source_quat_to_game(desired_source).inverted().normalized()
    target_bone = target.data.bones[target_name]
    parent_world = (
        proof.current_coh_world_rotation(target, target_bone.parent.name)
        if target_bone.parent is not None
        else Quaternion((1.0, 0.0, 0.0, 0.0))
    )
    desired_local = (desired_game_world @ parent_world.inverted()).normalized()
    return {
        "sourceRest": source_rest,
        "sourcePose": source_pose,
        "sourceDelta": source_delta,
        "targetRest": target_rest,
        "desiredSource": desired_source,
        "desiredGameWorld": desired_game_world,
        "desiredLocal": desired_local,
    }


def topological_grip_order(target):
    return sorted(
        GRIP_BONES,
        key=lambda name: sum(
            1
            for _ in iter_parent_names(target.data.bones[name])
        ),
    )


def iter_parent_names(bone):
    while bone.parent is not None:
        yield bone.parent.name
        bone = bone.parent


def apply_grip_frame(source, target, base_values, frame, weight):
    desired = {}
    # F2/T3 precede their child channels in the native target hierarchy.  The
    # parent world quaternion therefore includes the derived parent grip before
    # the child local channel is solved.
    for target_name in topological_grip_order(target):
        source_name = SOURCE_FINGER_MAP[target_name]
        desired[target_name] = target_runtime_local_for_source_finger(
            source, target, target_name, source_name,
        )
        base_rotation = base_values[target_name]["rotation"]
        target_rotation = desired[target_name]["desiredLocal"]
        pose_bone = target.pose.bones[target_name]
        pose_bone.rotation_mode = "QUATERNION"
        pose_bone.rotation_quaternion = slerp(base_rotation, target_rotation, weight)
        pose_bone.keyframe_insert(
            data_path="rotation_quaternion", frame=frame, group=target_name,
        )
        bpy.context.view_layer.update()
    return desired


def validate_derived_pose(target, base_by_frame, desired_by_frame, object_matrix):
    max_non_grip_rotation = 0.0
    max_non_grip_location = 0.0
    max_non_grip_scale = 0.0
    max_grip_location = 0.0
    max_grip_scale = 0.0
    max_object_transform = 0.0
    max_grip_rotation_error = 0.0
    changed_bones = set()
    changed_frames = set()

    for frame in range(FRAME_START, FRAME_END + 1):
        bpy.context.scene.frame_set(frame)
        bpy.context.view_layer.update()
        current = pose_snapshot(target)
        base = base_by_frame[frame]
        for name, values in current.items():
            base_values = base[name]
            rotation_delta = q_error_degrees(values["rotation"], base_values["rotation"])
            location_delta = vector_error(values["location"], base_values["location"])
            scale_delta = vector_error(values["scale"], base_values["scale"])
            max_non_grip_location = max(max_non_grip_location, location_delta)
            max_non_grip_scale = max(max_non_grip_scale, scale_delta)
            if name in GRIP_BONES:
                max_grip_location = max(max_grip_location, location_delta)
                max_grip_scale = max(max_grip_scale, scale_delta)
                weight = CLOSURE_WEIGHTS.get(frame, 0.0)
                expected = base_values["rotation"]
                if weight > 0.0:
                    expected = slerp(
                        expected,
                        desired_by_frame[frame][name]["desiredLocal"],
                        weight,
                    )
                expected_error = q_error_degrees(values["rotation"], expected)
                max_grip_rotation_error = max(max_grip_rotation_error, expected_error)
                if expected_error > Q_TOLERANCE_DEGREES:
                    raise RuntimeError(
                        f"Grip rotation mismatch frame={frame} bone={name} "
                        f"errorDegrees={expected_error:.9g}"
                    )
                if rotation_delta > Q_TOLERANCE_DEGREES:
                    changed_bones.add(name)
                    changed_frames.add(frame)
            else:
                max_non_grip_rotation = max(max_non_grip_rotation, rotation_delta)
                if rotation_delta > Q_TOLERANCE_DEGREES:
                    raise RuntimeError(
                        f"Non-grip rotation changed frame={frame} bone={name} "
                        f"errorDegrees={rotation_delta:.9g}"
                    )
            if location_delta > CHANNEL_TOLERANCE:
                raise RuntimeError(
                    f"Pose location changed frame={frame} bone={name} "
                    f"error={location_delta:.9g}"
                )
            if scale_delta > CHANNEL_TOLERANCE:
                raise RuntimeError(
                    f"Pose scale changed frame={frame} bone={name} "
                    f"error={scale_delta:.9g}"
                )
        max_object_transform = max(max_object_transform, matrix_error(target.matrix_world, object_matrix))

    if max_object_transform > CHANNEL_TOLERANCE:
        raise RuntimeError(f"Target object transform changed: {max_object_transform:.9g}")
    expected_frames = sorted(
        frame for frame, weight in CLOSURE_WEIGHTS.items()
        if weight > 0.0 and any(
            q_error_degrees(
                base_by_frame[frame][name]["rotation"],
                desired_by_frame[frame][name]["desiredLocal"],
            ) > Q_TOLERANCE_DEGREES
            for name in GRIP_BONES
        )
    )
    if not changed_bones:
        raise RuntimeError("Grip derivation produced no changed finger/thumb channels")
    if not changed_frames:
        raise RuntimeError("Grip derivation produced no changed authored frames")
    return {
        "pass": True,
        "maxNonGripRotationDeltaDegrees": max_non_grip_rotation,
        "maxNonGripLocationDelta": max_non_grip_location,
        "maxNonGripScaleDelta": max_non_grip_scale,
        "maxGripRotationExpectationErrorDegrees": max_grip_rotation_error,
        "maxGripLocationDelta": max_grip_location,
        "maxGripScaleDelta": max_grip_scale,
        "maxObjectTransformDelta": max_object_transform,
        "changedBones": sorted(changed_bones),
        "changedFrames": sorted(changed_frames),
        "expectedChangedFrames": expected_frames,
        "allowedBones": list(GRIP_BONES),
        "allowedFrameRange": [GRIP_WINDOW[0], GRIP_WINDOW[-1]],
        "nonGripTransformsUnchanged": True,
        "rootTranslationUnchanged": max_object_transform <= CHANNEL_TOLERANCE,
    }


def validate_curve_scope(base_curves, derived_curves):
    allowed_paths = {
        (f'pose.bones["{name}"].rotation_quaternion', index)
        for name in GRIP_BONES
        for index in range(4)
    }
    if set(base_curves) != set(derived_curves):
        raise RuntimeError("Derived grip changed the action F-curve set")
    changed = []
    for key in sorted(base_curves):
        if base_curves[key] != derived_curves[key]:
            if key not in allowed_paths:
                raise RuntimeError(f"Unexpected action curve change: {key}")
            changed.append(key)
    if not changed:
        raise RuntimeError("Derived grip did not change any allowed action curve")
    return [
        {"dataPath": key[0], "arrayIndex": key[1]}
        for key in changed
    ]


def main():
    args = parse_args()
    args.blend = args.blend.resolve()
    args.source_fbx = args.source_fbx.resolve()
    args.rig_json = args.rig_json.resolve()
    args.output_dir = args.output_dir.resolve()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    if not args.blend.is_file():
        raise SystemExit(f"Accepted blend not found: {args.blend}")
    if len(args.base_runtime_sha256) != 64:
        raise SystemExit("--base-runtime-sha256 must be a SHA-256 hex digest")

    bpy.ops.wm.open_mainfile(filepath=str(args.blend))
    source = bpy.data.objects.get("Mixamo_Source_Armature")
    target = bpy.data.objects.get("CoH_Male_Exact_Export_Rig")
    if source is None or target is None:
        raise RuntimeError("Accepted blend is missing the source or exact Male target")
    rig_report = validate_input(source, target, args)

    scene = bpy.context.scene
    object_matrix = target.matrix_world.copy()
    action = target.animation_data.action
    base_curves = curve_snapshot(action)
    base_by_frame = {}
    for frame in range(FRAME_START, FRAME_END + 1):
        scene.frame_set(frame)
        bpy.context.view_layer.update()
        base_by_frame[frame] = pose_snapshot(target)

    desired_by_frame = {}
    for frame in GRIP_WINDOW:
        scene.frame_set(frame)
        bpy.context.view_layer.update()
        desired_by_frame[frame] = apply_grip_frame(
            source, target, base_by_frame[frame], frame, CLOSURE_WEIGHTS[frame],
        )

    target.animation_data.action.name = f"{ASSET_NAME}_ACTION"
    scene.frame_start = FRAME_START
    scene.frame_end = FRAME_END
    scene.render.fps = 30
    proof_report = validate_derived_pose(target, base_by_frame, desired_by_frame, object_matrix)
    derived_curves = curve_snapshot(target.animation_data.action)
    changed_curves = validate_curve_scope(base_curves, derived_curves)

    output_blend = args.output_dir / f"{args.asset_name}.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(output_blend))
    output_report = args.output_dir / "rest-basis-grip.math.json"
    source_hash = sha256(args.source_fbx)
    report_out = {
        "tool": "agent/animation/create_issue36_rest_basis_grip.py",
        "blender": bpy.app.version_string,
        "source": {
            "fbx": root_relative(args.source_fbx),
            "fbxSha256": source_hash,
            "baseBlend": root_relative(args.blend),
            "baseBlendSha256": sha256(args.blend),
            "baseRuntimeAsset": f"MALE/{BASE_ASSET_NAME}",
            "baseRuntimeSha256": args.base_runtime_sha256.lower(),
            "rigJson": root_relative(args.rig_json),
            "armature": source.name,
            "action": SOURCE_ACTION,
            "fps": 30,
            "sourceFingerMap": SOURCE_FINGER_MAP,
        },
        "target": {
            "armature": target.name,
            "asset": f"MALE/{args.asset_name}",
            "baseAsset": f"MALE/{BASE_ASSET_NAME}",
            "action": target.animation_data.action.name,
            "boneCount": len(target.data.bones),
            "authoredFrameRange": [FRAME_START, FRAME_END],
            "authoredFrames": FRAME_END - FRAME_START + 1,
            "runtimeFkContract": target.get("coh_export_fk"),
            "referenceRigAnimation": rig_report.get("animation"),
        },
        "modification": {
            "bones": list(GRIP_BONES),
            "sourceBones": SOURCE_FINGER_MAP,
            "authoredFrameRange": [GRIP_WINDOW[0], GRIP_WINDOW[-1]],
            "closureWeights": {str(frame): CLOSURE_WEIGHTS[frame] for frame in GRIP_WINDOW},
            "method": "slerp accepted runtime-local channel toward source-finger rest-basis target channel",
            "rotationComposition": "baseRuntimeLocal.slerp(sourceFingerRestBasisRuntimeLocal, closureWeight)",
            "translationPolicy": "fixed bind translations; zero pose-bone location; unit pose-bone scale",
            "nonFingerChannelsWritten": False,
            "rootTranslationWritten": False,
            "changedActionCurves": changed_curves,
        },
        "proof": proof_report,
        "sourceSafety": {
            "sourceFbxWritten": False,
            "baseBlendWritten": False,
            "productionAssetWritten": False,
            "productionSequencerWritten": False,
            "webSwingPhysicsWritten": False,
        },
    }
    output_report.write_text(json.dumps(report_out, indent=2) + "\n", encoding="utf-8")
    print("ISSUE36_REST_BASIS_GRIP_CREATED " + json.dumps({
        "asset": f"MALE/{args.asset_name}",
        "blend": str(output_blend),
        "report": str(output_report),
        "authoredFrameRange": [FRAME_START, FRAME_END],
        "modifiedFrameRange": [GRIP_WINDOW[0], GRIP_WINDOW[-1]],
        "changedBones": proof_report["changedBones"],
        "changedFrames": proof_report["changedFrames"],
        "maxNonGripRotationDeltaDegrees": proof_report["maxNonGripRotationDeltaDegrees"],
        "maxGripRotationExpectationErrorDegrees": proof_report["maxGripRotationExpectationErrorDegrees"],
    }, sort_keys=True))


if __name__ == "__main__":
    main()
