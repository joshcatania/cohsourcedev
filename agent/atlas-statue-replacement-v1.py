"""Build the issue #35 Atlas replacement scene from new Blender-native forms.

The source Atlas OBJ is used only to recreate the hidden reference mannequin
and the normal-size globe.  The replacement body is built from fresh,
overlapping anatomical primitives and voxel-remeshed into one contiguous
mesh before export through atlas-blender-bridge.py.

Run with Blender 4.5:

    blender.exe --background --python agent/atlas-statue-replacement-v1.py -- \
        --source-obj <atlas-reference.obj> --blend <atlas-replacement.blend> \
        --compare <original-vs-replacement.png> --stats <stats.json>
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path


MODEL = "_H_M_Statue_Atlas_Giant"
BODY_MATERIAL = "X_Male_Statue_Atlas_01"
GLOBE_MATERIAL = "X_Male_Statue_Atlas_Globe_01"


def parse_source_obj(path: Path):
    """Read the narrow two-shape OBJ written by the existing bridge."""
    vertices = []
    uvs = []
    shapes = []
    current = None
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        fields = line.split()
        kind = fields[0]
        if kind == "o":
            current = {"name": fields[1], "material": "white", "faces": []}
            shapes.append(current)
        elif kind == "usemtl":
            current["material"] = fields[1]
        elif kind == "v":
            vertices.append(tuple(float(value) for value in fields[1:4]))
        elif kind == "vt":
            uvs.append(tuple(float(value) for value in fields[1:3]))
        elif kind == "f":
            current["faces"].append(
                [tuple(int(value) - 1 for value in corner.split("/")) for corner in fields[1:]]
            )
    if len(shapes) != 2:
        raise RuntimeError(f"expected two source shapes, got {len(shapes)}")
    for shape in shapes:
        used_vertices = sorted({corner[0] for face in shape["faces"] for corner in face})
        vertex_map = {old: new for new, old in enumerate(used_vertices)}
        used_uvs = sorted({corner[1] for face in shape["faces"] for corner in face})
        uv_map = {old: new for new, old in enumerate(used_uvs)}
        shape["vertices"] = [vertices[index] for index in used_vertices]
        shape["uvs"] = [uvs[index] for index in used_uvs]
        shape["faces"] = [
            [(vertex_map[v], uv_map[uv], normal) for v, uv, normal in face]
            for face in shape["faces"]
        ]
    return shapes


def clear_scene(bpy):
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for datablocks in (bpy.data.meshes, bpy.data.curves, bpy.data.metaballs,
                       bpy.data.materials, bpy.data.cameras, bpy.data.lights):
        for datablock in list(datablocks):
            datablocks.remove(datablock)


def material(bpy, name: str, color):
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = (*color, 1.0)
    mat.use_nodes = True
    principled = mat.node_tree.nodes.get("Principled BSDF")
    principled.inputs["Base Color"].default_value = (*color, 1.0)
    principled.inputs["Roughness"].default_value = 0.72
    return mat


def source_mesh(bpy, shape, mat, name: str):
    mesh = bpy.data.meshes.new(name + "_Mesh")
    mesh.from_pydata(shape["vertices"], [], [[corner[0] for corner in face] for face in shape["faces"]])
    mesh.update()
    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon, face in zip(mesh.polygons, shape["faces"]):
        for loop_index, corner in zip(polygon.loop_indices, face):
            uv_layer.data[loop_index].uv = shape["uvs"][corner[1]]
    for polygon in mesh.polygons:
        polygon.use_smooth = True
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    obj.data.materials.append(mat)
    obj["coh_material"] = mat.name
    return obj


def add_uv_sphere(bpy, parts, location, scale, segments=28, rings=18, name="part"):
    bpy.ops.mesh.primitive_uv_sphere_add(
        segments=segments, ring_count=rings, location=location
    )
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    parts.append(obj)
    return obj


def add_capsule(bpy, parts, start, end, radius, name):
    from mathutils import Vector

    a = Vector(start)
    b = Vector(end)
    delta = b - a
    length = delta.length
    if length < 0.01:
        return add_uv_sphere(bpy, parts, a, (radius, radius, radius), name=name)
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=28, radius=radius, depth=length, location=(a + b) * 0.5
    )
    obj = bpy.context.object
    obj.name = name
    obj.rotation_mode = "QUATERNION"
    obj.rotation_quaternion = delta.to_track_quat("Z", "Y")
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    parts.append(obj)
    add_uv_sphere(bpy, parts, a, (radius * 1.02, radius * 1.02, radius * 1.02), name=name + "_joint_a")
    add_uv_sphere(bpy, parts, b, (radius * 1.02, radius * 1.02, radius * 1.02), name=name + "_joint_b")
    return obj


def build_new_body(bpy, mat, revision: int = 1):
    """Create a fresh raised-globe Atlas mannequin and make it contiguous."""
    parts = []

    # Atlas-specific silhouette landmarks in the original CoH coordinate
    # frame.  These are pose/placement references, not copied source vertices.
    # The broad chest, deltoids, neck, and head are intentionally authored as
    # new overlapping forms before the union/remesh.
    add_uv_sphere(bpy, parts, (28, 82, 9), (24, 17, 22), name="new_ribcage")
    add_uv_sphere(bpy, parts, (28, 96, 10), (28, 18, 18), name="new_upper_torso")
    add_uv_sphere(bpy, parts, (28, 61, 7), (22, 15, 17), name="new_pelvis")
    add_uv_sphere(bpy, parts, (28, 105, 8), (31, 14, 12), name="new_trapezius")

    # Deliberate pectoral masses create a modern chest plane rather than a
    # smooth balloon torso.  They overlap the ribcage so remeshing joins them.
    add_uv_sphere(bpy, parts, (16, 94, 24), (15, 9, 8), name="new_pectoral_l")
    add_uv_sphere(bpy, parts, (40, 94, 24), (15, 9, 8), name="new_pectoral_r")
    add_uv_sphere(bpy, parts, (28, 88, 25), (7, 7, 6), name="new_sternum_plane")

    # Neck, head, jaw, brow, and nose are separate new forms that overlap the
    # torso.  This is intentionally a fresh head design, not an edit of the
    # old head shell.
    add_capsule(bpy, parts, (28, 102, 8), (28, 119, 8), 10.5, "new_neck")
    add_uv_sphere(bpy, parts, (28, 128, 8), (15, 16, 14), name="new_head")
    add_uv_sphere(bpy, parts, (28, 119, 19), (11, 8, 7), name="new_jaw")
    add_uv_sphere(bpy, parts, (21, 131, 20), (6, 5, 4), name="new_brow_l")
    add_uv_sphere(bpy, parts, (35, 131, 20), (6, 5, 4), name="new_brow_r")
    add_uv_sphere(bpy, parts, (28, 126, 23), (3.5, 4, 5), name="new_nose")

    # Raised arms preserve the iconic globe relationship while giving the
    # shoulder-to-neck transition a single intentional mass.
    add_uv_sphere(bpy, parts, (4, 105, 8), (13, 13, 13), name="new_deltoid_l")
    add_uv_sphere(bpy, parts, (52, 105, 8), (13, 13, 13), name="new_deltoid_r")
    add_capsule(bpy, parts, (2, 105, 8), (-13, 119, 5), 9.0, "new_upper_arm_l")
    add_capsule(bpy, parts, (-13, 119, 5), (-9, 134, 9), 7.2, "new_forearm_l")
    add_uv_sphere(bpy, parts, (-8, 135, 10), (8, 7, 7), name="new_hand_l")
    add_capsule(bpy, parts, (54, 105, 8), (70, 119, 11), 9.4, "new_upper_arm_r")
    add_capsule(bpy, parts, (70, 119, 11), (78, 134, 14), 7.4, "new_forearm_r")
    add_uv_sphere(bpy, parts, (80, 135, 15), (8, 7, 7), name="new_hand_r")

    # Athletic legs: broad quads, explicit knees, and cleaner tapered calves.
    add_capsule(bpy, parts, (15, 58, 7), (8, 32, 3), 13.0, "new_thigh_l")
    add_uv_sphere(bpy, parts, (8, 31, 3), (11, 10, 10), name="new_knee_l")
    add_capsule(bpy, parts, (8, 30, 3), (3, 8, 0), 9.0, "new_calf_l")
    add_uv_sphere(bpy, parts, (3, 5, 1), (9, 7, 11), name="new_foot_l")
    add_capsule(bpy, parts, (41, 58, 8), (53, 32, 10), 13.5, "new_thigh_r")
    add_uv_sphere(bpy, parts, (53, 31, 10), (11, 10, 10), name="new_knee_r")
    add_capsule(bpy, parts, (53, 30, 10), (66, 8, 13), 9.4, "new_calf_r")
    add_uv_sphere(bpy, parts, (68, 5, 14), (10, 7, 12), name="new_foot_r")

    # A slightly wider shoulder/chest revision is useful for the first runtime
    # candidate because the in-game statue is viewed from a distance.
    if revision >= 2:
        add_uv_sphere(bpy, parts, (28, 101, 9), (35, 15, 13), name="revision_2_shoulder_bridge")
        add_uv_sphere(bpy, parts, (28, 91, 25), (32, 8, 5), name="revision_2_chest_plane")
        add_uv_sphere(bpy, parts, (28, 78, 12), (27, 18, 23), name="revision_2_abdominal_plane")

    bpy.ops.object.select_all(action="DESELECT")
    for part in parts:
        part.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    bpy.ops.object.join()
    body = bpy.context.object
    body.name = MODEL + "_1"

    remesh = body.modifiers.new("Issue35_ContiguousVoxelRemesh", "REMESH")
    remesh.mode = "VOXEL"
    remesh.voxel_size = 2.20 if revision == 0 else (1.55 if revision == 1 else 1.35)
    remesh.use_smooth_shade = True
    bpy.context.view_layer.objects.active = body
    bpy.ops.object.modifier_apply(modifier=remesh.name)

    # One light smoothing pass after the union removes voxel stair-stepping
    # without reintroducing disconnected source topology.
    smooth = body.modifiers.new("Issue35_SurfaceRelax", "SMOOTH")
    smooth.factor = 0.20 if revision == 0 else (0.24 if revision == 1 else 0.30)
    smooth.iterations = 1 if revision == 0 else (2 if revision == 1 else 5)
    bpy.ops.object.modifier_apply(modifier=smooth.name)

    if revision >= 2:
        # Subdivision softens the remesh transition at the major joints.  A
        # bounded decimate pass keeps the result in the requested experimental
        # triangle range while retaining the new large-form silhouette.
        subdiv = body.modifiers.new("Issue35_AnatomySubdivision", "SUBSURF")
        subdiv.subdivision_type = "CATMULL_CLARK"
        subdiv.levels = 1
        subdiv.render_levels = 1
        bpy.ops.object.modifier_apply(modifier=subdiv.name)
        decimate = body.modifiers.new("Issue35_ExperimentalBudget", "DECIMATE")
        decimate.ratio = 0.52
        bpy.ops.object.modifier_apply(modifier=decimate.name)

    body.data.materials.clear()
    body.data.materials.append(mat)
    body["coh_material"] = BODY_MATERIAL
    body["issue35_construction"] = "new primitive anatomy union -> voxel remesh -> light surface relax"
    body["issue35_revision"] = revision
    for polygon in body.data.polygons:
        polygon.use_smooth = True
    body.data.update()

    # The first primitive is the join active object, so Blender keeps its
    # location as an object transform.  The existing bridge serializes mesh
    # local coordinates and does not serialize object transforms; bake this
    # placement now or the CoH WRL would receive a body at the wrong origin.
    bpy.context.view_layer.objects.active = body
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)

    # The bridge requires an explicit active per-loop UV layer.  A fresh smart
    # unwrap is sufficient for v1; the old UV layout is deliberately not used.
    bpy.context.view_layer.objects.active = body
    body.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.uv.smart_project(angle_limit=math.radians(66), island_margin=0.02)
    bpy.ops.object.mode_set(mode="OBJECT")
    return body


def project_uv_from_reference(body, reference_body):
    """Project the old atlas UVs onto the new surface as a runtime aid."""
    from mathutils import Vector
    from mathutils.bvhtree import BVHTree

    ref_mesh = reference_body.data
    ref_uv = ref_mesh.uv_layers.active
    if ref_uv is None:
        return False
    ref_vertices = [vertex.co.copy() for vertex in ref_mesh.vertices]
    ref_polygons = [[vertex_index for vertex_index in polygon.vertices] for polygon in ref_mesh.polygons]
    tree = BVHTree.FromPolygons(ref_vertices, ref_polygons, all_triangles=True)
    uv_layer = body.data.uv_layers.active
    if uv_layer is None:
        return False

    def barycentric(point, a, b, c):
        v0 = b - a
        v1 = c - a
        v2 = point - a
        d00 = v0.dot(v0)
        d01 = v0.dot(v1)
        d11 = v1.dot(v1)
        d20 = v2.dot(v0)
        d21 = v2.dot(v1)
        denom = d00 * d11 - d01 * d01
        if abs(denom) < 1e-8:
            return 1.0, 0.0, 0.0
        v = (d11 * d20 - d01 * d21) / denom
        w = (d00 * d21 - d01 * d20) / denom
        return 1.0 - v - w, v, w

    for loop in body.data.loops:
        point = body.data.vertices[loop.vertex_index].co
        nearest, _normal, polygon_index, _distance = tree.find_nearest(point)
        if polygon_index < 0 or nearest is None:
            uv_layer.data[loop.index].uv = (0.5, 0.5)
            continue
        polygon = ref_mesh.polygons[polygon_index]
        if len(polygon.vertices) != 3:
            uv_layer.data[loop.index].uv = (0.5, 0.5)
            continue
        positions = [ref_mesh.vertices[index].co for index in polygon.vertices]
        uvs = [ref_uv.data[index].uv.copy() for index in polygon.loop_indices]
        weights = barycentric(nearest, positions[0], positions[1], positions[2])
        projected = uvs[0] * weights[0] + uvs[1] * weights[1] + uvs[2] * weights[2]
        uv_layer.data[loop.index].uv = projected
    body["issue35_uvs"] = "nearest-surface projection from hidden original Atlas body UV donor"
    return True


def flip_winding(body, bpy):
    """Match the source WRL's face winding convention for a fresh mesh."""
    bpy.context.view_layer.objects.active = body
    body.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.mesh.flip_normals()
    bpy.ops.object.mode_set(mode="OBJECT")
    body["issue35_winding"] = "flipped for CoH runtime face-winding convention"


