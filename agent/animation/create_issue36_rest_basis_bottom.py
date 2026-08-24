"""Generate the bounded Issue 36 corrected BOTTOM rest-basis proof.

This is the five-sample extension of the accepted frame-20 canary.  It uses
the exact source-pose delta and target-rest-basis transfer from
``create_issue36_rest_basis_frame20.py`` for source frames 18..22, then writes
those five evaluated CoH runtime-local poses to authored samples 1..5.

The production ``COHSOURCEDEV_RETARGET_SWING_FULL`` blend and runtime asset
are never opened for writing.  The resulting blend and ANIMX are proof
artifacts; the compiled runtime asset has a separate name and is installed
through the private canary include only.
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Quaternion


ROOT = Path(__file__).resolve().parents[2]
ANIMATION_DIR = ROOT / "agent" / "animation"
if str(ANIMATION_DIR) not in sys.path:
    sys.path.insert(0, str(ANIMATION_DIR))

import blender_export_animx as exporter  # noqa: E402
import diagnose_issue36_orientation as diagnostic  # noqa: E402
import prove_mixamo_anatomical_pose as proof  # noqa: E402
from create_blender_canary import build_source_rest, load_rig  # noqa: E402
from create_issue36_rest_basis_frame20 import (  # noqa: E402
    clone_target_for_proof,
    compare_expected_actual,
    expected_pose_records,
    q_values,
    runtime_fk_records,
)


ASSET_NAME = "COHSOURCEDEV_RETARGET_RESTBASIS_BOTTOM"
SOURCE_ACTION = "Armature|mixamo.com|Layer0"
SOURCE_FRAMES = (18, 19, 20, 21, 22)
RUNTIME_AUTHORED_FRAMES = tuple(range(1, len(SOURCE_FRAMES) + 1))
FOCUS_TARGET = tuple(diagnostic.FOCUS_TARGET)
RUNTIME_ROTATION_TOLERANCE_DEGREES = 0.1


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--blend", required=True, type=Path)
    parser.add_argument("--source-fbx", required=True, type=Path)
    parser.add_argument("--rig-json", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--asset-name", default=ASSET_NAME)
    return parser.parse_args(argv)


def q_error_degrees(left, right):
    dot = max(-1.0, min(1.0, abs(left.normalized().dot(right.normalized()))))
    return math.degrees(2.0 * math.acos(dot))


def root_relative(path):
    candidate = Path(path).resolve()
    try:
        return candidate.relative_to(ROOT).as_posix()
    except ValueError:
        return candidate.as_posix()


def quaternion_from_pose_bone(pose_bone):
    """Read the authored runtime-local quaternion without Blender FK evaluation."""
    return pose_bone.matrix_basis.to_3x3().to_quaternion().normalized()


def pose_values(target):
    values = {}
    for pose_bone in target.pose.bones:
        values[pose_bone.name] = {
            "rotation": pose_bone.rotation_quaternion.copy(),
            "location": pose_bone.location.copy(),
            "scale": pose_bone.scale.copy(),
            "runtimeLocalQuaternion": q_values(quaternion_from_pose_bone(pose_bone)),
            "localLocationMagnitude": float(pose_bone.matrix_basis.translation.length),
            "localScaleError": float(
                max(abs(value - 1.0) for value in pose_bone.matrix_basis.to_3x3().to_scale())
            ),
        }
    return values


def keyframe_pose_samples(target, samples):
    action = bpy.data.actions.new(f"{ASSET_NAME}_ACTION")
    target.animation_data_create()
    target.animation_data.action = action
    scene = bpy.context.scene
    scene.frame_start = RUNTIME_AUTHORED_FRAMES[0]
    scene.frame_end = RUNTIME_AUTHORED_FRAMES[-1]
    scene.render.fps = 30

    for authored_frame, sample in zip(RUNTIME_AUTHORED_FRAMES, samples):
        scene.frame_set(authored_frame)
        for pose_bone in target.pose.bones:
            values = sample[pose_bone.name]
            pose_bone.rotation_mode = "QUATERNION"
            pose_bone.rotation_quaternion = values["rotation"]
            pose_bone.location = values["location"]
            pose_bone.scale = values["scale"]
            pose_bone.keyframe_insert(
                data_path="rotation_quaternion", frame=authored_frame, group=pose_bone.name,
            )
            pose_bone.keyframe_insert(
                data_path="location", frame=authored_frame, group=pose_bone.name,
            )
            pose_bone.keyframe_insert(
                data_path="scale", frame=authored_frame, group=pose_bone.name,
            )

    fcurves = list(getattr(action, "fcurves", []))
    if not fcurves and hasattr(action, "layers"):
        for layer in action.layers:
            for strip in layer.strips:
                for channelbag in strip.channelbags:
                    fcurves.extend(channelbag.fcurves)
    for fcurve in fcurves:
        for keyframe in fcurve.keyframe_points:
            keyframe.interpolation = "LINEAR"

    return action


def validate_input(source, target, args):
    if args.asset_name != ASSET_NAME:
        raise SystemExit(f"The proof asset name is fixed to {ASSET_NAME}.")
    if args.source_fbx.name.lower() != "swinging.fbx":
        raise SystemExit("--source-fbx must be swinginganimations/Swinging.fbx")
    if not args.source_fbx.is_file():
        raise SystemExit(f"Source FBX not found: {args.source_fbx}")
    if source.animation_data is None or source.animation_data.action is None:
        raise RuntimeError("Mixamo source has no active action")
    if source.animation_data.action.name != SOURCE_ACTION:
        raise RuntimeError(
            f"Expected source action {SOURCE_ACTION}, got {source.animation_data.action.name}"
        )
    action_start, action_end = source.animation_data.action.frame_range
    if min(SOURCE_FRAMES) < action_start or max(SOURCE_FRAMES) > action_end:
        raise RuntimeError(
            f"Source frames {SOURCE_FRAMES[0]}..{SOURCE_FRAMES[-1]} are outside "
            f"the action range {action_start}..{action_end}"
        )
    if target.get("coh_export_fk") != "runtime-local-bind-translation":
        raise RuntimeError("Exact target is missing the runtime-local-bind-translation contract")


def apply_bottom_rest_basis_runtime_fk(source, target, rest_world, pairs):
    """Apply the accepted frame-20 runtime-local rest-basis transfer."""
    diagnostic.reset_target(target)
    results = {}
    for source_semantic, target_name in diagnostic.topological_target_pairs(target, pairs):
        source_delta, _, _ = diagnostic.semantic_pose_delta(source, source_semantic, "source")
        target_rest = diagnostic.world_rotation(
            target, target.data.bones[target_name].matrix_local,
        )
        desired_source = (source_delta @ target_rest).normalized()
        desired_game = proof.source_quat_to_game(desired_source).inverted().normalized()
        bone = target.data.bones[target_name]
        parent_world = (
            proof.current_coh_world_rotation(target, bone.parent.name)
            if bone.parent is not None else Quaternion((1.0, 0.0, 0.0, 0.0))
        )
        local = (desired_game @ parent_world.inverted()).normalized()
        pose_bone = target.pose.bones[target_name]
        pose_bone.rotation_mode = "QUATERNION"
        pose_bone.location = (0.0, 0.0, 0.0)
        pose_bone.rotation_quaternion = local
        pose_bone.scale = (1.0, 1.0, 1.0)
        bpy.context.view_layer.update()
        actual_game = proof.current_coh_world_rotation(target, target_name)
        world_error = q_error_degrees(desired_game, actual_game)
        # Blender stores pose channels as float32.  The accepted frame-20
        # proof already observes up to 0.039565 degrees at this comparison;
        # use the established 0.1-degree runtime tolerance here rather than
        # rejecting harmless quaternion quantization before export.
        if world_error > RUNTIME_ROTATION_TOLERANCE_DEGREES:
            raise RuntimeError(
                "REST_BASIS_BOTTOM_RUNTIME_FK_WORLD_FAIL "
                + json.dumps({
                    "bone": target_name,
                    "errorDegrees": world_error,
                    "desiredGame": q_values(desired_game),
                    "actualGame": q_values(actual_game),
                    "local": q_values(local),
                    "parent": q_values(parent_world),
                }, sort_keys=True)
            )
        results[target_name] = {
            "sourceSemantic": source_semantic,
            "targetPoseWorldQuaternion": q_values(desired_source),
            "runtimeGameWorldQuaternion": q_values(desired_game),
            "runtimeLocalQuaternion": q_values(local),
            "runtimeWorldErrorDegrees": world_error,
        }
    bpy.context.view_layer.update()
    return results


def main():
    args = parse_args()
    args.blend = args.blend.resolve()
    args.source_fbx = args.source_fbx.resolve()
    args.rig_json = args.rig_json.resolve()
    args.output_dir = args.output_dir.resolve()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    bpy.ops.wm.open_mainfile(filepath=str(args.blend))
    source = bpy.data.objects.get("Mixamo_Source_Armature")
    target = bpy.data.objects.get("CoH_Male_Exact_Export_Rig")
    if source is None or target is None:
        raise RuntimeError("Input proof blend is missing the source or exact Male target")
    validate_input(source, target, args)

    report, bones, by_id = load_rig(args.rig_json)
    source_rest_local, rest_world = build_source_rest(bones, by_id)
    ordered = sorted(bones.values(), key=lambda item: item["id"])
    proof_target = clone_target_for_proof(target)

    frame_reports = []
    pose_samples = []
    for source_frame, authored_frame in zip(SOURCE_FRAMES, RUNTIME_AUTHORED_FRAMES):
        bpy.context.scene.frame_set(source_frame)
        bpy.context.view_layer.update()
        expected = expected_pose_records(source, proof_target, rest_world)
        apply_report = apply_bottom_rest_basis_runtime_fk(
            source,
            proof_target,
            rest_world,
            diagnostic.CONTROL_TO_TARGET_PAIRS,
        )
        bpy.context.scene.frame_set(source_frame)
        bpy.context.view_layer.update()
        source_world = runtime_fk_records(proof_target, ordered, bones, source_rest_local)
        rows = compare_expected_actual(proof_target, rest_world, expected, source_world)
        sample = pose_values(proof_target)
        pose_samples.append(sample)

        max_world_error = max(row["runtimeWorldQuaternionErrorDegrees"] for row in rows)
        max_direction = max(row["directionDeltaDegrees"] for row in rows)
        max_roll = max(row["rollDeltaDegrees"] for row in rows)
        max_basis = max(row["fullBasisDeltaDegrees"] for row in rows)
        max_location = max(value["localLocationMagnitude"] for value in sample.values())
        max_scale = max(value["localScaleError"] for value in sample.values())
        if (
            max_world_error > RUNTIME_ROTATION_TOLERANCE_DEGREES
            or max_direction > RUNTIME_ROTATION_TOLERANCE_DEGREES
            or max_roll > RUNTIME_ROTATION_TOLERANCE_DEGREES
            or max_basis > RUNTIME_ROTATION_TOLERANCE_DEGREES
        ):
            raise RuntimeError(
                "REST_BASIS_BOTTOM_PRE_EXPORT_MISMATCH "
                + json.dumps({
                    "sourceFrame": source_frame,
                    "maxWorldErrorDegrees": max_world_error,
                    "maxDirectionDegrees": max_direction,
                    "maxRollDegrees": max_roll,
                    "maxBasisDegrees": max_basis,
                }, sort_keys=True)
            )
        if max_location > 1.0e-6 or max_scale > 1.0e-6:
            raise RuntimeError(
                "REST_BASIS_BOTTOM_ROTATION_ONLY_FAIL "
                f"sourceFrame={source_frame} location={max_location:.9g} "
                f"scaleError={max_scale:.9g}"
            )

        frame_reports.append({
            "sourceFrame": source_frame,
            "authoredFrame": authored_frame,
            "diagnosticApply": apply_report,
            "preExportProof": {
                "pass": True,
                "maxRuntimeWorldQuaternionErrorDegrees": max_world_error,
                "maxDirectionDeltaDegrees": max_direction,
                "maxRollDeltaDegrees": max_roll,
                "maxFullBasisDeltaDegrees": max_basis,
                "maxLocalLocationMagnitude": max_location,
                "maxLocalScaleError": max_scale,
                "bones": rows,
            },
        })

    action = keyframe_pose_samples(proof_target, pose_samples)
    bpy.context.scene.frame_set(RUNTIME_AUTHORED_FRAMES[0])
    bpy.context.view_layer.update()

    output_blend = args.output_dir / f"{args.asset_name}.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(output_blend))
    output_report = args.output_dir / "rest-basis-bottom.math.json"

    all_pre_export = [frame["preExportProof"] for frame in frame_reports]
    expected_local_by_frame = {
        str(authored_frame): {
            name: sample[name]["runtimeLocalQuaternion"]
            for name in sorted(sample)
        }
        for authored_frame, sample in zip(RUNTIME_AUTHORED_FRAMES, pose_samples)
    }
    max_values = {
        "maxRuntimeWorldQuaternionErrorDegrees": max(
            value["maxRuntimeWorldQuaternionErrorDegrees"] for value in all_pre_export
        ),
        "maxDirectionDeltaDegrees": max(
            value["maxDirectionDeltaDegrees"] for value in all_pre_export
        ),
        "maxRollDeltaDegrees": max(
            value["maxRollDeltaDegrees"] for value in all_pre_export
        ),
        "maxFullBasisDeltaDegrees": max(
            value["maxFullBasisDeltaDegrees"] for value in all_pre_export
        ),
        "maxLocalLocationMagnitude": max(
            value["maxLocalLocationMagnitude"] for value in all_pre_export
        ),
        "maxLocalScaleError": max(value["maxLocalScaleError"] for value in all_pre_export),
    }
    report_out = {
        "tool": "agent/animation/create_issue36_rest_basis_bottom.py",
        "blender": bpy.app.version_string,
        "source": {
            "fbx": root_relative(args.source_fbx),
            "blend": root_relative(args.blend),
            "rigJson": root_relative(args.rig_json),
            "armature": source.name,
            "action": SOURCE_ACTION,
            "sourceFrames": list(SOURCE_FRAMES),
            "fps": 30,
        },
        "target": {
            "armature": proof_target.name,
            "asset": f"MALE/{args.asset_name}",
            "referenceAnimation": report["animation"],
            "boneCount": len(proof_target.data.bones),
            "focusedBones": list(FOCUS_TARGET),
            "action": action.name,
            "authoredFrames": len(RUNTIME_AUTHORED_FRAMES),
            "authoredFrameMap": [
                {"authoredFrame": authored, "sourceFrame": source_frame}
                for authored, source_frame in zip(RUNTIME_AUTHORED_FRAMES, SOURCE_FRAMES)
            ],
            "sourceFrameRange": [SOURCE_FRAMES[0], SOURCE_FRAMES[-1]],
            "staticPose": False,
            "runtimeFkContract": proof_target.get("coh_export_fk"),
        },
        "mapping": {
            "sourceToTarget": diagnostic.TARGET_MAP,
            "formula": "sourceDelta = sourcePoseWorld * inverse(sourceRestWorld); targetPoseWorld = sourceDelta * targetRestWorld",
            "runtimeConvention": "CoH runtime-local game quaternion channels; exporter/runtime FK composes local * parent and converts game frame to ANIMX source frame",
            "translationPolicy": "fixed bind translations; zero pose-bone location; unit pose-bone scale",
        },
        "preExportProof": {
            "pass": True,
            "runtimeLocalRepresentation": "reconstructed through blender_export_animx.runtime_fk_world before ANIMX export",
            "rotationToleranceDegrees": RUNTIME_ROTATION_TOLERANCE_DEGREES,
            **max_values,
            "expectedRuntimeLocalQuaternionByAuthoredFrame": expected_local_by_frame,
            "frames": frame_reports,
        },
        "sourceSafety": {
            "sourceFbxWritten": False,
            "productionAssetWritten": False,
            "productionSequencerWritten": False,
            "webSwingPhysicsWritten": False,
            "ascendWritten": False,
        },
    }
    output_report.write_text(json.dumps(report_out, indent=2) + "\n", encoding="utf-8")
    print("ISSUE36_REST_BASIS_BOTTOM_CREATED " + json.dumps({
        "asset": f"MALE/{args.asset_name}",
        "blend": str(output_blend),
        "report": str(output_report),
        "sourceFrames": list(SOURCE_FRAMES),
        "authoredFrames": list(RUNTIME_AUTHORED_FRAMES),
        "focusedBones": len(FOCUS_TARGET),
        **max_values,
    }, sort_keys=True))


if __name__ == "__main__":
    main()
