"""Author purpose-built Web Swing phase animations in Blender.

The script reconstructs one of the shipped CoH player rigs from a
GetAnimation2 runtime report, creates a real Blender armature, inserts
pose-bone quaternion keyframes, and saves a normal .blend source artifact.
The existing evaluated-pose ANIMX exporter and GetAnimation2 compiler remain
the only path from this authoring source to a runtime .anim file.
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Quaternion, Vector


AUTHORED_BONES = [
    "HIPS", "WAIST", "CHEST",
    "COL_R", "UARMR", "LARMR", "HANDR",
    "COL_L", "UARML", "LARML", "HANDL",
    "ULEGR", "LLEGR", "FOOTR",
    "ULEGL", "LLEGL", "FOOTL",
    "NECK", "HEAD",
]

ACTION_SPECS = {
    "stretch": {
        "logical": "COHSOURCEDEV_WEBSWING_STRETCH",
        "frames": 30,
        "keyframes": [1, 6, 15, 24, 30],
        "profiles": [0.0, 0.65, -0.70, 0.45, 0.0],
        "description": "long asymmetric hang with a raised tether arm and trailing legs",
    },
    "tuck": {
        "logical": "COHSOURCEDEV_WEBSWING_TUCK",
        "frames": 24,
        "keyframes": [1, 5, 12, 19, 24],
        "profiles": [0.0, 0.45, -0.35, 0.30, 0.0],
        "description": "compact athletic bottom-of-arc compression",
    },
    "ascend": {
        "logical": "COHSOURCEDEV_WEBSWING_ASCEND",
        "frames": 28,
        "keyframes": [1, 6, 14, 22, 28],
        "profiles": [0.0, 0.55, -0.45, 0.35, 0.0],
        "description": "open outbound extension after the bottom tuck",
    },
}


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--rig-json", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--action", required=True, choices=sorted(ACTION_SPECS))
    parser.add_argument("--rig-type", required=True, choices=["male", "fem", "huge"])
    return parser.parse_args(argv)


def normalize_quaternion(values):
    q = Quaternion((values[3], values[0], values[1], values[2]))
    if q.magnitude <= 1.0e-12:
        return Quaternion((1.0, 0.0, 0.0, 0.0))
    q.normalize()
    return q


def game_quat_to_source(values):
    """Invert process_animx.c's runtime-to-ANIMX axis conversion."""
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
    missing = [name for name in AUTHORED_BONES if name not in bones]
    if missing:
        raise ValueError(f"{path} is missing common Web Swing bones: {', '.join(missing)}")
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
    world = {}
    for bone in ordered_bones(bones, by_id):
        name = bone["name"]
        local_rotation = game_quat_to_source(bone["frame0LocalRotation"])
        local_translation = game_position_to_source(bone["frame0LocalTranslation"])
        parent = by_id.get(bone["parent"])
        if parent is None:
            world[name] = (local_rotation.copy(), local_translation.copy())
        else:
            parent_rotation, parent_translation = world[parent["name"]]
            world[name] = (
                local_rotation @ parent_rotation,
                parent_translation + (parent_rotation @ local_translation),
            )
    return world


def matrix_from_transform(rotation, translation):
    matrix = rotation.to_matrix().to_4x4()
    matrix.translation = translation
    return matrix


def create_armature(report, bones, by_id, rest_world, rig_type, logical_name):
    if bpy.context.object and bpy.context.object.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)

    armature_data = bpy.data.armatures.new(f"CoH_{rig_type.title()}_AnimationRig")
    armature_object = bpy.data.objects.new(armature_data.name, armature_data)
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
    armature_object["coh_source_animation"] = f"{rig_type}/{logical_name}"
    armature_object["coh_reference_animation"] = report["animation"]
    armature_object["coh_rig_type"] = rig_type
    armature_object["coh_bone_count"] = len(bones)
    armature_object["coh_authored_bones"] = ",".join(AUTHORED_BONES)
    return armature_object


def delta_quaternion(*angles):
    """Compose local source-frame axis-angle deltas in authoring order."""
    result = Quaternion((1.0, 0.0, 0.0, 0.0))
    for axis, degrees in angles:
        if abs(degrees) > 1.0e-6:
            result = Quaternion(axis, math.radians(degrees)) @ result
    return result


def add_profile(base, amount, profile):
    return base + amount * profile