def duplicate_for_neutral(bpy, source, material_obj, offset_x, name):
    obj = source.copy()
    obj.data = source.data.copy()
    obj.name = name
    obj.location.x += offset_x
    obj.data.materials.clear()
    obj.data.materials.append(material_obj)
    obj.hide_render = False
    obj.hide_viewport = False
    bpy.context.collection.objects.link(obj)
    return obj


def make_camera_and_lights(bpy, center=(28, 135, 5), wide=False):
    from mathutils import Vector

    bpy.ops.object.camera_add(location=(28, 145, 560 if not wide else 720))
    camera = bpy.context.object
    camera.name = "Issue35_ComparisonCamera"
    camera.data.type = "ORTHO"
    # Blender's orthographic scale is the horizontal frame width; account for
    # the 900x620 output aspect so the full 0..292 Atlas height remains visible.
    camera.data.ortho_scale = 360 if not wide else 485
    target = Vector(center)
    camera.rotation_euler = (target - camera.location).to_track_quat("-Z", "Y").to_euler()
    # Keep CoH's Y-up composition readable in the neutral board; the raw
    # camera track quaternion presents the statue upside-down in this view.
    camera.rotation_euler.rotate_axis("Z", math.pi)
    bpy.context.scene.camera = camera

    for name, location, energy, size in (
        ("Key", (-110, 260, 300), 3200, 170),
        ("Fill", (170, 180, 240), 2400, 210),
        ("Rim", (40, 330, -240), 2800, 150),
    ):
        bpy.ops.object.light_add(type="AREA", location=location)
        light = bpy.context.object
        light.name = "Issue35_" + name
        light.data.energy = energy
        light.data.shape = "DISK"
        light.data.size = size
        light.rotation_euler = (Vector(center) - light.location).to_track_quat("-Z", "Y").to_euler()
    world = bpy.context.scene.world
    world.color = (0.09, 0.09, 0.09)
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.09, 0.09, 0.09, 1)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.6


