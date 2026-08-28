"""Create a real Blender armature and keyframed CoH animation canary.

The armature is reconstructed from a GetAnimation2 runtime report.  Blender
is treated as the ANIMX/3ds-Max source coordinate frame (right-handed, Z-up),
so the only coordinate conversion in the export path is the inverse of
process_animx.c's runtime conversion.  The animation is authored by changing
pose-bone properties and inserting Blender keyframes; the exporter reads the
evaluated pose from the resulting .blend file.
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Quaternion, Vector


AUTHORED_BONES = [
    "HIPS", "WAIST", "CHEST", "COL_R", "UARMR", "LARMR",
    "COL_L", "UARML", "LARML", "ULEGR", "ULEGL", "NECK", "HEAD",
]


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--rig-json", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--frames", type=int, default=36)
    return parser.parse_args(argv)


def normalize_quaternion(values):
    q = Quaternion((values[3], values[0], values[1], values[2]))
    if q.magnitude <= 1.0e-12:
        return Quaternion((1.0, 0.0, 0.0, 0.0))
    q.normalize()
    return q


def game_quat_to_source(values):
    """Invert process_animx.c's source-axis conversion for one local q."""
    game_q = normalize_quaternion(values)
    if game_q.w < 0.0:
        game_q.negate()
    angle = 2.0 * math.acos(max(-1.0, min(1.0, game_q.w)))
    sine = math.sin(angle * 0.5)
    if abs(sine) <= 1.0e-6:
        return Quaternion((1.0, 0.0, 0.0, 0.0))
    game_axis = Vector((game_q.x / sine, game_q.y / sine, game_q.z / sine))
    source_axis = Vector((game_axis.x, game_axis.z, -game_axis.y))
    return Quaternion(source_axis, angle)


def game_position_to_source(values):
    """Exact inverse of ConvertCoordsFrom3DSMAX in processanim.c."""
    return Vector((-values[0], -values[2], values[1]))


def load_rig(path):
    report = json.loads(path.read_text(encoding="utf-8"))
    bones = {bone["name"]: bone for bone in report["bones"]}
    by_id = {bone["id"]: bone for bone in report["bones"]}
    return report, bones, by_id


def ordered_bones(bones, by_id):
    ordered = []
    pending = dict(bones)
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
    return ordered


def build_source_rest(bones, by_id):
    local = {}
    world = {}
    for bone in ordered_bones(bones, by_id):
        name = bone["name"]
        local_rotation = game_quat_to_source(bone["frame0LocalRotation"])
        local_translation = game_position_to_source(bone["frame0LocalTranslation"])
        local[name] = (local_rotation, local_translation)
        parent = by_id.get(bone["parent"])
        if parent is None:
            world[name] = (local_rotation.copy(), local_translation.copy())
        else:
            parent_rotation, parent_translation = world[parent["name"]]
            world[name] = (
                local_rotation @ parent_rotation,
                parent_translation + (parent_rotation @ local_translation),
            )
    return local, world


def matrix_from_transform(rotation, translation):
    matrix = rotation.to_matrix().to_4x4()
    matrix.translation = translation
    return matrix


def create_armature(report, bones, by_id, rest_world):
    bpy.ops.object.mode_set(mode="OBJECT") if bpy.context.object and bpy.context.object.mode != "OBJECT" else None
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)

    armature_data = bpy.data.armatures.new("CoH_Male_AnimationRig")
    armature_object = bpy.data.objects.new("CoH_Male_AnimationRig", armature_data)
    bpy.context.collection.objects.link(armature_object)
    bpy.context.view_layer.objects.active = armature_object
    armature_object.select_set(True)
    armature_data.display_type = "BBONE"
    armature_data.axes_position = 0

    bpy.ops.object.mode_set(mode="EDIT")
    edit_bones = {}
    for bone in ordered_bones(bones, by_id):
        name = bone["name"]
        edit_bone = armature_data.edit_bones.new(name)
        parent = by_id.get(bone["parent"])
        if parent is not None:
            edit_bone.parent = edit_bones[parent["name"]]
            edit_bone.use_connect = False
        transform = matrix_from_transform(*rest_world[name])
        head = transform.translation
        tail = head + (transform.to_3x3() @ Vector((0.0, 0.25, 0.0)))
        edit_bone.head = head
        edit_bone.tail = tail if (tail - head).length > 1.0e-4 else head + Vector((0.0, 0.25, 0.0))
        edit_bone.matrix = transform
        edit_bones[name] = edit_bone
    bpy.ops.object.mode_set(mode="OBJECT")

    armature_object["coh_animation_source"] = "ANIMX/3ds-Max source frame"
    armature_object["coh_source_animation"] = "male/cohsourcedev_blender_canary"
    armature_object["coh_reference_animation"] = report["animation"]
    armature_object["coh_bone_count"] = len(bones)
    armature_object["coh_authored_bones"] = ",".join(AUTHORED_BONES)
    return armature_object


