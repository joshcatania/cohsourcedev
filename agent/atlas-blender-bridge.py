"""Issue #33: narrow Atlas high-LOD VRML/Blender bridge.

This is deliberately limited to the deterministic #31 Atlas VRML subset:
two high-LOD IndexedFaceSet Shapes, each made of triangles with explicit
coordinate, normal, UV, and index arrays.  It is not a general VRML importer.

Commands:
  py -3 agent/atlas-blender-bridge.py extract --wrl <phase-a.wrl> --obj <atlas.obj>
  blender.exe --background --python agent/atlas-blender-bridge.py -- author \
      --obj <atlas.obj> --blend <atlas.blend> --edited-obj <atlas-edited.obj>
  py -3 agent/atlas-blender-bridge.py splice --wrl <phase-a.wrl> \
      --edited-obj <atlas-edited.obj> --out <edited.wrl>
"""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path


MODEL = "_H_M_Statue_Atlas_Giant"


@dataclass
class Shape:
    name: str
    material: str
    vertices: list[tuple[float, float, float]]
    normals: list[tuple[float, float, float]]
    uvs: list[tuple[float, float]]
    faces: list[tuple[tuple[int, int, int], tuple[int, int, int], tuple[int, int, int]]]


def balanced_block(text: str, start: int) -> tuple[str, int]:
    open_at = text.index("{", start)
    depth = 0
    for index in range(open_at, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start : index + 1], index + 1
    raise ValueError("unclosed VRML block")


def array_text(block: str, marker: str) -> str:
    start = block.index(marker)
    bracket = block.index("[", start)
    end = block.index("]", bracket)
    return block[bracket + 1 : end]


def floats(text: str) -> list[float]:
    return [float(value) for value in re.findall(r"[-+]?\d+(?:\.\d*)?(?:[eE][-+]?\d+)?", text)]


def ints(text: str) -> list[int]:
    return [int(value) for value in re.findall(r"-?\d+", text)]


def vectors(text: str, width: int) -> list[tuple[float, ...]]:
    values = floats(text)
    if len(values) % width:
        raise ValueError(f"array has {len(values)} values, not divisible by {width}")
    return [tuple(values[index : index + width]) for index in range(0, len(values), width)]


def index_faces(text: str) -> list[tuple[int, int, int]]:
    values = ints(text)
    faces: list[tuple[int, int, int]] = []
    current: list[int] = []
    for value in values:
        if value == -1:
            if len(current) != 3:
                raise ValueError(f"expected triangles, got {current}")
            faces.append(tuple(current))
            current = []
        else:
            current.append(value)
    if current:
        raise ValueError("unterminated face index")
    return faces


def high_lod_shapes(wrl: str) -> tuple[str, list[Shape]]:
    model_start = wrl.index(f"DEF {MODEL} Transform")
    next_model = wrl.find("\nDEF ", model_start + 1)
    model_end = len(wrl) if next_model < 0 else next_model + 1
    model_block = wrl[model_start:model_end]
    shapes: list[Shape] = []
    cursor = 0
    shape_number = 0
    while True:
        shape_at = model_block.find("Shape {", cursor)
        if shape_at < 0:
            break
        shape_block, cursor = balanced_block(model_block, shape_at)
        geometry_at = shape_block.index("IndexedFaceSet")
        geometry_block, _ = balanced_block(shape_block, geometry_at)
        material = re.search(r'ImageTexture\s*\{\s*url\s+"([^"]+)"', shape_block)
        if not material:
            raise ValueError("high LOD Shape has no ImageTexture")
        coord = vectors(array_text(geometry_block, "Coordinate { point"), 3)
        normal = vectors(array_text(geometry_block, "Normal { vector"), 3)
        uv = vectors(array_text(geometry_block, "TextureCoordinate { point"), 2)
        coord_faces = index_faces(array_text(geometry_block, "coordIndex"))
        uv_faces = index_faces(array_text(geometry_block, "texCoordIndex"))
        normal_faces = index_faces(array_text(geometry_block, "normalIndex"))
        if not (len(coord_faces) == len(uv_faces) == len(normal_faces)):
            raise ValueError("coordinate, UV, and normal face counts differ")
        faces = [
            tuple(zip(cf, uf, nf))
            for cf, uf, nf in zip(coord_faces, uv_faces, normal_faces)
        ]
        shapes.append(
            Shape(
                name=f"{MODEL}_{shape_number}",
                material=material.group(1),
                vertices=coord,
                normals=normal,
                uvs=uv,
                faces=faces,
            )
        )
        shape_number += 1
    if len(shapes) != 2:
        raise ValueError(f"expected exactly two high LOD Shapes, got {len(shapes)}")
    return model_block, shapes