def render_neutral_comparison(bpy, reference_body, reference_globe, body, globe, output: Path):
    gray = material(bpy, "Issue35_Neutral_Gray", (0.60, 0.63, 0.66))
    dark_gray = material(bpy, "Issue35_Neutral_Reference_Gray", (0.38, 0.41, 0.44))
    ref_body = duplicate_for_neutral(bpy, reference_body, dark_gray, -125, "Neutral_Original_Atlas")
    ref_globe = duplicate_for_neutral(bpy, reference_globe, dark_gray, -125, "Neutral_Original_Globe")
    new_body = duplicate_for_neutral(bpy, body, gray, 125, "Neutral_Replacement_Atlas")
    new_globe = duplicate_for_neutral(bpy, globe, gray, 125, "Neutral_Replacement_Globe")
    for obj in (reference_body, reference_globe, body, globe):
        obj.hide_render = True
    make_camera_and_lights(bpy, center=(28, 140, 5), wide=True)
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_WORKBENCH"
    scene.display.shading.light = "STUDIO"
    scene.display.shading.studio_light = "paint.sl"
    scene.display.shading.color_type = "MATERIAL"
    scene.display.shading.show_shadows = True
    scene.display.shading.show_cavity = True
    scene.display.shading.cavity_type = "BOTH"
    scene.display.shading.curvature_ridge_factor = 1.5
    scene.display.shading.curvature_valley_factor = 1.0
    scene.display.shading.background_type = "WORLD"
    scene.display.shading.show_specular_highlight = True
    scene.render.resolution_x = 900
    scene.render.resolution_y = 620
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = str(output)
    scene.render.film_transparent = False
    bpy.ops.render.render(write_still=True)
    for obj in (ref_body, ref_globe, new_body, new_globe):
        bpy.data.objects.remove(obj, do_unlink=True)
    for obj in list(bpy.data.objects):
        if obj.name.startswith("Issue35_"):
            bpy.data.objects.remove(obj, do_unlink=True)
    for obj in (reference_body, reference_globe, body, globe):
        obj.hide_render = False


