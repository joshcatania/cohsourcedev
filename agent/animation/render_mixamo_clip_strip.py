"""Render evenly sampled frames from a Mixamo FBX for Web Swing auditioning.

This is deliberately source-side: it lets animation choreography be selected
before any CoH retarget/export work, keeping the accepted rest-basis math out
of subjective clip selection.
"""

from __future__ import annotations

import argparse
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--fbx", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--samples", type=int, default=9)
    return parser.parse_args(argv)


def world_bounds(objects):
    points = []
    depsgraph = bpy.context.evaluated_depsgraph_get()
    for object_ in objects:
        evaluated = object_.evaluated_get(depsgraph)
        points.extend(evaluated.matrix_world @ Vector(corner) for corner in evaluated.bound_box)
    minimum = Vector((min(point.x for point in points), min(point.y for point in points), min(point.z for point in points)))
    maximum = Vector((max(point.x for point in points), max(point.y for point in points), max(point.z for point in points)))
    return minimum, maximum


def add_camera(center, extent):
    camera_data = bpy.data.cameras.new("webswing_audit_camera")
    camera = bpy.data.objects.new("webswing_audit_camera", camera_data)
    bpy.context.collection.objects.link(camera)
    distance = max(extent.x, extent.z) * 2.35
    camera.location = center + Vector((0.0, -distance, extent.z * 0.08))
    camera.rotation_euler = (center - camera.location).to_track_quat("-Z", "Y").to_euler()
    camera_data.type = "ORTHO"
    camera_data.ortho_scale = max(extent.x * 1.45, extent.z * 1.18)
    bpy.context.scene.camera = camera
    return camera, distance


def place_camera(camera, center, distance, vertical_offset):
    camera.location = center + Vector((0.0, -distance, vertical_offset))
    camera.rotation_euler = (center - camera.location).to_track_quat("-Z", "Y").to_euler()


def main():
    args = parse_args()
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=str(args.fbx))
    armature = next((object_ for object_ in bpy.data.objects if object_.type == "ARMATURE"), None)
    meshes = [object_ for object_ in bpy.data.objects if object_.type == "MESH"]
    if armature is None or not meshes:
        raise SystemExit("FBX must contain an armature and visible mesh")
    action = armature.animation_data.action
    first = int(math.floor(action.frame_range[0]))
    last = int(math.ceil(action.frame_range[1]))
    frames = sorted({round(first + index * (last - first) / max(args.samples - 1, 1)) for index in range(args.samples)})

    scene = bpy.context.scene
    frame_bounds = {}
    widest = Vector((0.0, 0.0, 0.0))
    for frame in frames:
        scene.frame_set(frame)
        bpy.context.view_layer.update()
        minimum, maximum = world_bounds(meshes)
        frame_bounds[frame] = (minimum, maximum)
        extent = maximum - minimum
        widest.x = max(widest.x, extent.x)
        widest.y = max(widest.y, extent.y)
        widest.z = max(widest.z, extent.z)
    center = sum(frame_bounds[frames[0]], Vector()) * 0.5
    camera, distance = add_camera(center, widest)
    scene.render.engine = "BLENDER_WORKBENCH"
    scene.display.shading.light = "STUDIO"
    scene.display.shading.studio_light = "paint.sl"
    scene.display.shading.show_shadows = True
    scene.display.shading.show_cavity = True
    scene.display.shading.cavity_type = "WORLD"
    scene.display.shading.color_type = "MATERIAL"
    scene.render.resolution_x = 420
    scene.render.resolution_y = 520
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = False

    args.output_dir.mkdir(parents=True, exist_ok=True)
    for frame in frames:
        scene.frame_set(frame)
        bpy.context.view_layer.update()
        minimum, maximum = frame_bounds[frame]
        center = (minimum + maximum) * 0.5
        place_camera(camera, center, distance, widest.z * 0.08)
        scene.render.filepath = str(args.output_dir / f"frame-{frame:03d}.png")
        bpy.ops.render.render(write_still=True)
    print(f"WEBSWING_SOURCE_STRIP clip={args.fbx.name} frames={','.join(map(str, frames))} output={args.output_dir}")


if __name__ == "__main__":
    main()