def write_obj(path: Path, shapes: list[Shape]) -> None:
    lines = ["# Issue #33 Atlas high-LOD Blender bridge OBJ", "# coordinates and winding are kept in CoH order"]
    vertex_offset = 0
    uv_offset = 0
    normal_offset = 0
    for shape in shapes:
        lines.extend([f"o {shape.name}", f"usemtl {shape.material}"])
        lines.extend(f"v {x:.9g} {y:.9g} {z:.9g}" for x, y, z in shape.vertices)
        lines.extend(f"vt {u:.9g} {v:.9g}" for u, v in shape.uvs)
        lines.extend(f"vn {x:.9g} {y:.9g} {z:.9g}" for x, y, z in shape.normals)
        for face in shape.faces:
            fields = []
            for vertex, uv, normal in face:
                fields.append(
                    f"{vertex + 1 + vertex_offset}/{uv + 1 + uv_offset}/{normal + 1 + normal_offset}"
                )
            lines.append("f " + " ".join(fields))
        vertex_offset += len(shape.vertices)
        uv_offset += len(shape.uvs)
        normal_offset += len(shape.normals)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def parse_obj(path: Path) -> list[Shape]:
    vertices: list[tuple[float, float, float]] = []
    uvs: list[tuple[float, float]] = []
    normals: list[tuple[float, float, float]] = []
    shapes: list[Shape] = []
    current: Shape | None = None

    def require_shape() -> Shape:
        if current is None:
            raise ValueError("OBJ face appeared before an object")
        return current

    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        if parts[0] in ("o", "g"):
            if current is not None:
                shapes.append(current)
            current = Shape(parts[1], "white", [], [], [], [])
        elif parts[0] == "usemtl":
            require_shape().material = parts[1]
        elif parts[0] == "v":
            vertices.append(tuple(float(value) for value in parts[1:4]))
        elif parts[0] == "vt":
            uvs.append(tuple(float(value) for value in parts[1:3]))
        elif parts[0] == "vn":
            normals.append(tuple(float(value) for value in parts[1:4]))
        elif parts[0] == "f":
            shape = require_shape()
            face = []
            for field in parts[1:]:
                vi, ti, ni = (int(value) for value in field.split("/"))
                face.append((vi - 1, ti - 1, ni - 1))
            if len(face) != 3:
                raise ValueError("edited OBJ must contain triangles")
            shape.faces.append(tuple(face))
    if current is not None:
        shapes.append(current)
    if len(shapes) != 2:
        raise ValueError(f"edited OBJ must contain two objects, got {len(shapes)}")
    for shape in shapes:
        shape.vertices = vertices
        shape.uvs = uvs
        shape.normals = normals
    return shapes


def write_vrml_array(block: str, marker: str, values: list[tuple[float, ...]] | list[tuple[int, int, int]]) -> str:
    start = block.index(marker)
    bracket = block.index("[", start)
    end = block.index("]", bracket)
    width = len(values[0]) if values else 3
    if values and isinstance(values[0][0], int):
        body = "\n".join("          " + ", ".join(str(value) for value in row) + ", -1," for row in values)
    else:
        body = "\n".join("          " + " ".join(f"{value:.9g}" for value in row) + "," for row in values)
    replacement = "[\n" + body + "\n          ]"
    return block[:bracket] + replacement + block[end + 1 :]