def make_local_delta(axis, angle):
    return Quaternion(Vector(axis), angle)


def action_metrics(action):
    fcurves = list(getattr(action, "fcurves", []))
    if not fcurves and hasattr(action, "layers"):
        for layer in action.layers:
            for strip in layer.strips:
                for channelbag in strip.channelbags:
                    fcurves.extend(channelbag.fcurves)
    return len(fcurves), sum(len(curve.keyframe_points) for curve in fcurves)


def author_keyframes(armature_object, bones, frames):
    scene = bpy.context.scene
    scene.frame_start = 1
    scene.frame_end = frames
    scene.render.fps = 30
    action = bpy.data.actions.new("COHSOURCEDEV_CUSTOM_CANARY_BLENDER_ACTION")
    armature_object.animation_data_create()
    armature_object.animation_data.action = action

    keyframes = [1, max(2, frames // 3), max(3, (2 * frames) // 3), frames]
    for frame in keyframes:
        phase = 2.0 * math.pi * (frame - 1) / max(1, frames - 1)
        wave = math.sin(phase)
        lift = math.sin(phase * 2.0)
        for bone_name in AUTHORED_BONES:
            pose_bone = armature_object.pose.bones[bone_name]
            pose_bone.rotation_mode = "QUATERNION"
            angle = 0.0
            axis = (0.0, 0.0, 1.0)
            if bone_name == "CHEST":
                axis, angle = (0.0, 0.0, 1.0), math.radians(7.0) * wave
            elif bone_name in ("UARMR", "UARML"):
                axis, angle = (1.0, 0.0, 0.0), math.radians(42.0) * wave * (1.0 if bone_name == "UARMR" else -1.0)
            elif bone_name in ("LARMR", "LARML"):
                axis, angle = (1.0, 0.0, 0.0), math.radians(65.0) * wave * (-1.0 if bone_name == "LARMR" else 1.0)
            elif bone_name in ("ULEGR", "ULEGL"):
                axis, angle = (1.0, 0.0, 0.0), math.radians(12.0) * lift * (1.0 if bone_name == "ULEGR" else -1.0)
            elif bone_name == "HEAD":
                axis, angle = (0.0, 0.0, 1.0), math.radians(5.0) * wave
            pose_bone.rotation_quaternion = make_local_delta(axis, angle)
            pose_bone.keyframe_insert(data_path="rotation_quaternion", frame=frame, group=bone_name)

            if bone_name == "CHEST":
                pose_bone.location = Vector((0.0, 0.02 * lift, 0.0))
                pose_bone.keyframe_insert(data_path="location", frame=frame, group=bone_name)

    for fcurve in getattr(action, "fcurves", []):
        for keyframe in fcurve.keyframe_points:
            keyframe.interpolation = "BEZIER"
    scene.frame_set(1)
    bpy.context.view_layer.update()
    fcurve_count, keyframe_point_count = action_metrics(action)
    armature_object["coh_keyframe_frame_count"] = len(keyframes)
    armature_object["coh_keyframe_point_count"] = keyframe_point_count
    armature_object["coh_fcurve_count"] = fcurve_count


def main():
    args = parse_args()
    if args.frames < 2:
        raise SystemExit("--frames must be at least 2")
    report, bones, by_id = load_rig(args.rig_json)
    _, rest_world = build_source_rest(bones, by_id)
    armature_object = create_armature(report, bones, by_id, rest_world)
    author_keyframes(armature_object, bones, args.frames)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(args.output))
    print(
        "BLENDER_CANARY_CREATED "
        f"blend={args.output} frames={args.frames} bones={len(bones)} "
        f"authored={len(AUTHORED_BONES)} keyframeFrames={armature_object['coh_keyframe_frame_count']} "
        f"keyframePoints={armature_object['coh_keyframe_point_count']} "
        f"fcurves={armature_object['coh_fcurve_count']} reference={report['animation']}"
    )


if __name__ == "__main__":
    main()
