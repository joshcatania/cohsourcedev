"""Generate the Issue 36 complete Male frame-20 rest-basis proof.

This is a proof-only authoring step.  It opens the existing production proof
blend, evaluates the exact Mixamo action at source frame 20, and applies the
accepted diagnostic transfer to a separate copy of the exact Male export rig:

    source_delta = source_pose_world * inverse(source_rest_world)
    target_pose_world = source_delta * target_rest_world

The local channels written to the proof rig use the already-proven CoH
runtime-FK convention.  The script records the expected source-frame basis,
the reconstructed source-frame basis obtained from those runtime-local
channels, and the rotation-only channel assertions.  Export, compilation,
and runtime decoding are deliberately separate commands so a failure at any
later boundary cannot be hidden by this generator.

The production Web Swing asset and sequencer are never opened for writing.
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Quaternion, Vector


ROOT = Path(__file__).resolve().parents[2]
ANIMATION_DIR = ROOT / "agent" / "animation"
if str(ANIMATION_DIR) not in sys.path:
    sys.path.insert(0, str(ANIMATION_DIR))

import blender_export_animx as exporter  # noqa: E402
import diagnose_issue36_orientation as diagnostic  # noqa: E402
import prove_mixamo_anatomical_pose as proof  # noqa: E402
from create_blender_canary import build_source_rest, load_rig  # noqa: E402


ASSET_NAME = "COHSOURCEDEV_RETARGET_RESTBASIS_FRAME20"
SOURCE_ACTION = "Armature|mixamo.com|Layer0"
SOURCE_FRAME = 20
FOCUS_TARGET = tuple(diagnostic.FOCUS_TARGET)
RUNTIME_ROTATION_TOLERANCE_DEGREES = 0.1


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--blend", required=True, type=Path)
    parser.add_argument("--source-fbx", required=True, type=Path)
    parser.add_argument("--rig-json", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--source-frame", type=int, default=SOURCE_FRAME)
    parser.add_argument("--frames", type=int, default=5)
    parser.add_argument("--asset-name", default=ASSET_NAME)
    return parser.parse_args(argv)


def q_values(q):
    q = q.normalized()
    if q.w < 0.0:
        q.negate()
    return [float(q.x), float(q.y), float(q.z), float(q.w)]


def q_from_values(values):
    q = Quaternion((values[3], values[0], values[1], values[2]))
    q.normalize()
    return q


def q_error_degrees(left, right):
    dot = max(-1.0, min(1.0, abs(left.normalized().dot(right.normalized()))))
    return math.degrees(2.0 * math.acos(dot))


def root_relative(path):
    candidate = Path(path).resolve()
    try:
        return candidate.relative_to(ROOT).as_posix()
    except ValueError:
        return candidate.as_posix()


def clone_target_for_proof(target):
    """Keep the old target available while making the proof target canonical."""
    target.name = "CoH_Male_Current_Retarget_Rig"
    target.data.name = "CoH_Male_Current_Retarget_Rig"
    target.hide_render = True
    target.hide_viewport = False

    proof_target = target.copy()
    proof_target.data = target.data.copy()
    proof_target.name = "CoH_Male_Exact_Export_Rig"
    proof_target.data.name = "CoH_Male_Exact_Export_Rig"
    proof_target.animation_data_clear()
    proof_target.hide_render = True
    proof_target.hide_viewport = False
    bpy.context.collection.objects.link(proof_target)
    return proof_target


def keyframe_static_pose(target, frames):
    """Key every rig bone with the evaluated rotation-only proof pose."""
    values = {
        pose_bone.name: (
            pose_bone.rotation_quaternion.copy(),
            pose_bone.location.copy(),
            pose_bone.scale.copy(),
        )
        for pose_bone in target.pose.bones
    }

    action = bpy.data.actions.new(f"{ASSET_NAME}_ACTION")
    target.animation_data_create()
    target.animation_data.action = action
    scene = bpy.context.scene
    scene.frame_start = 1
    scene.frame_end = frames
    scene.render.fps = 30

    for frame in range(1, frames + 1):
        for pose_bone in target.pose.bones:
            rotation, location, scale = values[pose_bone.name]
            pose_bone.rotation_mode = "QUATERNION"
            pose_bone.rotation_quaternion = rotation
            pose_bone.location = location
            pose_bone.scale = scale
            pose_bone.keyframe_insert(
                data_path="rotation_quaternion", frame=frame, group=pose_bone.name,
            )
            pose_bone.keyframe_insert(
                data_path="location", frame=frame, group=pose_bone.name,
            )
            pose_bone.keyframe_insert(
                data_path="scale", frame=frame, group=pose_bone.name,
            )

    # Constant interpolation makes the frozen nature explicit even if a
    # human later scrubs between the authored samples in Blender.
    fcurves = list(getattr(action, "fcurves", []))
    if not fcurves and hasattr(action, "layers"):
        for layer in action.layers:
            for strip in layer.strips:
                for channelbag in strip.channelbags:
                    fcurves.extend(channelbag.fcurves)
    for fcurve in fcurves:
        for keyframe in fcurve.keyframe_points:
            keyframe.interpolation = "CONSTANT"

    scene.frame_set(SOURCE_FRAME)
    bpy.context.view_layer.update()
    return action, values


def target_basis_record(target, name, orientation, rest_world):
    rest_segment = diagnostic.target_rest_segment(rest_world, target, name).normalized()
    primary = orientation @ rest_segment
    return diagnostic.basis_record(
        diagnostic.basis_from_primary_and_orientation(primary, orientation),
    )


def expected_pose_records(source, target, rest_world):
    """Build the diagnostic B targets independently of the apply report."""
    expected = {}
    for source_semantic, target_name in diagnostic.topological_target_pairs(
        target, diagnostic.CONTROL_TO_TARGET_PAIRS,
    ):
        source_delta, source_rest, source_pose = diagnostic.semantic_pose_delta(
            source, source_semantic, "source",
        )
        target_rest = diagnostic.world_rotation(
            target, target.data.bones[target_name].matrix_local,
        )
        desired = (source_delta @ target_rest).normalized()
        expected[target_name] = {
            "sourceSemantic": source_semantic,
            "sourceRestWorldQuaternion": q_values(source_rest),
            "sourcePoseWorldQuaternion": q_values(source_pose),
            "sourcePoseDeltaQuaternion": q_values(source_delta),
            "targetRestWorldQuaternion": q_values(target_rest),
            "expectedTargetPoseWorldQuaternion": q_values(desired),
            "expectedRuntimeGameWorldQuaternion": q_values(
                proof.source_quat_to_game(desired).inverted().normalized(),
            ),
            "expectedBasis": target_basis_record(target, target_name, desired, rest_world),
        }
    return expected


def apply_rest_basis_runtime_fk(source, target, pairs):
    """Apply B as exact CoH runtime-local channels, in target-parent order."""
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
        if world_error > 1.0e-4:
            raise RuntimeError(
                f"REST_BASIS_RUNTIME_FK_WORLD_FAIL {target_name} {world_error:.9g} degrees"
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


def runtime_fk_records(target, ordered, bones, source_rest_local):
    """Return the existing exporter/runtime-FK source-world reconstruction."""
    source_world = exporter.runtime_fk_world(
        target, ordered, bones, source_rest_local,
    )
    return source_world


def actual_basis_records(target, rest_world, source_world):
    result = {}
    for name in FOCUS_TARGET:
        orientation = source_world[name][0]
        result[name] = target_basis_record(target, name, orientation, rest_world)
    return result


def compare_expected_actual(target, rest_world, expected, source_world):
    rows = []
    for name in FOCUS_TARGET:
        expected_q = q_from_values(expected[name]["expectedTargetPoseWorldQuaternion"])
        actual_game = proof.current_coh_world_rotation(target, name)
        # The proof target is evaluated in Blender's source-frame matrix
        # convention, while the CoH runtime stores inverse-Hamilton game
        # quaternions.  Undo that runtime convention before comparing against
        # the diagnostic B source-frame basis.
        actual_q = proof.game_quat_to_source_rotation(actual_game.inverted()).normalized()
        expected_basis = diagnostic.basis_from_primary_and_orientation(
            expected_q @ diagnostic.target_rest_segment(rest_world, target, name).normalized(),
            expected_q,
        )
        actual_basis_matrix = diagnostic.basis_from_primary_and_orientation(
            actual_q @ diagnostic.target_rest_segment(rest_world, target, name).normalized(),
            actual_q,
        )
        metrics = diagnostic.compare_basis(expected_basis, actual_basis_matrix)
        pose_bone = target.pose.bones[name]
        rows.append({
            "bone": name,
            "sourceSemantic": expected[name]["sourceSemantic"],
            "expectedTargetPoseWorldQuaternion": expected[name]["expectedTargetPoseWorldQuaternion"],
            "expectedRuntimeGameWorldQuaternion": expected[name]["expectedRuntimeGameWorldQuaternion"],
            "runtimeGameWorldQuaternion": q_values(actual_game),
            "runtimeReconstructedSourceWorldQuaternion": q_values(actual_q),
            "runtimeExporterSourceWorldQuaternion": q_values(source_world[name][0]),
            "runtimeWorldQuaternionErrorDegrees": q_error_degrees(expected_q, actual_q),
            "expectedBasis": expected[name]["expectedBasis"],
            "actualBasis": diagnostic.basis_record(actual_basis_matrix),
            "directionDeltaDegrees": metrics["directionAngularErrorDegrees"],
            "rollDeltaDegrees": metrics["rollOnlyAngularErrorDegrees"],
            "fullBasisDeltaDegrees": metrics["basisAngularErrorDegrees"],
            "localLocationMagnitude": float(pose_bone.matrix_basis.translation.length),
            "localScaleError": float(max(abs(value - 1.0) for value in pose_bone.matrix_basis.to_3x3().to_scale())),
        })
    return rows


def validate_input(source, target, args):
    if args.source_frame != SOURCE_FRAME:
        raise SystemExit("This forensic pilot is fixed to source frame 20.")
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
    if args.source_frame < source.animation_data.action.frame_range[0] or args.source_frame > source.animation_data.action.frame_range[1]:
        raise RuntimeError("Source frame 20 is outside the imported action range")
    if target.get("coh_export_fk") != "runtime-local-bind-translation":
        raise RuntimeError("Exact target is missing the runtime-local-bind-translation contract")


def main():
    args = parse_args()
    if args.frames < 2:
        raise SystemExit("--frames must be at least 2")
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

    bpy.context.scene.frame_set(args.source_frame)
    bpy.context.view_layer.update()
    proof_target = clone_target_for_proof(target)
    expected = expected_pose_records(source, proof_target, rest_world)

    # This is the exact source->target rest-basis formula used by diagnostic B,
    # authored as the real CoH runtime-local FK channels.
    apply_report = apply_rest_basis_runtime_fk(
        source,
        proof_target,
        diagnostic.CONTROL_TO_TARGET_PAIRS,
    )
    bpy.context.scene.frame_set(args.source_frame)
    bpy.context.view_layer.update()
    source_world = runtime_fk_records(proof_target, ordered, bones, source_rest_local)
    rows = compare_expected_actual(proof_target, rest_world, expected, source_world)

    max_world_error = max(row["runtimeWorldQuaternionErrorDegrees"] for row in rows)
    max_direction = max(row["directionDeltaDegrees"] for row in rows)
    max_roll = max(row["rollDeltaDegrees"] for row in rows)
    max_basis = max(row["fullBasisDeltaDegrees"] for row in rows)
    max_location = max(row["localLocationMagnitude"] for row in rows)
    max_scale = max(row["localScaleError"] for row in rows)
    if (
        max_world_error > RUNTIME_ROTATION_TOLERANCE_DEGREES
        or max_direction > RUNTIME_ROTATION_TOLERANCE_DEGREES
        or max_roll > RUNTIME_ROTATION_TOLERANCE_DEGREES
        or max_basis > RUNTIME_ROTATION_TOLERANCE_DEGREES
    ):
        raise RuntimeError(
            "REST_BASIS_PRE_EXPORT_MISMATCH "
            + json.dumps({
                "maxWorldErrorDegrees": max_world_error,
                "maxDirectionDegrees": max_direction,
                "maxRollDegrees": max_roll,
                "maxBasisDegrees": max_basis,
            }, sort_keys=True)
        )
    if max_location > 1.0e-6 or max_scale > 1.0e-6:
        raise RuntimeError(
            f"REST_BASIS_ROTATION_ONLY_FAIL location={max_location:.9g} scale={max_scale:.9g}"
        )

    action, values = keyframe_static_pose(proof_target, args.frames)
    bpy.context.scene.frame_set(args.source_frame)
    bpy.context.view_layer.update()

    output_blend = args.output_dir / f"{args.asset_name}.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(output_blend))
    output_report = args.output_dir / "rest-basis-frame20.math.json"
    report_out = {
        "tool": "agent/animation/create_issue36_rest_basis_frame20.py",
        "blender": bpy.app.version_string,
        "source": {
            "fbx": root_relative(args.source_fbx),
            "blend": root_relative(args.blend),
            "armature": source.name,
            "action": SOURCE_ACTION,
            "sourceFrame": SOURCE_FRAME,
            "fps": 30,
        },
        "target": {
            "armature": proof_target.name,
            "asset": f"MALE/{args.asset_name}",
            "referenceAnimation": report["animation"],
            "boneCount": len(proof_target.data.bones),
            "focusedBones": list(FOCUS_TARGET),
            "action": action.name,
            "authoredFrames": args.frames,
            "staticPose": True,
            "runtimeFkContract": proof_target.get("coh_export_fk"),
        },
        "mapping": {
            "sourceToTarget": diagnostic.TARGET_MAP,
            "formula": "sourceDelta = sourcePoseWorld * inverse(sourceRestWorld); targetPoseWorld = sourceDelta * targetRestWorld",
            "runtimeConvention": "CoH runtime-local game quaternion channels; exporter/runtime FK composes local * parent and converts game frame to ANIMX source frame",
            "translationPolicy": "fixed bind translations; zero pose-bone location; unit pose-bone scale",
        },
        "diagnosticApply": apply_report,
        "preExportProof": {
            "pass": True,
            "runtimeLocalRepresentation": "reconstructed through blender_export_animx.runtime_fk_world before ANIMX export",
            "maxRuntimeWorldQuaternionErrorDegrees": max_world_error,
            "maxDirectionDeltaDegrees": max_direction,
            "maxRollDeltaDegrees": max_roll,
            "maxFullBasisDeltaDegrees": max_basis,
            "rotationToleranceDegrees": RUNTIME_ROTATION_TOLERANCE_DEGREES,
            "maxLocalLocationMagnitude": max_location,
            "maxLocalScaleError": max_scale,
            "bones": rows,
        },
        "sourceSafety": {
            "sourceFbxWritten": False,
            "productionAssetWritten": False,
            "productionSequencerWritten": False,
            "webSwingPhysicsWritten": False,
        },
    }
    output_report.write_text(json.dumps(report_out, indent=2), encoding="utf-8")
    print("ISSUE36_REST_BASIS_FRAME20_CREATED " + json.dumps({
        "asset": f"MALE/{args.asset_name}",
        "blend": str(output_blend),
        "report": str(output_report),
        "sourceFrame": SOURCE_FRAME,
        "frames": args.frames,
        "focusedBones": len(FOCUS_TARGET),
        "maxFullBasisDeltaDegrees": max_basis,
        "maxRuntimeWorldQuaternionErrorDegrees": max_world_error,
    }, sort_keys=True))


if __name__ == "__main__":
    main()
