"""Extract focused Blender/runtime-FK pose correspondence evidence.

This is a diagnostic companion to ``blender_export_animx.py``.  It loads one
of the frame-locked proof blends, samples the exact CoH Male target at the
requested source frame, and records the local pose channels plus the game and
ANIMX world rotations implied by the existing runtime-FK equations.  It does
not modify the blend, source FBX, or runtime assets.
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Quaternion


FOCUS = [
    "HIPS", "WAIST", "CHEST", "NECK", "HEAD",
    "COL_L", "UARML", "LARML", "HANDL",
    "COL_R", "UARMR", "LARMR", "HANDR",
    "ULEGL", "LLEGL", "ULEGR", "LLEGR",
]


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--blend", required=True, type=Path)
    parser.add_argument("--rig-json", required=True, type=Path)
    parser.add_argument("--frame", required=True, type=int)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--armature-name", default="CoH_Male_Exact_Export_Rig")
    return parser.parse_args(argv)


def q_values(q):
    q = q.normalized()
    if q.w < 0.0:
        q.negate()
    return [float(q.x), float(q.y), float(q.z), float(q.w)]


def source_quat_from_game(q):
    q = q.normalized()
    if q.w < 0.0:
        q.negate()
    angle = 2.0 * math.acos(max(-1.0, min(1.0, q.w)))
    sine = math.sin(angle * 0.5)
    if abs(sine) <= 1.0e-7:
        return Quaternion((1.0, 0.0, 0.0, 0.0))
    axis = q.axis.normalized()
    return Quaternion((axis.x, axis.z, -axis.y), angle)


def hierarchy_world(armature, name, local_world):
    bone = armature.data.bones[name]
    local = armature.pose.bones[name].matrix_basis.to_3x3().to_quaternion().normalized()
    parent = bone.parent
    if parent is None:
        world = local
    else:
        parent_world = local_world[parent.name]
        world = local @ parent_world
    local_world[name] = world
    return world


def main():
    args = parse_args()
    bpy.ops.wm.open_mainfile(filepath=str(args.blend))
    armature = bpy.data.objects.get(args.armature_name)
    if armature is None or armature.type != "ARMATURE":
        raise SystemExit(f"Armature not found: {args.armature_name}")
    bpy.context.scene.frame_set(args.frame)
    bpy.context.view_layer.update()

    runtime = json.loads(args.rig_json.read_text(encoding="utf-8"))
    runtime_bones = {item["name"]: item for item in runtime["bones"]}
    local_world = {}
    rows = []
    for name in FOCUS:
        if name not in armature.pose.bones:
            raise SystemExit(f"Focused bone missing from Blender target: {name}")
        pose_bone = armature.pose.bones[name]
        basis = pose_bone.matrix_basis.to_3x3().to_quaternion().normalized()
        game_world = hierarchy_world(armature, name, local_world)
        row = {
            "bone": name,
            "parent": pose_bone.parent.name if pose_bone.parent else None,
            "blenderLocalRotation": q_values(basis),
            "exporterGameLocalRotation": q_values(basis),
            "exporterGameWorldRotation": q_values(game_world),
            "exporterAnimxSourceWorldRotation": q_values(source_quat_from_game(game_world)),
            "matrixBasisLocation": [float(v) for v in pose_bone.matrix_basis.translation],
            "matrixBasisScale": [float(v) for v in pose_bone.matrix_basis.to_3x3().to_scale()],
        }
        if name in runtime_bones:
            row["runtimeFrame0LocalRotation"] = runtime_bones[name]["frame0LocalRotation"]
        rows.append(row)

    report = {
        "tool": "agent/animation/extract_issue36_correspondence.py",
        "blend": str(args.blend),
        "rigJson": str(args.rig_json),
        "frame": args.frame,
        "armature": armature.name,
        "runtimeAnimation": runtime["animation"],
        "equations": {
            "gameWorld": "localRotation * parentWorldRotation",
            "animxSourceWorld": "game quaternion axis (x,y,z) -> (x,z,-y)",
            "exporterLocalContract": "Blender matrix_basis rotation is already game-frame local; bind translations are fixed",
        },
        "bones": rows,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print("ISSUE36_CORRESPONDENCE " + json.dumps({
        "frame": args.frame,
        "armature": armature.name,
        "bones": len(rows),
        "output": str(args.output),
    }, sort_keys=True))


if __name__ == "__main__":
    main()
