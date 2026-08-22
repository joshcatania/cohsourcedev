"""Freeze one evaluated armature frame into a short rotation-only action."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

import bpy


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--blend", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--armature-name", required=True)
    parser.add_argument("--source-frame", required=True, type=int)
    parser.add_argument("--frames", type=int, default=5)
    parser.add_argument("--action-name", default="COHSOURCEDEV_RETARGET_POSE_PROOF_ACTION")
    return parser.parse_args(argv)


def main():
    args = parse_args()
    if args.frames < 2:
        raise SystemExit("--frames must be at least 2 for GetAnimation2")
    bpy.ops.wm.open_mainfile(filepath=str(args.blend.resolve()))
    armature = bpy.data.objects.get(args.armature_name)
    if armature is None or armature.type != "ARMATURE":
        raise SystemExit(f"Armature not found: {args.armature_name}")

    scene = bpy.context.scene
    scene.frame_set(args.source_frame)
    bpy.context.view_layer.update()
    frozen = {
        bone.name: (
            bone.rotation_quaternion.copy(),
            bone.location.copy(),
            bone.scale.copy(),
        )
        for bone in armature.pose.bones
    }

    action = bpy.data.actions.new(args.action_name)
    armature.animation_data_create()
    armature.animation_data.action = action
    scene.frame_start = 1
    scene.frame_end = args.frames
    for frame in range(1, args.frames + 1):
        for bone in armature.pose.bones:
            rotation, location, scale = frozen[bone.name]
            bone.rotation_mode = "QUATERNION"
            bone.rotation_quaternion = rotation
            bone.location = location
            bone.scale = scale
            bone.keyframe_insert("rotation_quaternion", frame=frame, group=bone.name)
            bone.keyframe_insert("location", frame=frame, group=bone.name)
            bone.keyframe_insert("scale", frame=frame, group=bone.name)
    scene.frame_set(1)
    bpy.context.view_layer.update()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(args.output.resolve()))
    print(
        f"STATIC_POSE_CREATED output={args.output} sourceFrame={args.source_frame} "
        f"frames={args.frames} bones={len(armature.pose.bones)}"
    )


if __name__ == "__main__":
    main()
