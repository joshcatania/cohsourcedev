#!/usr/bin/env python3
"""Bounded curvature-guided Atlas statue high-LOD recovery.

The input is the deterministic Phase A VRML export.  Only
_H_M_Statue_Atlas_Giant is changed; its two material shapes retain separate
edge caches so material and UV seams cannot be crossed.  Original vertices
remain anchors, while each source triangle becomes four triangles with
normal-guided edge points.  Lower LOD models and all material URLs are copied
verbatim.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import re
from pathlib import Path
from typing import Iterable, Sequence


MODEL_NAME = "_H_M_Statue_Atlas_Giant"
TARGET_TRIANGLE_LIMIT = 20_000


def matching_brace(text: str, opening: int) -> int:
    depth = 0
    for index in range(opening, len(text)):
        char = text[index]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return index
    raise ValueError(f"unclosed brace at offset {opening}")


def parse_floats(payload: str) -> list[float]:
    number = r"[-+]?(?:\d+\.\d*|\.\d+|\d+)(?:[eE][-+]?\d+)?"
    return [float(token) for token in re.findall(number, payload)]


def parse_ints(payload: str) -> list[int]:
    return [int(token) for token in re.findall(r"[-+]?\d+", payload)]


def parse_vectors(payload: str, width: int) -> list[tuple[float, ...]]:
    values = parse_floats(payload)
    if len(values) % width:
        raise ValueError(f"array has {len(values)} values, not divisible by {width}")
    return [tuple(values[index : index + width]) for index in range(0, len(values), width)]


def parse_indexed_triangles(payload: str) -> list[tuple[int, int, int]]:
    values = parse_ints(payload)
    triangles: list[tuple[int, int, int]] = []
    current: list[int] = []
    for value in values:
        if value == -1:
            if current:
                if len(current) != 3:
                    raise ValueError(f"non-triangle face has {len(current)} indices")
                triangles.append(tuple(current))
                current = []
        else:
            current.append(value)
    if current:
        if len(current) != 3:
            raise ValueError(f"unterminated face has {len(current)} indices")
        triangles.append(tuple(current))
    return triangles


def array_payload(block: str, kind: str, field: str) -> tuple[list[float], tuple[int, int]]:
    pattern = {
        "vec3": rf"{field}\s*\{{\s*(?:point|vector)\s*\[\s*(.*?)\s*\]",
        "vec2": rf"{field}\s*\{{\s*point\s*\[\s*(.*?)\s*\]",
        "index": rf"\b{field}\s*\[\s*(.*?)\s*\]",
    }[kind]
    match = re.search(pattern, block, re.DOTALL)
    if not match:
        raise ValueError(f"missing {field} array")
    return parse_ints(match.group(1)) if kind == "index" else parse_floats(match.group(1)), match.span(1)


def replace_array(block: str, kind: str, field: str, values: str) -> str:
    pattern = {
        "vec3": rf"{field}\s*\{{\s*(?:point|vector)\s*\[\s*(.*?)\s*\]",
        "vec2": rf"{field}\s*\{{\s*point\s*\[\s*(.*?)\s*\]",
        "index": rf"\b{field}\s*\[\s*(.*?)\s*\]",
    }[kind]
    match = re.search(pattern, block, re.DOTALL)
    if not match:
        raise ValueError(f"missing {field} array")
    return block[: match.start(1)] + values + block[match.end(1) :]


def fmt_float(value: float) -> str:
    if abs(value) < 5e-10:
        value = 0.0
    return format(value, ".9g")


def fmt_vectors(values: Iterable[Sequence[float]]) -> str:
    return "\n".join(
        "          " + " ".join(fmt_float(component) for component in value) + ","
        for value in values
    )


def fmt_indices(triangles: Iterable[Sequence[int]]) -> str:
    return "\n".join(
        f"          {int(triangle[0])}, {int(triangle[1])}, {int(triangle[2])}, -1,"
        for triangle in triangles
    )


def vector_add(a: Sequence[float], b: Sequence[float]) -> tuple[float, float, float]:
    return (a[0] + b[0], a[1] + b[1], a[2] + b[2])


def vector_sub(a: Sequence[float], b: Sequence[float]) -> tuple[float, float, float]:
    return (a[0] - b[0], a[1] - b[1], a[2] - b[2])


def vector_scale(a: Sequence[float], scale: float) -> tuple[float, float, float]:
    return (a[0] * scale, a[1] * scale, a[2] * scale)


def dot(a: Sequence[float], b: Sequence[float]) -> float:
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def length(a: Sequence[float]) -> float:
    return math.sqrt(dot(a, a))


def normalize(a: Sequence[float]) -> tuple[float, float, float]:
    magnitude = length(a)
    if magnitude < 1e-8:
        return (0.0, 1.0, 0.0)
    return vector_scale(a, 1.0 / magnitude)


def guided_edge(
    p0: Sequence[float],
    p1: Sequence[float],
    n0: Sequence[float],
    n1: Sequence[float],
    edge_factor: float,
    displacement_cap: float,
    normal_recovery_gain: float,
) -> tuple[tuple[float, float, float], float]:
    """Return a bounded PN-style edge point and its displacement from linear."""

    linear = vector_scale(vector_add(p0, p1), 0.5)
    edge = length(vector_sub(p1, p0))
    if edge < 1e-8:
        return (tuple(linear), 0.0)

    n0n = normalize(n0)
    n1n = normalize(n1)
    # Project the linear edge toward each endpoint's tangent plane.  Averaging
    # the two projections recovers a small amount of the surface curvature
    # implied by the authored normals without changing the anchor vertices.
    projection0 = vector_sub(linear, vector_scale(n0n, dot(vector_sub(linear, p0), n0n)))
    projection1 = vector_sub(linear, vector_scale(n1n, dot(vector_sub(linear, p1), n1n)))
    guided = vector_scale(vector_add(projection0, projection1), 0.5)
    # The authored normals describe a smoother surface than the source
    # triangle positions.  Expose that curvature signal before applying the
    # edge-relative and absolute bounds below; this is still a normal-guided
    # recovery, not free-form vertex inflation.
    delta = vector_scale(vector_sub(guided, linear), normal_recovery_gain)

    # Keep the recovery conservative relative to the local edge and the
    # statue's authored proportions.  This is deliberately not a generic
    # smoothing pass: only a bounded normal-implied correction is allowed.
    maximum = min(edge * edge_factor, displacement_cap)
    displacement = length(delta)
    if displacement > maximum:
        delta = vector_scale(delta, maximum / displacement)
        displacement = maximum
    return (vector_add(linear, delta), displacement)


def parse_shape(block: str) -> dict:
    url_match = re.search(r'ImageTexture\s*\{\s*url\s+"([^"]+)"', block)
    if not url_match:
        raise ValueError("shape has no ImageTexture URL")
    positions, _ = array_payload(block, "vec3", "Coordinate")
    normals, _ = array_payload(block, "vec3", "Normal")
    uvs, _ = array_payload(block, "vec2", "TextureCoordinate")
    coord_indices, _ = array_payload(block, "index", "coordIndex")
    normal_indices, _ = array_payload(block, "index", "normalIndex")
    uv_indices, _ = array_payload(block, "index", "texCoordIndex")
    if not isinstance(positions, list) or not isinstance(normals, list) or not isinstance(uvs, list):
        raise AssertionError("unexpected array type")
    position_vectors = [tuple(positions[index : index + 3]) for index in range(0, len(positions), 3)]
    normal_vectors = [tuple(normals[index : index + 3]) for index in range(0, len(normals), 3)]
    uv_vectors = [tuple(uvs[index : index + 2]) for index in range(0, len(uvs), 2)]
    coord_triangles = parse_indexed_triangles(",".join(str(value) for value in coord_indices))
    normal_triangles = parse_indexed_triangles(",".join(str(value) for value in normal_indices))
    uv_triangles = parse_indexed_triangles(",".join(str(value) for value in uv_indices))
    if coord_triangles != normal_triangles or coord_triangles != uv_triangles:
        raise ValueError("Phase A export does not have aligned position/normal/UV indices")
    return {
        "url": url_match.group(1),
        "block": block,
        "positions": position_vectors,
        "normals": normal_vectors,
        "uvs": uv_vectors,
        "triangles": coord_triangles,
    }


def transform_high_model(
    model_block: str,
    edge_factor: float,
    displacement_cap: float,
    normal_recovery_gain: float,
) -> tuple[str, dict]:
    shape_matches = list(re.finditer(r"\bShape\s*\{", model_block))
    if len(shape_matches) != 2:
        raise ValueError(f"expected two material shapes in {MODEL_NAME}, got {len(shape_matches)}")

    shapes: list[dict] = []
    for match in shape_matches:
        end = matching_brace(model_block, model_block.find("{", match.start()))
        shapes.append(parse_shape(model_block[match.start() : end + 1]))

    original_positions = shapes[0]["positions"]
    original_normals = shapes[0]["normals"]
    original_uvs = shapes[0]["uvs"]
    for shape in shapes[1:]:
        if shape["positions"] != original_positions or shape["normals"] != original_normals or shape["uvs"] != original_uvs:
            raise ValueError("material shapes do not share the Phase A authored arrays")

    new_positions = list(original_positions)
    new_normals = list(original_normals)
    new_uvs = list(original_uvs)
    edge_cache: dict[tuple[int, int, int], int] = {}
    new_triangles: list[list[tuple[int, int, int]]] = []
    displacements: list[float] = []

    for shape_index, shape in enumerate(shapes):
        output_triangles: list[tuple[int, int, int]] = []
        for triangle in shape["triangles"]:
            a, b, c = triangle
            corners = (a, b, c)
            edge_indices: list[int] = []
            for first, second in ((a, b), (b, c), (c, a)):
                low, high = sorted((first, second))
                key = (shape_index, low, high)
                if key not in edge_cache:
                    edge_position, displacement = guided_edge(
                        original_positions[first],
                        original_positions[second],
                        original_normals[first],
                        original_normals[second],
                        edge_factor,
                        displacement_cap,
                        normal_recovery_gain,
                    )
                    edge_cache[key] = len(new_positions)
                    new_positions.append(edge_position)
                    new_normals.append(normalize(vector_add(original_normals[first], original_normals[second])))
                    new_uvs.append(
                        (
                            (original_uvs[first][0] + original_uvs[second][0]) * 0.5,
                            (original_uvs[first][1] + original_uvs[second][1]) * 0.5,
                        )
                    )
                    displacements.append(displacement)
                edge_indices.append(edge_cache[key])

            ab, bc, ca = edge_indices
            output_triangles.extend(
                [
                    (corners[0], ab, ca),
                    (ab, corners[1], bc),
                    (ca, bc, corners[2]),
                    (ab, bc, ca),
                ]
            )
        new_triangles.append(output_triangles)

    if sum(len(triangles) for triangles in new_triangles) > TARGET_TRIANGLE_LIMIT:
        raise ValueError("high-detail pilot exceeds the 20k triangle bound")

    replacements: list[tuple[int, int, str]] = []
    for match, shape, triangles in zip(shape_matches, shapes, new_triangles):
        end = matching_brace(model_block, model_block.find("{", match.start()))
        block = model_block[match.start() : end + 1]
        block = replace_array(block, "vec3", "Coordinate", fmt_vectors(new_positions))
        block = replace_array(block, "vec3", "Normal", fmt_vectors(new_normals))
        block = replace_array(block, "vec2", "TextureCoordinate", fmt_vectors(new_uvs))
        block = replace_array(block, "index", "coordIndex", fmt_indices(triangles))
        block = replace_array(block, "index", "normalIndex", fmt_indices(triangles))
        block = replace_array(block, "index", "texCoordIndex", fmt_indices(triangles))
        replacements.append((match.start(), end + 1, block))

    output = model_block
    for start, end, block in reversed(replacements):
        output = output[:start] + block + output[end:]

    report = {
        "model": MODEL_NAME,
        "materialUrls": [shape["url"] for shape in shapes],
        "verticesBefore": len(original_positions),
        "verticesAfter": len(new_positions),
        "trianglesBefore": sum(len(shape["triangles"]) for shape in shapes),
        "trianglesAfter": sum(len(triangles) for triangles in new_triangles),
        "triangleMultiplier": 4,
        "newEdgeVertices": len(new_positions) - len(original_positions),
        "normalGuidedEdgeDisplacement": {
            "mean": (sum(displacements) / len(displacements)) if displacements else 0.0,
            "max": max(displacements) if displacements else 0.0,
            "edgeFactor": edge_factor,
            "cap": displacement_cap,
            "normalRecoveryGain": normal_recovery_gain,
        },
        "lodsUnchanged": True,
        "seams": "per-material edge cache; original UV/normal seams remain anchors",
    }
    return output, report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--report", required=True, type=Path)
    parser.add_argument("--edge-factor", type=float, default=0.32)
    parser.add_argument("--displacement-cap", type=float, default=4.0)
    parser.add_argument("--normal-recovery-gain", type=float, default=4.0)
    args = parser.parse_args()

    if (
        args.edge_factor <= 0.0
        or args.displacement_cap <= 0.0
        or args.normal_recovery_gain <= 0.0
    ):
        raise SystemExit("edge factor, displacement cap, and normal recovery gain must be positive")

    source = args.input.read_text(encoding="utf-8")
    source_hash = hashlib.sha256(args.input.read_bytes()).hexdigest().upper()
    model_match = re.search(rf"\bDEF\s+{re.escape(MODEL_NAME)}\s+Transform\s*\{{", source)
    if not model_match:
        raise SystemExit(f"model not found: {MODEL_NAME}")
    model_start = model_match.start()
    model_open = source.find("{", model_match.start())
    model_end = matching_brace(source, model_open)
    transformed, geometry_report = transform_high_model(
        source[model_start : model_end + 1],
        args.edge_factor,
        args.displacement_cap,
        args.normal_recovery_gain,
    )
    output_text = source[:model_start] + transformed + source[model_end + 1 :]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(output_text, encoding="utf-8", newline="\n")
    output_hash = hashlib.sha256(args.output.read_bytes()).hexdigest().upper()

    report = {
        "schema": "coh.atlas-statue-geometry-pilot.v1",
        "source": str(args.input),
        "sourceSha256": source_hash,
        "output": str(args.output),
        "outputSha256": output_hash,
        "method": "normal-guided bounded edge recovery with 4-way triangle subdivision",
        "textureAndMaterialPolicy": "material URLs and per-shape ownership copied; textures untouched",
        "geometry": geometry_report,
    }
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8", newline="\n")
    print(json.dumps(report, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