def phase_pose(action, profile):
    """Return meaningful local rotations for the shared semantic bones.

    The source frame is the ANIMX/3ds-Max frame used by Blender here: source Z
    is game up, source Y is the inverse of game forward, and source X is the
    inverse of game right.  The signs below are therefore deliberately kept
    in this source-frame authoring layer instead of being scattered through
    the exporter.
    """
    if action == "stretch":
        values = {
            "HIPS": ((1, 0, 0), add_profile(15, 2, profile)),
            "WAIST": ((1, 0, 0), add_profile(10, 3, profile)),
            "CHEST": ((1, 0, 0), add_profile(8, 4, profile)),
            "COL_R": ((0, 1, 0), add_profile(38, 3, profile)),
            "UARMR": ((0, 1, 0), add_profile(57, 4, profile)),
            "LARMR": ((0, 1, 0), add_profile(29, 5, profile)),
            "HANDR": ((0, 1, 0), add_profile(8, 2, profile)),
            "COL_L": ((0, 1, 0), add_profile(22, 3, profile)),
            "UARML": ((1, 0, 0), add_profile(-20, 4, profile)),
            "LARML": ((0, 1, 0), add_profile(12, 3, profile)),
            "HANDL": ((1, 0, 0), add_profile(-10, 3, profile)),
            "ULEGR": ((1, 0, 0), add_profile(-16, 4, profile)),
            "LLEGR": ((1, 0, 0), add_profile(10, 6, profile)),
            "FOOTR": ((1, 0, 0), add_profile(-8, 3, profile)),
            "ULEGL": ((1, 0, 0), add_profile(-8, 3, profile)),
            "LLEGL": ((1, 0, 0), add_profile(20, 5, profile)),
            "FOOTL": ((1, 0, 0), add_profile(-4, 3, profile)),
            "NECK": ((1, 0, 0), add_profile(6, 2, profile)),
            "HEAD": ((1, 0, 0), add_profile(5, 2, profile)),
        }
    elif action == "tuck":
        values = {
            "HIPS": ((1, 0, 0), add_profile(8, 2, profile)),
            "WAIST": ((1, 0, 0), add_profile(18, 3, profile)),
            "CHEST": ((1, 0, 0), add_profile(25, 4, profile)),
            "COL_R": ((0, 1, 0), add_profile(31, 3, profile)),
            "UARMR": ((0, 1, 0), add_profile(50, 4, profile)),
            "LARMR": ((0, 1, 0), add_profile(53, 6, profile)),
            "HANDR": ((0, 1, 0), add_profile(7, 2, profile)),
            "COL_L": ((0, 1, 0), add_profile(28, 4, profile)),
            "UARML": ((1, 0, 0), add_profile(-30, 5, profile)),
            "LARML": ((1, 0, 0), add_profile(-45, 7, profile)),
            "HANDL": ((1, 0, 0), add_profile(-20, 4, profile)),
            "ULEGR": ((1, 0, 0), add_profile(-72, 5, profile)),
            "LLEGR": ((1, 0, 0), add_profile(108, 7, profile)),
            "FOOTR": ((1, 0, 0), add_profile(-18, 5, profile)),
            "ULEGL": ((1, 0, 0), add_profile(-58, 4, profile)),
            "LLEGL": ((1, 0, 0), add_profile(90, 6, profile)),
            "FOOTL": ((1, 0, 0), add_profile(-10, 4, profile)),
            "NECK": ((1, 0, 0), add_profile(11, 3, profile)),
            "HEAD": ((1, 0, 0), add_profile(8, 3, profile)),
        }
    else:
        values = {
            "HIPS": ((1, 0, 0), add_profile(18, 3, profile)),
            "WAIST": ((1, 0, 0), add_profile(13, 3, profile)),
            "CHEST": ((1, 0, 0), add_profile(7, 4, profile)),
            "COL_R": ((0, 1, 0), add_profile(43, 4, profile)),
            "UARMR": ((0, 1, 0), add_profile(63, 5, profile)),
            "LARMR": ((0, 1, 0), add_profile(34, 5, profile)),
            "HANDR": ((0, 1, 0), add_profile(8, 2, profile)),
            "COL_L": ((0, 1, 0), add_profile(25, 4, profile)),
            "UARML": ((1, 0, 0), add_profile(-25, 5, profile)),
            "LARML": ((1, 0, 0), add_profile(-34, 6, profile)),
            "HANDL": ((1, 0, 0), add_profile(-12, 3, profile)),
            "ULEGR": ((1, 0, 0), add_profile(-20, 5, profile)),
            "LLEGR": ((1, 0, 0), add_profile(-8, 5, profile)),
            "FOOTR": ((1, 0, 0), add_profile(-10, 3, profile)),
            "ULEGL": ((1, 0, 0), add_profile(-14, 4, profile)),
            "LLEGL": ((1, 0, 0), add_profile(-19, 5, profile)),
            "FOOTL": ((1, 0, 0), add_profile(-5, 3, profile)),
            "NECK": ((1, 0, 0), add_profile(6, 2, profile)),
            "HEAD": ((1, 0, 0), add_profile(5, 2, profile)),
        }

    # A small roll wave keeps the silhouette alive without introducing root
    # translation or a fake runtime orientation system.
    roll = profile * 3.0
    pose = {}
    for bone, (axis, angle) in values.items():
        if bone in ("HIPS", "WAIST", "CHEST"):
            pose[bone] = delta_quaternion((axis, angle), ((0, 0, 1), roll))
        else:
            pose[bone] = delta_quaternion((axis, angle))
    return pose


