"""Render a deterministic multi-angle proxy contact sheet for a Web Swing pose.

The source rigs intentionally contain only the export armature, not a costume
mesh.  This renderer therefore builds a small joint-and-segment mannequin
from the evaluated pose matrices.  It is a visual authoring gate: the actual
CoH runtime animation remains the final human check, while this view makes
shoulder continuity, elbow plane, wrist direction, and self-intersection
visible from five repeatable cameras.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

import bpy
from mathutils import Vector


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--blend", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--frame", type=int, default=15)
    parser.add_argument("--armature-name", default=None)
    return parser.parse_args(argv)


def make_material(name, color, metallic=0.0, roughness=0.55):
    material = bpy.data.materials.new(name)
    material.diffuse_color = (*color, 1.0)
    material.use_nodes = True
    principled = material.node_tree.nodes.get("Principled BSDF")
    principled.inputs["Base Color"].default_value = (*color, 1.0)
    principled.inputs["Metallic"].default_value = metallic
    principled.inputs["Roughness"].default_value = roughness
    return material


def add_sphere(name, location, radius, material, scale=None):
    bpy.ops.mesh.primitive_uv_sphere_add(
        segments=20,
        ring_count=12,
        radius=radius,
        location=location,
    )
    object_ = bpy.context.object
    object_.name = name
    if scale is not None:
        object_.scale = scale
    object_.data.materials.append(material)
    return object_


def add_segment(name, start, end, radius, material):
    vector = end - start
    if vector.length <= 1.0e-5:
        return None
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=16,
        radius=radius,
        depth=vector.length,
        location=(start + end) * 0.5,
    )
    object_ = bpy.context.object
    object_.name = name
    object_.rotation_euler = vector.to_track_quat("Z", "Y").to_euler()
    object_.data.materials.append(material)
    return object_


def origin(armature, name):
    return armature.pose.bones[name].matrix.translation.copy()


def add_limb(armature, prefix, material, joint_radius=0.055, segment_radius=0.045):
    names = [f"UARM{prefix}", f"LARM{prefix}", f"HAND{prefix}"]
    points = [origin(armature, name) for name in names]
    collar = origin(armature, f"COL_{prefix}")
    add_segment(f"COL_{prefix}_to_shoulder", collar, points[0], segment_radius * 0.9, material)
    add_segment(f"UARM{prefix}", points[0], points[1], segment_radius, material)
    add_segment(f"LARM{prefix}", points[1], points[2], segment_radius * 0.9, material)
    for index, point in enumerate(points):
        add_sphere(f"{prefix}_joint_{index}", point, joint_radius, material)
    return points


def add_leg(armature, prefix, material):
    names = [f"ULEG{prefix}", f"LLEG{prefix}", f"FOOT{prefix}"]
    points = [origin(armature, name) for name in names]
    add_segment(f"ULEG{prefix}", points[0], points[1], 0.065, material)
    add_segment(f"LLEG{prefix}", points[1], points[2], 0.052, material)
    for index, point in enumerate(points):
        add_sphere(f"{prefix}_leg_joint_{index}", point, 0.07 if index < 2 else 0.055, material)


def build_proxy(armature):
    body = make_material("body", (0.08, 0.20, 0.55), metallic=0.15)
    body_dark = make_material("body_dark", (0.04, 0.09, 0.25), metallic=0.1)
    right_arm = make_material("tether_arm", (0.95, 0.16, 0.07), metallic=0.05)
    left_arm = make_material("free_arm", (0.96, 0.55, 0.08), metallic=0.05)
    head_material = make_material("head", (0.85, 0.44, 0.22), roughness=0.65)
    tether_material = make_material("tether", (0.95, 0.04, 0.75), metallic=0.25, roughness=0.35)

    hips = origin(armature, "HIPS")
    waist = origin(armature, "WAIST")
    chest = origin(armature, "CHEST")
    neck = origin(armature, "NECK")
    head = origin(armature, "HEAD")
    add_segment("torso_lower", hips, waist, 0.16, body_dark)
    add_segment("torso_upper", waist, chest, 0.18, body)
    add_sphere("hips", hips, 0.20, body_dark, scale=(1.1, 0.75, 0.75))
    add_sphere("chest", chest, 0.28, body, scale=(1.05, 0.70, 0.72))
    add_segment("neck", chest, neck, 0.09, body_dark)
    add_sphere("head", head, 0.24, head_material, scale=(0.85, 0.85, 1.15))

    right_points = add_limb(armature, "R", right_arm)
    left_points = add_limb(armature, "L", left_arm)
    add_leg(armature, "R", body)
    add_leg(armature, "L", body)

    # The WEP marker is a real reconstructed CoH child origin.  Extending it
    # makes the authored hand orientation and tether implication inspectable.
    hand = right_points[-1]
    marker = origin(armature, "WEPR")
    marker_direction = marker - hand
    if marker_direction.length <= 1.0e-5:
        marker_direction = Vector((0.0, 0.0, 1.0))
    marker_direction.normalize()
    tether_end = hand + marker_direction * 0.95
    add_segment("implied_tether", hand, tether_end, 0.018, tether_material)
    add_sphere("tether_anchor", tether_end, 0.045, tether_material)


def add_floor():
    floor_material = make_material("floor", (0.012, 0.018, 0.035), roughness=0.9)
    bpy.ops.mesh.primitive_plane_add(size=30, location=(0.0, 0.0, 3.35))
    floor = bpy.context.object
    floor.name = "authoring_floor"
    floor.data.materials.append(floor_material)


def add_lighting():
    world = bpy.context.scene.world
    world.color = (0.005, 0.008, 0.02)
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.005, 0.008, 0.02, 1.0)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.35

    bpy.ops.object.light_add(type="AREA", location=(-3.0, -4.0, 9.0))
    key = bpy.context.object
    key.data.energy = 850.0
    key.data.shape = "DISK"
    key.data.size = 5.0
    key.rotation_euler = (Vector((0.0, 0.0, 5.3)) - key.location).to_track_quat("-Z", "Y").to_euler()

    bpy.ops.object.light_add(type="AREA", location=(5.0, 2.0, 6.0))
    fill = bpy.context.object
    fill.data.energy = 500.0
    fill.data.size = 4.0
    fill.rotation_euler = (Vector((0.0, 0.0, 5.2)) - fill.location).to_track_quat("-Z", "Y").to_euler()

    bpy.ops.object.light_add(type="POINT", location=(0.0, -1.0, 7.5))
    bpy.context.object.data.energy = 180.0


def make_camera(name, location, target):
    camera_data = bpy.data.cameras.new(name)
    camera = bpy.data.objects.new(name, camera_data)
    bpy.context.collection.objects.link(camera)
    camera.location = location
    camera.rotation_euler = (target - camera.location).to_track_quat("-Z", "Y").to_euler()
    camera_data.lens = 55.0
    camera_data.clip_start = 0.05
    camera_data.clip_end = 100.0
    return camera


def main():
    args = parse_args()
    bpy.ops.wm.open_mainfile(filepath=str(args.blend))
    armature = None
    if args.armature_name:
        armature = bpy.data.objects.get(args.armature_name)
    if armature is None:
        armature = next((object_ for object_ in bpy.data.objects if object_.type == "ARMATURE"), None)
    if armature is None:
        raise SystemExit("No armature found in blend")

    scene = bpy.context.scene
    scene.frame_set(args.frame)
    bpy.context.view_layer.update()
    for object_ in list(bpy.data.objects):
        if object_.type not in {"ARMATURE"}:
            bpy.data.objects.remove(object_, do_unlink=True)
    build_proxy(armature)
    add_floor()
    add_lighting()

    args.output_dir.mkdir(parents=True, exist_ok=True)
    target = Vector((0.0, 0.0, 5.25))
    views = {
        "front": (Vector((0.0, -8.5, 5.4)), target),
        "front-3-4": (Vector((-5.0, -7.5, 5.6)), target),
        "side": (Vector((-8.5, -0.15, 5.4)), target),
        "rear-3-4": (Vector((5.0, 7.5, 5.6)), target),
        "gameplay": (Vector((4.5, -9.5, 6.5)), target),
    }
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 480
    scene.render.resolution_y = 560
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = False
    for label, (location, view_target) in views.items():
        camera = make_camera(f"camera_{label}", location, view_target)
        scene.camera = camera
        scene.render.filepath = str(args.output_dir / f"{label}.png")
        bpy.ops.render.render(write_still=True)
        bpy.data.objects.remove(camera, do_unlink=True)
    print(
        "WEBSWING_CONTACT_RENDERED "
        f"blend={args.blend} frame={args.frame} outputDir={args.output_dir} views={','.join(views)}"
    )


if __name__ == "__main__":
    main()