def stats_for_object(body, globe):
    def one(obj):
        return {
            "object": obj.name,
            "vertices": len(obj.data.vertices),
            "polygons": len(obj.data.polygons),
            "triangles": sum(len(poly.vertices) - 2 for poly in obj.data.polygons),
            "material": obj.get("coh_material", obj.data.materials[0].name if obj.data.materials else None),
            "uv_layers": len(obj.data.uv_layers),
        }
    return {"body": one(body), "globe": one(globe), "total_triangles": one(body)["triangles"] + one(globe)["triangles"]}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-obj", type=Path, required=True)
    parser.add_argument("--blend", type=Path, required=True)
    parser.add_argument("--compare", type=Path, required=True)
    parser.add_argument("--stats", type=Path, required=True)
    parser.add_argument("--revision", type=int, default=1)
    parser.add_argument("--flip-winding", action="store_true")
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else sys.argv[1:]
    args = parser.parse_args(argv)

    import bpy

    clear_scene(bpy)
    shapes = parse_source_obj(args.source_obj)
    globe_mat = material(bpy, GLOBE_MATERIAL, (0.56, 0.58, 0.60))
    body_mat = material(bpy, BODY_MATERIAL, (0.42, 0.44, 0.47))
    reference_globe = source_mesh(bpy, shapes[0], globe_mat, "Atlas_Reference_Globe")
    reference_body = source_mesh(bpy, shapes[1], body_mat, "Atlas_Reference_Body")
    reference_body.hide_viewport = True
    reference_body.hide_render = True
    reference_body["issue35_role"] = "reference mannequin only; never exported"
    reference_globe["issue35_role"] = "normal-size globe retained as runtime globe"
    body = build_new_body(bpy, body_mat, args.revision)
    if args.flip_winding:
        flip_winding(body, bpy)
    project_uv_from_reference(body, reference_body)
    body.name = MODEL + "_1"
    globe = reference_globe.copy()
    globe.data = reference_globe.data.copy()
    globe.name = MODEL + "_0"
    globe.data.materials.clear()
    globe.data.materials.append(globe_mat)
    globe["coh_material"] = GLOBE_MATERIAL
    bpy.context.collection.objects.link(globe)
    globe.hide_viewport = False
    globe.hide_render = False
    reference_globe.hide_viewport = True
    reference_globe.hide_render = True

    # Save a clean working scene with the original mannequin hidden and the
    # replacement pair carrying the exact bridge object names.
    args.blend.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(args.blend))

    render_neutral_comparison(bpy, reference_body, reference_globe, body, globe, args.compare)
    result = stats_for_object(body, globe)
    result["blend"] = str(args.blend)
    result["compare"] = str(args.compare)
    result["construction"] = "fresh overlapping primitive anatomy, voxel remesh, smooth relax, smart UV unwrap"
    args.stats.parent.mkdir(parents=True, exist_ok=True)
    args.stats.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print("ISSUE35_REPLACEMENT: " + json.dumps(result, sort_keys=True))


if __name__ == "__main__":
    main()
