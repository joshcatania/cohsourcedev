"""Retarget one bounded Mixamo Web Swing clip with the accepted rest-basis math.

The transfer and runtime-FK proof functions are imported unchanged from the
Issue 36 accepted BOTTOM/full pipeline.  This file only generalizes source
selection and frame bounds so V2 choreography can use dedicated authored
clips without weakening the original fixed production proof.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import sys
from pathlib import Path

import bpy


ROOT = Path(__file__).resolve().parents[2]
ANIMATION_DIR = ROOT / "agent" / "animation"
if str(ANIMATION_DIR) not in sys.path:
    sys.path.insert(0, str(ANIMATION_DIR))

import diagnose_issue36_orientation as diagnostic  # noqa: E402
from create_blender_canary import build_source_rest, load_rig  # noqa: E402
from create_issue36_rest_basis_bottom import (  # noqa: E402
    RUNTIME_ROTATION_TOLERANCE_DEGREES,
    apply_bottom_rest_basis_runtime_fk,
    pose_values,
    root_relative,
)
from create_issue36_rest_basis_frame20 import (  # noqa: E402
    clone_target_for_proof,
    compare_expected_actual,
    expected_pose_records,
    runtime_fk_records,
)


SOURCE_ACTION = "Armature|mixamo.com|Layer0"


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--blend", required=True, type=Path)
    parser.add_argument("--source-fbx", required=True, type=Path)
    parser.add_argument("--rig-json", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--asset-name", required=True)
    parser.add_argument("--source-start", required=True, type=int)
    parser.add_argument("--source-end", required=True, type=int)
    parser.add_argument(
        "--reverse",
        action="store_true",
        help="Author the requested bounded source range in descending order.",
    )
    return parser.parse_args(argv)


def replace_source_armature(source_fbx):
    old_source = bpy.data.objects.get("Mixamo_Source_Armature")
    if old_source is not None:
        bpy.data.objects.remove(old_source, do_unlink=True)
    existing = set(bpy.data.objects)
    bpy.ops.import_scene.fbx(filepath=str(source_fbx))
    imported_armatures = [
        object_ for object_ in bpy.data.objects
        if object_ not in existing and object_.type == "ARMATURE"
    ]
    if len(imported_armatures) != 1:
        raise RuntimeError(f"Expected one imported source armature, found {len(imported_armatures)}")
    source = imported_armatures[0]
    source.name = "Mixamo_Source_Armature"
    return source


def keyframe_pose_samples(target, samples, authored_frames, asset_name):
    action = bpy.data.actions.new(f"{asset_name}_ACTION")
    target.animation_data_create()
    target.animation_data.action = action
    scene = bpy.context.scene
    scene.frame_start = authored_frames[0]
    scene.frame_end = authored_frames[-1]
    scene.render.fps = 30
    for authored_frame, sample in zip(authored_frames, samples):
        scene.frame_set(authored_frame)
        for pose_bone in target.pose.bones:
            values = sample[pose_bone.name]
            pose_bone.rotation_mode = "QUATERNION"
            pose_bone.rotation_quaternion = values["rotation"]
            pose_bone.location = values["location"]
            pose_bone.scale = values["scale"]
            for data_path in ("rotation_quaternion", "location", "scale"):
                pose_bone.keyframe_insert(data_path=data_path, frame=authored_frame, group=pose_bone.name)
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


def main():
    args = parse_args()
    args.blend = args.blend.resolve()
    args.source_fbx = args.source_fbx.resolve()
    args.rig_json = args.rig_json.resolve()
    args.output_dir = args.output_dir.resolve()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    if not args.source_fbx.is_file():
        raise SystemExit(f"Source FBX not found: {args.source_fbx}")
    if args.source_start < 1 or args.source_end < args.source_start:
        raise SystemExit("Invalid bounded source frame range")

    bpy.ops.wm.open_mainfile(filepath=str(args.blend))
    target = bpy.data.objects.get("CoH_Male_Exact_Export_Rig")
    if target is None:
        raise RuntimeError("Input proof blend is missing the exact Male target")
    source = replace_source_armature(args.source_fbx)
    if source.animation_data is None or source.animation_data.action is None:
        raise RuntimeError("Mixamo source has no active action")
    action = source.animation_data.action
    if not action.name.startswith(SOURCE_ACTION):
        raise RuntimeError(f"Expected source action {SOURCE_ACTION}, got {action.name}")
    action.name = SOURCE_ACTION
    action_start, action_end = (int(round(value)) for value in action.frame_range)
    if args.source_start < action_start or args.source_end > action_end:
        raise RuntimeError(
            f"Requested {args.source_start}..{args.source_end} outside action {action_start}..{action_end}"
        )
    if target.get("coh_export_fk") != "runtime-local-bind-translation":
        raise RuntimeError("Exact target is missing the runtime-local-bind-translation contract")

    source_frames = tuple(range(args.source_start, args.source_end + 1))
    if args.reverse:
        source_frames = tuple(reversed(source_frames))
    authored_frames = tuple(range(1, len(source_frames) + 1))
    report, bones, by_id = load_rig(args.rig_json)
    source_rest_local, rest_world = build_source_rest(bones, by_id)
    ordered = sorted(bones.values(), key=lambda item: item["id"])
    proof_target = clone_target_for_proof(target)

    frame_reports = []
    pose_samples = []
    for source_frame, authored_frame in zip(source_frames, authored_frames):
        bpy.context.scene.frame_set(source_frame)
        bpy.context.view_layer.update()
        expected = expected_pose_records(source, proof_target, rest_world)
        apply_report = apply_bottom_rest_basis_runtime_fk(
            source, proof_target, rest_world, diagnostic.CONTROL_TO_TARGET_PAIRS,
        )
        bpy.context.scene.frame_set(source_frame)
        bpy.context.view_layer.update()
        source_world = runtime_fk_records(proof_target, ordered, bones, source_rest_local)
        rows = compare_expected_actual(proof_target, rest_world, expected, source_world)
        sample = pose_values(proof_target)
        pose_samples.append(sample)
        maxima = {
            "maxRuntimeWorldQuaternionErrorDegrees": max(row["runtimeWorldQuaternionErrorDegrees"] for row in rows),
            "maxDirectionDeltaDegrees": max(row["directionDeltaDegrees"] for row in rows),
            "maxRollDeltaDegrees": max(row["rollDeltaDegrees"] for row in rows),
            "maxFullBasisDeltaDegrees": max(row["fullBasisDeltaDegrees"] for row in rows),
            "maxLocalLocationMagnitude": max(value["localLocationMagnitude"] for value in sample.values()),
            "maxLocalScaleError": max(value["localScaleError"] for value in sample.values()),
        }
        if any(maxima[key] > RUNTIME_ROTATION_TOLERANCE_DEGREES for key in (
            "maxRuntimeWorldQuaternionErrorDegrees", "maxDirectionDeltaDegrees",
            "maxRollDeltaDegrees", "maxFullBasisDeltaDegrees",
        )):
            raise RuntimeError("REST_BASIS_CLIP_PRE_EXPORT_MISMATCH " + json.dumps({"sourceFrame": source_frame, **maxima}, sort_keys=True))
        if maxima["maxLocalLocationMagnitude"] > 1.0e-6 or maxima["maxLocalScaleError"] > 1.0e-6:
            raise RuntimeError(f"REST_BASIS_CLIP_ROTATION_ONLY_FAIL sourceFrame={source_frame} location={maxima['maxLocalLocationMagnitude']:.9g} scaleError={maxima['maxLocalScaleError']:.9g}")
        frame_reports.append({
            "sourceFrame": source_frame,
            "authoredFrame": authored_frame,
            "diagnosticApply": apply_report,
            "preExportProof": {"pass": True, **maxima, "bones": rows},
        })

    action = keyframe_pose_samples(proof_target, pose_samples, authored_frames, args.asset_name)
    bpy.context.scene.frame_set(1)
    bpy.context.view_layer.update()
    output_blend = args.output_dir / f"{args.asset_name}.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(output_blend))
    proof_rows = [frame["preExportProof"] for frame in frame_reports]
    max_keys = (
        "maxRuntimeWorldQuaternionErrorDegrees", "maxDirectionDeltaDegrees",
        "maxRollDeltaDegrees", "maxFullBasisDeltaDegrees",
        "maxLocalLocationMagnitude", "maxLocalScaleError",
    )
    maxima = {key: max(row[key] for row in proof_rows) for key in max_keys}
    expected_local_by_frame = {
        str(authored_frame): {
            name: sample[name]["runtimeLocalQuaternion"] for name in sorted(sample)
        }
        for authored_frame, sample in zip(authored_frames, pose_samples)
    }
    output_report = args.output_dir / "rest-basis-clip.math.json"
    report_out = {
        "tool": "agent/animation/create_issue36_rest_basis_clip.py",
        "blender": bpy.app.version_string,
        "source": {
            "fbx": root_relative(args.source_fbx), "fbxSha256": sha256(args.source_fbx),
            "blend": root_relative(args.blend), "blendSha256": sha256(args.blend),
            "rigJson": root_relative(args.rig_json), "armature": source.name,
            "action": SOURCE_ACTION, "actionFrameRange": [action_start, action_end],
            "sourceFrames": list(source_frames), "fps": 30,
            "reversed": args.reverse,
        },
        "target": {
            "armature": proof_target.name, "asset": f"MALE/{args.asset_name}",
            "referenceAnimation": report["animation"], "boneCount": len(proof_target.data.bones),
            "focusedBones": list(diagnostic.FOCUS_TARGET), "action": action.name,
            "authoredFrames": len(authored_frames),
            "authoredFrameMap": [
                {"authoredFrame": authored, "sourceFrame": source_frame}
                for authored, source_frame in zip(authored_frames, source_frames)
            ],
            "sourceFrameRange": [args.source_start, args.source_end],
            "staticPose": False, "runtimeFkContract": proof_target.get("coh_export_fk"),
        },
        "mapping": {
            "sourceToTarget": diagnostic.TARGET_MAP,
            "formula": "sourceDelta = sourcePoseWorld * inverse(sourceRestWorld); targetPoseWorld = sourceDelta * targetRestWorld",
            "translationPolicy": "fixed bind translations; zero pose-bone location; unit pose-bone scale",
        },
        "preExportProof": {
            "pass": True, "rotationToleranceDegrees": RUNTIME_ROTATION_TOLERANCE_DEGREES,
            **maxima, "expectedRuntimeLocalQuaternionByAuthoredFrame": expected_local_by_frame,
            "frames": frame_reports,
        },
    }
    output_report.write_text(json.dumps(report_out, indent=2) + "\n", encoding="utf-8")
    print("ISSUE36_REST_BASIS_CLIP_CREATED " + json.dumps({
        "asset": f"MALE/{args.asset_name}", "blend": str(output_blend),
        "report": str(output_report), "sourceFrameRange": [args.source_start, args.source_end],
        "authoredFrames": len(authored_frames), "boneCount": len(proof_target.data.bones), **maxima,
    }, sort_keys=True))


if __name__ == "__main__":
    main()