def action_metrics(action):
    fcurves = list(getattr(action, "fcurves", []))
    if not fcurves and hasattr(action, "layers"):
        for layer in action.layers:
            for strip in layer.strips:
                for channelbag in strip.channelbags:
                    fcurves.extend(channelbag.fcurves)
    return len(fcurves), sum(len(curve.keyframe_points) for curve in fcurves)


def author_keyframes(armature_object, action_name):
    spec = ACTION_SPECS[action_name]
    scene = bpy.context.scene
    scene.frame_start = 1
    scene.frame_end = spec["frames"]
    scene.render.fps = 30
    action = bpy.data.actions.new(f"{spec['logical']}_{armature_object['coh_rig_type'].upper()}_BLENDER_ACTION")
    armature_object.animation_data_create()
    armature_object.animation_data.action = action

    for frame, profile in zip(spec["keyframes"], spec["profiles"]):
        pose = phase_pose(action_name, profile)
        for bone_name in AUTHORED_BONES:
            pose_bone = armature_object.pose.bones[bone_name]
            pose_bone.rotation_mode = "QUATERNION"
            pose_bone.rotation_quaternion = pose[bone_name]
            pose_bone.keyframe_insert(data_path="rotation_quaternion", frame=frame, group=bone_name)

    fcurves = list(getattr(action, "fcurves", []))
    if not fcurves and hasattr(action, "layers"):
        for layer in action.layers:
            for strip in layer.strips:
                for channelbag in strip.channelbags:
                    fcurves.extend(channelbag.fcurves)
    for fcurve in fcurves:
        for keyframe in fcurve.keyframe_points:
            keyframe.interpolation = "BEZIER"

    scene.frame_set(1)
    bpy.context.view_layer.update()
    fcurve_count, keyframe_point_count = action_metrics(action)
    armature_object["coh_logical_animation"] = spec["logical"]
    armature_object["coh_clip_frames"] = spec["frames"]
    armature_object["coh_keyframe_frames"] = ",".join(str(frame) for frame in spec["keyframes"])
    armature_object["coh_keyframe_point_count"] = keyframe_point_count
    armature_object["coh_fcurve_count"] = fcurve_count
    armature_object["coh_pose_description"] = spec["description"]


def main():
    args = parse_args()
    report, bones, by_id = load_rig(args.rig_json)
    spec = ACTION_SPECS[args.action]
    rest_world = build_source_rest(bones, by_id)
    armature_object = create_armature(report, bones, by_id, rest_world, args.rig_type, spec["logical"])
    author_keyframes(armature_object, args.action)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(args.output))
    print(
        "BLENDER_WEBSWING_CREATED "
        f"action={args.action} logical={spec['logical']} rig={args.rig_type} "
        f"blend={args.output} frames={spec['frames']} bones={len(bones)} "
        f"authored={len(AUTHORED_BONES)} keyframeFrames={armature_object['coh_keyframe_frames']} "
        f"keyframePoints={armature_object['coh_keyframe_point_count']} "
        f"fcurves={armature_object['coh_fcurve_count']} reference={report['animation']}"
    )


if __name__ == "__main__":
    main()
