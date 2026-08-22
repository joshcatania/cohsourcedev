"""Export evaluated Blender pose-bone keyframes as a CoH ANIMX source file."""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Quaternion, Vector


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--blend", required=True, type=Path)
    parser.add_argument("--rig-json", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--armature-name", default="CoH_Male_AnimationRig")
    parser.add_argument("--source-name", default=None)
    parser.add_argument("--start-frame", type=int, default=1)
    parser.add_argument("--end-frame", type=int, default=36)
    return parser.parse_args(argv)


def normalize_quaternion(q):
    q = q.copy()
    if q.magnitude <= 1.0e-12:
        return Quaternion((1.0, 0.0, 0.0, 0.0))
    q.normalize()
    if q.w < 0.0:
        q.negate()
    return q


def source_axis_angle(matrix):
    q = normalize_quaternion(matrix.to_quaternion())
    angle = q.angle
    if angle <= 1.0e-7:
        return Vector((0.0, 1.0, 0.0)), 0.0
    axis = q.axis.normalized()
    return axis, angle


def game_quat_to_source(values):
    q = Quaternion((values[3], values[0], values[1], values[2]))
    q.normalize()
    if q.w < 0.0:
        q.negate()
    angle = 2.0 * math.acos(max(-1.0, min(1.0, q.w)))
    sine = math.sin(angle * 0.5)
    if abs(sine) <= 1.0e-6:
        return Quaternion((1.0, 0.0, 0.0, 0.0))
    axis = Vector((q.x / sine, q.y / sine, q.z / sine))
    return Quaternion((axis.x, axis.z, -axis.y), angle)


def game_position_to_source(values):
    return Vector((-values[0], -values[2], values[1]))


def load_reference(path):
    report = json.loads(path.read_text(encoding="utf-8"))
    bones = {bone["name"]: bone for bone in report["bones"]}
    by_id = {bone["id"]: bone for bone in report["bones"]}
    local = {}
    world = {}
    pending = dict(bones)
    ordered = []
    while pending:
        progressed = False
        for name, bone in list(pending.items()):
            parent = by_id.get(bone["parent"])
            if parent is None or parent["name"] not in pending:
                ordered.append(bone)
                del pending[name]
                progressed = True
        if not progressed:
            raise ValueError("Rig hierarchy contains an unresolved parent")
    for bone in ordered:
        rotation = game_quat_to_source(bone["frame0LocalRotation"])
        translation = game_position_to_source(bone["frame0LocalTranslation"])
        local[bone["name"]] = (rotation, translation)
        parent = by_id.get(bone["parent"])
        if parent is None:
            world[bone["name"]] = (rotation.copy(), translation.copy())
        else:
            parent_rotation, parent_translation = world[parent["name"]]
            world[bone["name"]] = (
                rotation @ parent_rotation,
                parent_translation + (parent_rotation @ translation),
            )
    return report, bones, world


def matrix_from_transform(rotation, translation):
    matrix = rotation.to_matrix().to_4x4()
    matrix.translation = translation
    return matrix


def action_fcurve_count(action):
    fcurves = list(getattr(action, "fcurves", [])) if action else []
    if action and not fcurves and hasattr(action, "layers"):
        for layer in action.layers:
            for strip in layer.strips:
                for channelbag in strip.channelbags:
                    fcurves.extend(channelbag.fcurves)
    return len(fcurves)


def write_animx(args, scene, armature, report, bones, source_rest_world):
    start_frame = args.start_frame
    end_frame = args.end_frame
    total_frames = end_frame - start_frame + 1
    names = [bone["name"] for bone in sorted(bones.values(), key=lambda item: item["id"])]
    source_name = args.source_name or armature.get("coh_source_animation") or report["animation"]
    lines = [
        "# Exported from evaluated Blender pose-bone keyframes",
        "# Blender uses the ANIMX/3ds-Max source coordinate frame for this proof.",
        "",
        "Version 200",
        f"SourceName {source_name}",
        f"TotalFrames {total_frames}",
        "FirstFrame 0",
        "",
    ]

    scene.frame_start = start_frame
    scene.frame_end = end_frame
    for name in names:
        lines.extend([f'Bone "{name}"', "{"])
        pose_bone = armature.pose.bones[name]
        rest_blender = armature.data.bones[name].matrix_local.copy()
        rest_source = matrix_from_transform(*source_rest_world[name])
        for frame in range(start_frame, end_frame + 1):
            scene.frame_set(frame)
            bpy.context.view_layer.update()
            pose_blender = pose_bone.matrix.copy()
            # This delta is the evaluated Blender keyframe relative to the
            # exact reconstructed rest bone.  Applying it to the source-frame
            # rest matrix preserves CoH model-space transforms while allowing
            # Blender to own the authored keyframes.
            delta = rest_blender.inverted_safe() @ pose_blender
            source_world = rest_source @ delta
            axis, angle = source_axis_angle(source_world)
            translation = source_world.translation
            lines.extend([
                "        Transform",
                "        {",
                f"            Axis {axis.x:.9g} {axis.y:.9g} {axis.z:.9g}",
                f"            Angle {angle:.9g}",
                f"            Translation {translation.x:.9g} {translation.y:.9g} {translation.z:.9g}",
                "            Scale 1 1 1",
                "        }",
                "",
            ])
        lines.extend(["}", ""])

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n".join(lines), encoding="utf-8", newline="\n")
    action = armature.animation_data.action if armature.animation_data else None
    fcurve_count = action_fcurve_count(action)
    print(
        "BLENDER_ANIMX_EXPORTED "
        f"output={args.output} frames={total_frames} bones={len(names)} "
        f"fcurves={fcurve_count} source={source_name}"
    )


def main():
    args = parse_args()
    bpy.ops.wm.open_mainfile(filepath=str(args.blend))
    armature = bpy.data.objects.get(args.armature_name)
    if armature is None or armature.type != "ARMATURE":
        raise SystemExit(f"Armature not found: {args.armature_name}")
    report, bones, source_rest_world = load_reference(args.rig_json)
    write_animx(args, bpy.context.scene, armature, report, bones, source_rest_world)


if __name__ == "__main__":
    main()
