"""Intentional, local Atlas anatomy pass for issue #34.

This is deliberately statue-specific and runs inside an already-authored
Atlas .blend.  It keeps the globe at its normal size, relaxes only selected
upper-chest anatomy, and then makes a small controlled chest-form change.
"""

from __future__ import annotations

import argparse
from pathlib import Path


def sculpt(output: Path) -> None:
    import bpy
    import bmesh
    from mathutils import Vector

    body = bpy.data.objects.get("_H_M_Statue_Atlas_Giant_1")
    globe = bpy.data.objects.get("_H_M_Statue_Atlas_Giant_0")
    if not body or body.type != "MESH" or not globe or globe.type != "MESH":
        raise RuntimeError("expected the two Atlas high-LOD mesh objects")

    # The #33 author proof enlarged the globe about this same vertex centroid.
    # Reverse that proof-only scale before making the sculpt.
    globe_center = sum((vertex.co for vertex in globe.data.vertices), Vector()) / len(globe.data.vertices)
    for vertex in globe.data.vertices:
        vertex.co = globe_center + (vertex.co - globe_center) / 1.10
    globe.data.update()

    mesh = body.data
    bm = bmesh.new()
    bm.from_mesh(mesh)
    bm.verts.ensure_lookup_table()
    bm.edges.ensure_lookup_table()

    # Atlas's torso center in the source coordinate frame.  Limit the edit to
    # the upper body band; hands, lower legs, and the globe remain untouched.
    torso_x = 25.0
    def anatomy_weight(vertex) -> float:
        x, y, z = vertex.co
        if y < 74.0 or y > 103.0 or x < -6.0 or x > 68.0:
            return 0.0
        chest = max(0.0, 1.0 - abs(y - 91.0) / 20.0) * max(0.0, 1.0 - abs(x - torso_x) / 30.0)
        return chest

    anatomy_targets = {vertex for vertex in bm.verts if anatomy_weight(vertex) > 0.0}
    # The source high LOD has separate shell pieces at several joints.  Keep
    # those original boundaries fixed so a local edit cannot pull an overlap
    # apart and expose a seam in the game mesh.
    anatomy_interior = {
        vertex
        for vertex in anatomy_targets
        if all(edge.other_vert(vertex) in anatomy_targets for edge in vertex.link_edges)
    }

    # Selected-area relaxation: average only x/z across neighboring selected
    # vertices, preserving Atlas's raised-arm pose, topology, and vertical
    # proportions.  No global smoothing or subdivision is applied.
    for _ in range(2):
        updates = {}
        for vertex in anatomy_interior:
            weight = anatomy_weight(vertex)
            if weight <= 0.0:
                continue
            neighbors = [edge.other_vert(vertex) for edge in vertex.link_edges if anatomy_weight(edge.other_vert(vertex)) > 0.0]
            if len(neighbors) < 2:
                continue
            average = sum((neighbor.co for neighbor in neighbors), Vector()) / len(neighbors)
            updates[vertex] = (vertex.co.x * 0.78 + average.x * 0.22, vertex.co.z * 0.78 + average.z * 0.22, weight)
        for vertex, (x, z, weight) in updates.items():
            vertex.co.x = vertex.co.x * (1.0 - 0.28 * weight) + x * (0.28 * weight)
            vertex.co.z = vertex.co.z * (1.0 - 0.28 * weight) + z * (0.28 * weight)

    # Deliberate silhouette change: give the upper chest a controlled front
    # plane.  The neck/shoulder boundaries stay anchored because the source
    # statue uses separate intersecting shell pieces at those joints.
    for vertex in bm.verts:
        x, y, z = vertex.co
        if vertex in anatomy_interior and 77.0 <= y <= 103.0 and abs(x - torso_x) <= 30.0 and z > 0.0:
            chest_weight = max(0.0, 1.0 - abs(y - 90.0) / 18.0) * max(0.0, 1.0 - abs(x - torso_x) / 30.0)
            vertex.co.z += 3.5 * chest_weight

    bm.to_mesh(mesh)
    bm.free()
    for polygon in mesh.polygons:
        polygon.use_smooth = True
    mesh.update()
    body["issue34_sculpt"] = "v1: selected upper-chest relaxation and controlled front-plane form"
    globe["issue34_globe"] = "normal size restored from #33 proof scale"
    bpy.ops.wm.save_as_mainfile(filepath=str(output))
    print(f"ISSUE34_SCULPT: saved {output}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", type=Path, required=True)
    argv = __import__("sys").argv[__import__("sys").argv.index("--") + 1 :] if "--" in __import__("sys").argv else __import__("sys").argv[1:]
    args = parser.parse_args(argv)
    sculpt(args.out)


if __name__ == "__main__":
    main()