def replace_shape_arrays(original: str, shape: Shape, global_vertices: list[tuple[float, float, float]], global_uvs: list[tuple[float, float]], global_normals: list[tuple[float, float, float]]) -> str:
    geometry_at = original.index("IndexedFaceSet")
    geometry, geometry_end = balanced_block(original, geometry_at)
    vertex_values = [global_vertices[index] for index in sorted({face_corner[0] for face in shape.faces for face_corner in face})]
    vertex_map = {old: new for new, old in enumerate(sorted({face_corner[0] for face in shape.faces for face_corner in face}))}
    uv_values = [global_uvs[index] for index in sorted({face_corner[1] for face in shape.faces for face_corner in face})]
    uv_map = {old: new for new, old in enumerate(sorted({face_corner[1] for face in shape.faces for face_corner in face}))}
    normal_values = [global_normals[index] for index in sorted({face_corner[2] for face in shape.faces for face_corner in face})]
    normal_map = {old: new for new, old in enumerate(sorted({face_corner[2] for face in shape.faces for face_corner in face}))}
    coord_faces = [tuple(vertex_map[corner[0]] for corner in face) for face in shape.faces]
    uv_faces = [tuple(uv_map[corner[1]] for corner in face) for face in shape.faces]
    normal_faces = [tuple(normal_map[corner[2]] for corner in face) for face in shape.faces]
    updated = geometry
    updated = write_vrml_array(updated, "Coordinate { point", vertex_values)
    updated = write_vrml_array(updated, "Normal { vector", normal_values)
    updated = write_vrml_array(updated, "TextureCoordinate { point", uv_values)
    updated = write_vrml_array(updated, "coordIndex", coord_faces)
    updated = write_vrml_array(updated, "texCoordIndex", uv_faces)
    updated = write_vrml_array(updated, "normalIndex", normal_faces)
    return original[:geometry_at] + updated + original[geometry_end:]


def splice(wrl_path: Path, edited_obj_path: Path, output_path: Path) -> None:
    original = wrl_path.read_text(encoding="utf-8")
    model_block, _ = high_lod_shapes(original)
    edited = parse_obj(edited_obj_path)
    # Blender's OBJ writer uses one global index space. Split it back to the
    # two original Shapes while preserving their order and material names.
    updated_model = model_block
    global_vertices = edited[0].vertices
    global_uvs = edited[0].uvs
    global_normals = edited[0].normals
    ranges: list[tuple[int, int]] = []
    cursor = 0
    while True:
        shape_start = model_block.find("Shape {", cursor)
        if shape_start < 0:
            break
        _, shape_end = balanced_block(model_block, shape_start)
        ranges.append((shape_start, shape_end))
        cursor = shape_end
    if len(ranges) != len(edited):
        raise ValueError("original and edited Shape counts differ")
    updated_model = model_block
    # Replace from the end so the original offsets remain valid.
    for (shape_start, shape_end), shape in reversed(list(zip(ranges, edited))):
        old_shape = model_block[shape_start:shape_end]
        new_shape = replace_shape_arrays(old_shape, shape, global_vertices, global_uvs, global_normals)
        updated_model = updated_model[:shape_start] + new_shape + updated_model[shape_end:]
    model_start = original.index(f"DEF {MODEL} Transform")
    next_model = original.find("\nDEF ", model_start + 1)
    model_end = len(original) if next_model < 0 else next_model + 1
    result = original[:model_start] + updated_model + original[model_end:]
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(result, encoding="utf-8")


