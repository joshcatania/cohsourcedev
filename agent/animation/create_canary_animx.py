"""Author a small player canary directly from a GetAnimation2 rig report.

This is the no-Blender fallback for the feasibility spike.  It uses the same
local-to-world and game-to-ANIMX conversions as the runtime exporter, so the
result is a compact authored source fixture rather than a hand-edited binary.
The script intentionally animates only existing Male bones; the base skeleton
supplies every other bone at runtime.
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path


Vec3 = list[float]
Quat = list[float]


def add(a: Vec3, b: Vec3) -> Vec3:
    return [a[i] + b[i] for i in range(3)]


def scale(a: Vec3, value: float) -> Vec3:
    return [component * value for component in a]


def quat_normalize(q: Quat) -> Quat:
    magnitude = math.sqrt(sum(component * component for component in q))
    if magnitude <= 1.0e-12:
        return [0.0, 0.0, 0.0, 1.0]
    return [component / magnitude for component in q]


def quat_multiply(left: Quat, right: Quat) -> Quat:
    # Matches the engine's quatMultiply(local, parent, world) convention.
    lx, ly, lz, lw = left
    rx, ry, rz, rw = right
    return [
        lw * rx + lx * rw + ly * rz - lz * ry,
        lw * ry - lx * rz + ly * rw + lz * rx,
        lw * rz + lx * ry - ly * rx + lz * rw,
        lw * rw - lx * rx - ly * ry - lz * rz,
    ]


def quat_rotate(q: Quat, vector: Vec3) -> Vec3:
    pure = [vector[0], vector[1], vector[2], 0.0]
    inverse = [-q[0], -q[1], -q[2], q[3]]
    return quat_multiply(quat_multiply(q, pure), inverse)[:3]


def quat_from_axis_angle(axis: Vec3, angle: float) -> Quat:
    half = angle * 0.5
    sine = math.sin(half)
    return [axis[0] * sine, axis[1] * sine, axis[2] * sine, math.cos(half)]


def local_delta(base: Quat, axis: Vec3, angle: float) -> Quat:
    return quat_normalize(quat_multiply(quat_from_axis_angle(axis, angle), base))


def game_quat_to_animx(q: Quat) -> tuple[Vec3, float]:
    q = quat_normalize(q)
    if q[3] < 0.0:
        q = [-component for component in q]
    angle = 2.0 * math.acos(max(-1.0, min(1.0, q[3])))
    sine = math.sin(angle * 0.5)
    if abs(sine) <= 1.0e-6:
        return [0.0, 1.0, 0.0], 0.0
    game_axis = [q[0] / sine, q[1] / sine, q[2] / sine]
    # Exact inverse of process_animx.c's source-axis conversion plus angle sign.
    source_axis = [game_axis[0], game_axis[2], -game_axis[1]]
    return source_axis, angle


def game_position_to_animx(position: Vec3) -> Vec3:
    # Exact inverse of ConvertCoordsFrom3DSMAX in processanim.c.
    return [-position[0], -position[2], position[1]]


def load_rig(path: Path):
    report = json.loads(path.read_text(encoding="utf-8"))
    bones = {bone["name"]: bone for bone in report["bones"]}
    for bone in bones.values():
        bone["baseRotation"] = bone["frame0LocalRotation"]
        bone["baseTranslation"] = bone["frame0LocalTranslation"]
    return report, bones


def authored_local_pose(bones, frame: int, total_frames: int):
    phase = 2.0 * math.pi * (frame - 1) / max(1, total_frames - 1)
    wave = math.sin(phase)
    lift = math.sin(phase * 2.0)
    local = {
        name: (list(bone["baseRotation"]), list(bone["baseTranslation"]))
        for name, bone in bones.items()
    }

    # A deliberately obvious, symmetric upper-body canary pose.
    local["CHEST"] = (local["CHEST"][0], add(local["CHEST"][1], [0.0, 0.02 * lift, 0.0]))
    local["CHEST"] = (local_delta(local["CHEST"][0], [0.0, 0.0, 1.0], math.radians(7.0) * wave), local["CHEST"][1])
    local["UARMR"] = (local_delta(local["UARMR"][0], [1.0, 0.0, 0.0], math.radians(42.0) * wave), local["UARMR"][1])
    local["UARML"] = (local_delta(local["UARML"][0], [1.0, 0.0, 0.0], math.radians(-42.0) * wave), local["UARML"][1])
    local["LARMR"] = (local_delta(local["LARMR"][0], [1.0, 0.0, 0.0], math.radians(-65.0) * wave), local["LARMR"][1])
    local["LARML"] = (local_delta(local["LARML"][0], [1.0, 0.0, 0.0], math.radians(65.0) * wave), local["LARML"][1])
    local["ULEGR"] = (local_delta(local["ULEGR"][0], [1.0, 0.0, 0.0], math.radians(12.0) * lift), local["ULEGR"][1])
    local["ULEGL"] = (local_delta(local["ULEGL"][0], [1.0, 0.0, 0.0], math.radians(-12.0) * lift), local["ULEGL"][1])
    local["HEAD"] = (local_delta(local["HEAD"][0], [0.0, 0.0, 1.0], math.radians(5.0) * wave), local["HEAD"][1])
    return local


def world_pose(bones, local):
    world = {}
    pending = set(bones)
    while pending:
        progressed = False
        for name in list(pending):
            bone = bones[name]
            parent_id = bone["parent"]
            parent_name = next((candidate for candidate, value in bones.items() if value["id"] == parent_id), None)
            rotation, translation = local[name]
            if parent_name is None:
                world[name] = (rotation, translation)
            elif parent_name in world:
                parent_rotation, parent_translation = world[parent_name]
                world[name] = (
                    quat_multiply(rotation, parent_rotation),
                    add(parent_translation, quat_rotate(parent_rotation, translation)),
                )
            else:
                continue
            pending.remove(name)
            progressed = True
        if not progressed:
            raise ValueError("Rig hierarchy contains an unresolved parent")
    return world


def write_animx(report, bones, output: Path, total_frames: int):
    # Keep the source fixture compact: fallback bones come from the base anim.
    # The compiler converts authored world transforms to local transforms
    # bottom-up, so every authored bone's parent must also have a channel.
    authored = [
        "HIPS", "WAIST", "CHEST", "COL_R", "UARMR", "LARMR",
        "COL_L", "UARML", "LARML", "ULEGR", "ULEGL", "NECK", "HEAD",
    ]
    lines = [
        "# CoH authored animation canary generated from a runtime rig report",
        "# Direct ANIMX source fixture; no production player data is modified.",
        "",
        "Version 200",
        "SourceName male/cohsourcedev_custom_canary",
        f"TotalFrames {total_frames}",
        "FirstFrame 0",
        "",
    ]
    for bone_name in authored:
        lines.extend([f'Bone "{bone_name}"', "{"])
        for frame in range(1, total_frames + 1):
            local = authored_local_pose(bones, frame, total_frames)
            world = world_pose(bones, local)
            rotation, translation = world[bone_name]
            axis, angle = game_quat_to_animx(rotation)
            source_translation = game_position_to_animx(translation)
            lines.extend([
                "        Transform",
                "        {",
                f"            Axis {axis[0]:.9g} {axis[1]:.9g} {axis[2]:.9g}",
                f"            Angle {angle:.9g}",
                f"            Translation {source_translation[0]:.9g} {source_translation[1]:.9g} {source_translation[2]:.9g}",
                "            Scale 1 1 1",
                "        }",
                "",
            ])
        lines.extend(["}", ""])
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines), encoding="utf-8", newline="\n")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--rig-json", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--frames", type=int, default=36)
    args = parser.parse_args()
    if args.frames < 2:
        parser.error("--frames must be at least 2")
    report, bones = load_rig(args.rig_json)
    write_animx(report, bones, args.output, args.frames)
    print(f"Wrote {args.output} ({args.frames} frames, 13 authored bones, base {report['animation']})")


if __name__ == "__main__":
    main()
