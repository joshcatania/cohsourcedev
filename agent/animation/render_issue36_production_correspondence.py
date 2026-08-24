"""Render raw Mixamo and the production Blender CoH target side by side."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

import bpy
from mathutils import Vector


ROOT = Path(__file__).resolve().parents[2]
if str(ROOT / "agent" / "animation") not in sys.path:
    sys.path.insert(0, str(ROOT / "agent" / "animation"))

import prove_mixamo_anatomical_pose as proof  # noqa: E402
from create_blender_canary import build_source_rest, load_rig  # noqa: E402


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--blend", required=True, type=Path)
    parser.add_argument("--rig-json", required=True, type=Path)
    parser.add_argument("--frame", required=True, type=int)
    parser.add_argument("--output-dir", required=True, type=Path)
    return parser.parse_args(argv)


def create_clean_proxy(name, segments, points, color):
    """Render one representation as a single-color bone/joint skeleton.

    The proof renderer's asymmetric roll fins are useful for axis diagnostics,
    but they can read as a second skeleton in a source-vs-target comparison.
    This view intentionally removes those fins and axes so silhouette and
    connectivity are the only visual signals.
    """
    collection = bpy.data.collections.new(name)
    bpy.context.scene.collection.children.link(collection)
    material = proof.make_material(f"{name}_Body", color, metallic=0.05, roughness=0.7)
    for _, start, end, _ in segments:
        proof.add_cylinder_between(collection, start, end, 0.075, material)
    for point in points:
        proof.add_sphere(collection, point, 0.115, material)
    return collection


def main():
    args = parse_args()
    bpy.ops.wm.open_mainfile(filepath=str(args.blend))
    # The saved proof blend contains older visual proxy meshes.  They are
    # evidence from an earlier render pass, not inputs to this comparison;
    # hide existing renderable meshes before creating the two clean,
    # frame-locked representations.  Keep the scene's light/camera setup
    # available for the renderer to replace.
    for obj in bpy.data.objects:
        if obj.type in {"MESH", "ARMATURE"}:
            obj.hide_render = True
    source = bpy.data.objects.get("Mixamo_Source_Armature")
    target = bpy.data.objects.get("CoH_Male_Exact_Export_Rig")
    source_root = bpy.data.objects.get("Mixamo_Display_Root")
    if source is None or target is None or source_root is None:
        raise SystemExit("Production blend is missing source, target, or display-root objects")

    bpy.context.scene.frame_set(args.frame)
    bpy.context.view_layer.update()
    report, bones, by_id = load_rig(args.rig_json.resolve())
    _, rest_world = build_source_rest(bones, by_id)
    proof.align_source_for_display([source], source_root, source, target, rest_world)
    bpy.context.view_layer.update()
    source_segments, source_points = proof.semantic_proxy_points(source, "source")
    target_segments, target_points = proof.semantic_proxy_points(target, "coh")
    source_proxy = create_clean_proxy("Issue36_Raw_Mixamo", source_segments, source_points, (0.20, 0.68, 0.95))
    target_proxy = create_clean_proxy("Issue36_Blender_CoH_Male", target_segments, target_points, (0.95, 0.32, 0.08))
    proxies = {"source": source_proxy, "coh": target_proxy}
    all_points = source_points + target_points
    for segments in (source_segments, target_segments):
        for _, start, end, _ in segments:
            all_points.extend((start, end))
    bounds = (
        Vector((min(point.x for point in all_points), min(point.y for point in all_points), min(point.z for point in all_points))),
        Vector((max(point.x for point in all_points), max(point.y for point in all_points), max(point.z for point in all_points))),
    )
    proof.render_views(args.output_dir.resolve(), [source], source_root, proxies, bounds)
    print("ISSUE36_PRODUCTION_CORRESPONDENCE_RENDERED " + str(args.output_dir.resolve()))


if __name__ == "__main__":
    main()