def blender_author(obj_path: Path, blend_path: Path, edited_obj_path: Path) -> None:
    import bpy
    from mathutils import Vector

    shapes = parse_obj(obj_path)
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for datablocks in (bpy.data.meshes, bpy.data.materials, bpy.data.cameras, bpy.data.lights):
        for datablock in list(datablocks):
            datablocks.remove(datablock)

    objects = []
    for shape_index, source in enumerate(shapes):
        # Convert global OBJ references into local mesh arrays for this Shape.
        used_vertices = sorted({corner[0] for face in source.faces for corner in face})
        vertex_map = {old: new for new, old in enumerate(used_vertices)}
        local_vertices = [source.vertices[index] for index in used_vertices]
        local_faces = [[vertex_map[corner[0]] for corner in face] for face in source.faces]
        mesh = bpy.data.meshes.new(source.name + "_Mesh")
        mesh.from_pydata(local_vertices, [], local_faces)
        for polygon in mesh.polygons:
            polygon.use_smooth = True
        mesh.update()
        uv_layer = mesh.uv_layers.new(name="UVMap")
        for polygon in mesh.polygons:
            for loop_index, corner in zip(polygon.loop_indices, source.faces[polygon.index]):
                uv_layer.data[loop_index].uv = source.uvs[corner[1]]
        material = bpy.data.materials.new(source.material)
        material.diffuse_color = (0.65, 0.65, 0.65, 1.0) if "Globe" in source.material else (0.5, 0.5, 0.5, 1.0)
        obj = bpy.data.objects.new(source.name, mesh)
        bpy.context.collection.objects.link(obj)
        obj.data.materials.append(material)
        obj["coh_material"] = source.material
        obj["issue33_proof_edit"] = "globe scaled 1.10x about local center in Blender"
        objects.append(obj)
        if shape_index == 0:
            center = sum((vertex.co for vertex in mesh.vertices), Vector()) / len(mesh.vertices)
            for vertex in mesh.vertices:
                vertex.co = center + (vertex.co - center) * 1.10
            mesh.update()

    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))

    # Export explicit per-loop UVs/normals. This avoids depending on Blender's
    # optional OBJ exporter and keeps the splice deterministic and local.
    lines = ["# Issue #33 edited by Blender 4.5.3", "# proof edit: Globe object scaled 1.10x about its own center"]
    vertex_offset = 0
    uv_offset = 0
    normal_offset = 0
    for obj in objects:
        mesh = obj.data
        mesh.update()
        mesh.calc_loop_triangles()
        lines.extend([f"o {obj.name}", f"usemtl {obj.data.materials[0].name}"])
        lines.extend(f"v {vertex.co.x:.9g} {vertex.co.y:.9g} {vertex.co.z:.9g}" for vertex in mesh.vertices)
        loop_uvs = [mesh.uv_layers.active.data[loop.index].uv for loop in mesh.loops]
        lines.extend(f"vt {uv.x:.9g} {uv.y:.9g}" for uv in loop_uvs)
        lines.extend(f"vn {loop.normal.x:.9g} {loop.normal.y:.9g} {loop.normal.z:.9g}" for loop in mesh.loops)
        for polygon in mesh.polygons:
            fields = []
            for loop_index, vertex_index in zip(polygon.loop_indices, polygon.vertices):
                fields.append(f"{vertex_index + 1 + vertex_offset}/{loop_index + 1 + uv_offset}/{loop_index + 1 + normal_offset}")
            lines.append("f " + " ".join(fields))
        vertex_offset += len(mesh.vertices)
        uv_offset += len(mesh.loops)
        normal_offset += len(mesh.loops)
    edited_obj_path.parent.mkdir(parents=True, exist_ok=True)
    edited_obj_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"ISSUE33_BLENDER: saved {blend_path}")
    print(f"ISSUE33_BLENDER: exported {edited_obj_path}")
    print("ISSUE33_BLENDER: proof edit Globe scale=1.10 center=local bounding-box centroid")


def main() -> None:
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="command", required=True)
    extract_parser = sub.add_parser("extract")
    extract_parser.add_argument("--wrl", type=Path, required=True)
    extract_parser.add_argument("--obj", type=Path, required=True)
    author_parser = sub.add_parser("author")
    author_parser.add_argument("--obj", type=Path, required=True)
    author_parser.add_argument("--blend", type=Path, required=True)
    author_parser.add_argument("--edited-obj", type=Path, required=True)
    splice_parser = sub.add_parser("splice")
    splice_parser.add_argument("--wrl", type=Path, required=True)
    splice_parser.add_argument("--edited-obj", type=Path, required=True)
    splice_parser.add_argument("--out", type=Path, required=True)
    argv = sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else sys.argv[1:]
    args = parser.parse_args(argv)
    if args.command == "extract":
        _, shapes = high_lod_shapes(args.wrl.read_text(encoding="utf-8"))
        write_obj(args.obj, shapes)
        print(f"ISSUE33_BRIDGE: extracted shapes=2 vertices={sum(len(shape.vertices) for shape in shapes)} triangles={sum(len(shape.faces) for shape in shapes)}")
    elif args.command == "author":
        blender_author(args.obj, args.blend, args.edited_obj)
    elif args.command == "splice":
        splice(args.wrl, args.edited_obj, args.out)
        print(f"ISSUE33_BRIDGE: spliced {args.out}")


if __name__ == "__main__":
    main()
