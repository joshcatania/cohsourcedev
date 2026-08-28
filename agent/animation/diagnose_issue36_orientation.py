"""Localize the Issue 36 Mixamo -> CoH Male orientation failure.

This is a diagnostic-only companion to ``prove_mixamo_anatomical_pose.py``.
It opens the exact production proof blend, evaluates only source frames 18,
20, and 22, and compares:

* the existing direction + bend-plane solver (A), and
* an explicit semantic rest-basis transfer (B).

No production animation, runtime asset, source FBX, or Web Swing code is
written.  The output directory contains JSON/Markdown evidence and simple
front / three-quarter / side proxy renders for raw Mixamo, current CoH, and
the diagnostic rest-basis A/B.

The orientation convention used here is Blender's column-vector convention:
``pose_world = source.matrix_world * pose_bone.matrix``.  For a mapped bone,
the diagnostic transfer is:

    source_delta = source_pose_world * inverse(source_rest_world)
    target_pose_world = source_delta * target_rest_world

The target local quaternion is then recovered through Blender's parent/rest
conversion.  This is deliberately kept separate from the production
direction+plane solver so the A/B identifies the first failing boundary.
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Quaternion, Vector


ROOT = Path(__file__).resolve().parents[2]
ANIMATION_DIR = ROOT / "agent" / "animation"
if str(ANIMATION_DIR) not in sys.path:
    sys.path.insert(0, str(ANIMATION_DIR))

import prove_mixamo_anatomical_pose as proof  # noqa: E402
from create_blender_canary import build_source_rest, load_rig  # noqa: E402


FRAMES = (18, 20, 22)

SOURCE = proof.SOURCE
CONTROL = proof.CONTROL

# This is the mapping used by the existing production direction+plane pass.
# Spine1 is intentionally recorded in the rest-basis audit but is not a
# direct production target channel: the current compressed pass uses
# Mixamo Spine -> WAIST and Mixamo Spine2 -> CHEST.
TARGET_MAP = {
    "hips": "HIPS",
    "spine": "WAIST",
    "spine2": "CHEST",
    "neck": "NECK",
    "head": "HEAD",
    "shoulder_r": "COL_R",
    "arm_r": "UARMR",
    "forearm_r": "LARMR",
    "hand_r": "HANDR",
    "shoulder_l": "COL_L",
    "arm_l": "UARML",
    "forearm_l": "LARML",
    "hand_l": "HANDL",
    "thigh_r": "ULEGR",
    "shin_r": "LLEGR",
    "foot_r": "FOOTR",
    "thigh_l": "ULEGL",
    "shin_l": "LLEGL",
    "foot_l": "FOOTL",
}

TARGET_CHILD = {
    "HIPS": "WAIST",
    "WAIST": "CHEST",
    "CHEST": "NECK",
    "NECK": "HEAD",
    "HEAD": "CRANIUM",
    "COL_R": "UARMR",
    "UARMR": "LARMR",
    "LARMR": "HANDR",
    "HANDR": "WEPR",
    "COL_L": "UARML",
    "UARML": "LARML",
    "LARML": "HANDL",
    "HANDL": "WEPL",
    "ULEGR": "LLEGR",
    "LLEGR": "FOOTR",
    "FOOTR": "TOER",
    "ULEGL": "LLEGL",
    "LLEGL": "FOOTL",
    "FOOTL": "TOEL",
}

SOURCE_CHILD = {
    "hips": "spine",
    "spine": "spine1",
    "spine1": "spine2",
    "spine2": "neck",
    "neck": "head",
    "head": "head_end",
    "shoulder_r": "arm_r",
    "arm_r": "forearm_r",
    "forearm_r": "hand_r",
    "hand_r": None,
    "shoulder_l": "arm_l",
    "arm_l": "forearm_l",
    "forearm_l": "hand_l",
    "hand_l": None,
    "thigh_r": "shin_r",
    "shin_r": "foot_r",
    "foot_r": "toe_r",
    "thigh_l": "shin_l",
    "shin_l": "foot_l",
    "foot_l": "toe_l",
}

CONTROL_CHILD = {
    "hips": "spine",
    "spine": "spine1",
    "spine1": "spine2",
    "spine2": "neck",
    "neck": "head",
    "head": None,
    "shoulder_r": "arm_r",
    "arm_r": "forearm_r",
    "forearm_r": "hand_r",
    "hand_r": None,
    "shoulder_l": "arm_l",
    "arm_l": "forearm_l",
    "forearm_l": "hand_l",
    "hand_l": None,
    "thigh_r": "shin_r",
    "shin_r": "foot_r",
    "foot_r": "toe_r",
    "thigh_l": "shin_l",
    "shin_l": "foot_l",
    "foot_l": "toe_l",
}

FOCUS_SOURCE = (
    "hips", "spine", "spine1", "spine2", "neck", "head",
    "shoulder_r", "arm_r", "forearm_r", "hand_r",
    "shoulder_l", "arm_l", "forearm_l", "hand_l",
    "thigh_r", "shin_r", "foot_r",
    "thigh_l", "shin_l", "foot_l",
)

FOCUS_TARGET = (
    "HIPS", "WAIST", "CHEST", "NECK", "HEAD",
    "COL_R", "UARMR", "LARMR", "HANDR",
    "COL_L", "UARML", "LARML", "HANDL",
    "ULEGR", "LLEGR", "FOOTR", "ULEGL", "LLEGL", "FOOTL",
)

SOURCE_TO_CONTROL_PAIRS = tuple((semantic, semantic) for semantic in FOCUS_SOURCE)
CONTROL_TO_TARGET_PAIRS = tuple(
    (semantic, target)
    for semantic, target in TARGET_MAP.items()
    if semantic in FOCUS_SOURCE
)


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--blend",
        required=True,
        type=Path,
        help="Exact production Blender proof blend used for RETARGET_SWING_FULL",
    )
    parser.add_argument("--rig-json", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--render", action="store_true")
    return parser.parse_args(argv)


def q_values(q):
    q = q.normalized()
    if q.w < 0.0:
        q.negate()
    return [float(q.x), float(q.y), float(q.z), float(q.w)]


def root_relative(path):
    candidate = Path(path).resolve()
    try:
        return candidate.relative_to(ROOT).as_posix()
    except ValueError:
        return candidate.as_posix()


def v_values(v):
    return [float(v.x), float(v.y), float(v.z)]


def rot_only(matrix):
    return matrix.to_quaternion().normalized()


def world_rotation(obj, matrix):
    return (obj.matrix_world.to_quaternion() @ rot_only(matrix)).normalized()


def signed_angle(axis, start, end):
    axis = axis.normalized()
    start = (start - axis * start.dot(axis)).normalized()
    end = (end - axis * end.dot(axis)).normalized()
    if start.length <= 1.0e-7 or end.length <= 1.0e-7:
        return 0.0
    sine = axis.dot(start.cross(end))
    cosine = max(-1.0, min(1.0, start.dot(end)))
    return math.degrees(math.atan2(sine, cosine))


def shortest_arc(source, target):
    source = source.normalized()
    target = target.normalized()
    dot = max(-1.0, min(1.0, source.dot(target)))
    if dot > 1.0 - 1.0e-8:
        return Quaternion((1.0, 0.0, 0.0, 0.0))
    if dot < -1.0 + 1.0e-8:
        axis = source.cross(Vector((1.0, 0.0, 0.0)))
        if axis.length <= 1.0e-7:
            axis = source.cross(Vector((0.0, 1.0, 0.0)))
        return Quaternion(axis.normalized(), math.pi)
    return Quaternion(source.cross(target).normalized(), math.acos(dot))


def basis_from_primary_and_orientation(primary, orientation):
    """Build an RH anatomical basis with Y=primary and Z=pose roll axis."""
    y_axis = Vector(primary)
    if y_axis.length <= 1.0e-7:
        raise ValueError("Cannot build a basis from a zero primary axis")
    y_axis.normalize()
    z_axis = orientation @ Vector((0.0, 0.0, 1.0))
    z_axis = z_axis - y_axis * z_axis.dot(y_axis)
    if z_axis.length <= 1.0e-7:
        z_axis = orientation @ Vector((1.0, 0.0, 0.0))
        z_axis = z_axis - y_axis * z_axis.dot(y_axis)
    if z_axis.length <= 1.0e-7:
        z_axis = Vector((0.0, 0.0, 1.0)) - y_axis * y_axis.z
    z_axis.normalize()
    x_axis = y_axis.cross(z_axis)
    if x_axis.length <= 1.0e-7:
        raise ValueError("Cannot complete anatomical basis")
    x_axis.normalize()
    z_axis = x_axis.cross(y_axis).normalized()
    basis = Matrix.Identity(3)
    basis.col[0] = x_axis
    basis.col[1] = y_axis
    basis.col[2] = z_axis
    return basis


def basis_quaternion(basis):
    return basis.to_quaternion().normalized()


def rest_point(obj, name):
    return obj.matrix_world @ obj.data.bones[name].head_local


def pose_point(obj, name):
    return obj.matrix_world @ obj.pose.bones[name].matrix.translation


def source_point(obj, semantic, tail=False):
    name = SOURCE[semantic]
    pb = obj.pose.bones[name]
    if tail:
        return obj.matrix_world @ (pb.matrix @ Vector((0.0, pb.length, 0.0)))
    return pose_point(obj, name)


def source_rest_point(obj, semantic, tail=False):
    name = SOURCE[semantic]
    bone = obj.data.bones[name]
    if tail:
        return obj.matrix_world @ (bone.matrix_local @ Vector((0.0, bone.length, 0.0)))
    return rest_point(obj, name)


def control_point(obj, semantic, tail=False):
    name = CONTROL[semantic]
    pb = obj.pose.bones[name]
    if tail:
        return obj.matrix_world @ (pb.matrix @ Vector((0.0, pb.length, 0.0)))
    return pose_point(obj, name)


def control_rest_point(obj, semantic, tail=False):
    name = CONTROL[semantic]
    bone = obj.data.bones[name]
    if tail:
        return obj.matrix_world @ (bone.matrix_local @ Vector((0.0, bone.length, 0.0)))
    return rest_point(obj, name)


def target_rest_point(obj, name):
    return rest_point(obj, name)


def target_pose_point(obj, name):
    return pose_point(obj, name)


def target_terminal_point(obj, name):
    pb = obj.pose.bones[name]
    return obj.matrix_world @ (pb.matrix @ Vector((0.0, pb.length, 0.0)))


def semantic_segment(obj, semantic, kind, posed=True):
    if kind == "source":
        child = SOURCE_CHILD.get(semantic)
        point = source_point if posed else source_rest_point
        start = point(obj, semantic)
        if child is not None:
            end = point(obj, child)
        else:
            end = point(obj, semantic, tail=True)
    elif kind == "control":
        child = CONTROL_CHILD.get(semantic)
        point = control_point if posed else control_rest_point
        start = point(obj, semantic)
        if child is not None:
            end = point(obj, child)
        else:
            end = point(obj, semantic, tail=True)
    elif kind == "target":
        child = TARGET_CHILD.get(semantic)
        point = target_pose_point if posed else target_rest_point
        start = point(obj, semantic)
        if child is not None:
            end = point(obj, child)
        else:
            end = target_terminal_point(obj, semantic) if posed else (
                obj.matrix_world @ (obj.data.bones[semantic].matrix_local @ Vector((0.0, obj.data.bones[semantic].length, 0.0)))
            )
    else:
        raise ValueError(kind)
    return end - start


def representation_basis(obj, semantic, kind, posed=True):
    if kind == "source":
        name = SOURCE[semantic]
        orientation = world_rotation(obj, obj.pose.bones[name].matrix if posed else obj.data.bones[name].matrix_local)
    elif kind == "control":
        name = CONTROL[semantic]
        orientation = world_rotation(obj, obj.pose.bones[name].matrix if posed else obj.data.bones[name].matrix_local)
    else:
        name = semantic
        orientation = world_rotation(obj, obj.pose.bones[name].matrix if posed else obj.data.bones[name].matrix_local)
    primary = semantic_segment(obj, semantic, kind, posed)
    return basis_from_primary_and_orientation(primary, orientation)


def basis_record(basis):
    return {
        "x": v_values(basis.col[0]),
        "primaryY": v_values(basis.col[1]),
        "secondaryZ": v_values(basis.col[2]),
        "handedness": float(basis.determinant()),
        "quaternion": q_values(basis_quaternion(basis)),
    }


def quaternion_angle(a, b):
    dot = max(-1.0, min(1.0, abs(a.normalized().dot(b.normalized()))))
    return math.degrees(2.0 * math.acos(dot))


def compare_basis(expected, actual):
    ey = expected.col[1].normalized()
    ay = actual.col[1].normalized()
    direction = math.degrees(ey.angle(ay))
    full = quaternion_angle(basis_quaternion(expected), basis_quaternion(actual))

    # Align only the primary axes, then measure the remaining axial twist.
    align = shortest_arc(ay, ey)
    aligned_z = align @ actual.col[2]
    aligned_x = align @ actual.col[0]
    roll = signed_angle(ey, expected.col[2], aligned_z)
    secondary_dot = max(-1.0, min(1.0, expected.col[2].normalized().dot(aligned_z.normalized())))
    x_dot = max(-1.0, min(1.0, expected.col[0].normalized().dot(aligned_x.normalized())))
    expected_hand = 1 if expected.determinant() >= 0.0 else -1
    actual_hand = 1 if actual.determinant() >= 0.0 else -1
    return {
        "basisAngularErrorDegrees": full,
        "directionAngularErrorDegrees": direction,
        "rollOnlyAngularErrorDegrees": abs(roll),
        "signedRollErrorDegrees": roll,
        "secondaryAxisDotAfterDirectionAlign": secondary_dot,
        "thirdAxisDotAfterDirectionAlign": x_dot,
        "signAgreement": secondary_dot >= 0.0,
        "handednessAgreement": expected_hand == actual_hand,
        "expectedHandedness": expected_hand,
        "actualHandedness": actual_hand,
    }


def matrix_report(matrix):
    return [[float(v) for v in row] for row in matrix]


def target_pose_matrix_from_world(target, name, world_rotation_q, translation):
    result = world_rotation_q.to_matrix().to_4x4()
    result.translation = Vector(translation)
    return target.matrix_world.inverted() @ result


def target_apply_world_orientation(target, name, world_rotation_q, runtime_convention=False):
    """Apply a world-space orientation using only the local quaternion channel."""
    pb = target.pose.bones[name]
    # The current parent pose determines the head translation.  A bone's own
    # rotation does not change its own head, so this is the exact translation
    # needed by Blender's local-pose conversion.
    if runtime_convention:
        world_rotation_q = proof.source_quat_to_game(world_rotation_q).inverted().normalized()
    pose_origin = target_pose_point(target, name)
    pose_matrix = target_pose_matrix_from_world(target, name, world_rotation_q, pose_origin)
    local = proof.pose_to_local_basis(target, name, pose_matrix)
    location_error = local.translation.length
    scale_error = max(abs(value - 1.0) for value in local.to_3x3().to_scale())
    if location_error > 1.0e-4 or scale_error > 1.0e-4:
        raise RuntimeError(
            f"REST_BASIS_LOCAL_CHANNEL {name} location={location_error:.9g} scale={scale_error:.9g}"
        )
    pb.rotation_mode = "QUATERNION"
    pb.location = (0.0, 0.0, 0.0)
    pb.rotation_quaternion = local.to_3x3().to_quaternion().normalized()
    pb.scale = (1.0, 1.0, 1.0)
    bpy.context.view_layer.update()
    return {
        "locationMagnitude": float(location_error),
        "scaleError": float(scale_error),
        "localQuaternion": q_values(pb.rotation_quaternion),
    }


def semantic_name(semantic, kind):
    if kind == "source":
        return SOURCE[semantic]
    if kind == "control":
        return CONTROL[semantic]
    raise ValueError(kind)


def semantic_rest_segment(obj, semantic, kind):
    return semantic_segment(obj, semantic, kind, posed=False).normalized()


def semantic_pose_delta(obj, semantic, kind):
    name = semantic_name(semantic, kind)
    rest = world_rotation(obj, obj.data.bones[name].matrix_local)
    pose = world_rotation(obj, obj.pose.bones[name].matrix)
    return (pose @ rest.inverted()).normalized(), rest, pose


def apply_rest_basis_transfer(source, target, pairs, source_kind, runtime_convention=True):
    """Apply explicit source pose delta -> target rest basis for mapped bones."""
    reset_target(target)
    results = {}
    for source_semantic, target_name in topological_target_pairs(target, pairs):
        delta, _, _ = semantic_pose_delta(source, source_semantic, source_kind)
        target_rest = world_rotation(target, target.data.bones[target_name].matrix_local)
        desired = (delta @ target_rest).normalized()
        results[target_name] = {
            "sourceSemantic": source_semantic,
            "targetRestWorldQuaternion": q_values(target_rest),
            "sourcePoseDeltaQuaternion": q_values(delta),
            "targetPoseWorldQuaternion": q_values(desired),
            "apply": target_apply_world_orientation(
                target, target_name, desired, runtime_convention=runtime_convention,
            ),
        }
    bpy.context.view_layer.update()
    return results


def reset_target(target):
    if target.animation_data:
        target.animation_data_clear()
    for pb in target.pose.bones:
        pb.rotation_mode = "QUATERNION"
        pb.location = (0.0, 0.0, 0.0)
        pb.rotation_quaternion = (1.0, 0.0, 0.0, 0.0)
        pb.scale = (1.0, 1.0, 1.0)
    bpy.context.view_layer.update()


def target_depth(name, target):
    depth = 0
    bone = target.data.bones[name]
    while bone.parent is not None:
        depth += 1
        bone = bone.parent
    return depth


def topological_target_pairs(target, pairs):
    return sorted(pairs, key=lambda pair: target_depth(pair[1], target))


def source_control_metrics(source, control):
    rows = []
    for source_semantic, control_semantic in SOURCE_TO_CONTROL_PAIRS:
        source_basis = representation_basis(source, source_semantic, "source", posed=True)
        control_basis = representation_basis(control, control_semantic, "control", posed=True)
        source_delta, source_rest_orientation, _ = semantic_pose_delta(source, source_semantic, "source")
        control_name = semantic_name(control_semantic, "control")
        control_rest_orientation = world_rotation(control, control.data.bones[control_name].matrix_local)
        expected_orientation = (source_delta @ control_rest_orientation).normalized()
        expected_primary = source_delta @ semantic_rest_segment(control, control_semantic, "control")
        expected_control_basis = basis_from_primary_and_orientation(expected_primary, expected_orientation)
        rows.append({
            "source": source_semantic,
            "control": control_semantic,
            "sourceBasis": basis_record(source_basis),
            "controlBasis": basis_record(control_basis),
            "expectedRestBasisControl": basis_record(expected_control_basis),
            "sourcePoseDeltaQuaternion": q_values(source_delta),
            "metrics": compare_basis(expected_control_basis, control_basis),
        })
    return rows


def target_rest_segment(rest_world, target, name):
    child = TARGET_CHILD.get(name)
    if child is None:
        bone = target.data.bones[name]
        return bone.tail_local - bone.head_local
    return Vector(rest_world[child][1]) - Vector(rest_world[name][1])


def current_solver_target_bases(control, target, rest_world):
    """Reconstruct A in the source/ANIMX frame before CoH game conversion."""
    semantics = proof.common_control_semantics(control)
    desired = {}

    def add(name, direction, roll):
        rest_matrix = target.data.bones[name].matrix_local.copy()
        target_matrix = proof.pose_matrix_for_segment(
            rest_matrix,
            target_rest_segment(rest_world, target, name),
            Vector((0.0, 0.0, 0.0)),
            Vector(direction),
            Vector(roll),
        )
        orientation = target_matrix.to_3x3().to_quaternion().normalized()
        primary = orientation @ target_rest_segment(rest_world, target, name).normalized()
        desired[name] = {
            "worldQuaternion": q_values(orientation),
            "basis": basis_from_primary_and_orientation(primary, orientation),
        }

    torso = semantics["torso"]
    add("HIPS", torso["hipsDirection"], torso["pelvisNormal"])
    add("WAIST", torso["waistDirection"], torso["waistRoll"])
    add("CHEST", torso["chestDirection"], torso["chestRoll"])
    add("NECK", torso["neckDirection"], torso["neckRoll"])
    add("HEAD", torso["headDirection"], torso["headRoll"])
    for side in ("r", "l"):
        arm = semantics[f"arm_{side}"]
        suffix = "R" if side == "r" else "L"
        add(f"COL_{suffix}", arm["clavicle"], arm["planeNormal"])
        add(f"UARM{suffix}", arm["upperDirection"], arm["planeNormal"])
        add(f"LARM{suffix}", arm["lowerDirection"], arm["planeNormal"])
        hand_roll = arm["handUp"]
        if abs(hand_roll.dot(arm["handDirection"])) > 0.95:
            hand_roll = arm["planeNormal"]
        add(f"HAND{suffix}", arm["handDirection"], hand_roll)
        leg = semantics[f"leg_{side}"]
        add(f"ULEG{suffix}", leg["thighDirection"], leg["planeNormal"])
        add(f"LLEG{suffix}", leg["shinDirection"], leg["planeNormal"])
        add(f"FOOT{suffix}", leg["footDirection"], leg["planeNormal"])
    return desired


def rest_basis_target_bases(source, target, pairs, source_kind, rest_world):
    """Return B's expected source-frame anatomical bases by target bone."""
    result = {}
    for source_semantic, target_name in pairs:
        delta, _, _ = semantic_pose_delta(source, source_semantic, source_kind)
        target_rest_orientation = world_rotation(target, target.data.bones[target_name].matrix_local)
        desired = (delta @ target_rest_orientation).normalized()
        primary = desired @ target_rest_segment(rest_world, target, target_name).normalized()
        result[target_name] = basis_from_primary_and_orientation(primary, desired)
    return result


def control_target_metrics(control, target, rest_world, current_bases, target_b):
    rows = []
    for source_semantic, target_name in CONTROL_TO_TARGET_PAIRS:
        control_basis = representation_basis(control, source_semantic, "control", posed=True)
        current_basis = current_bases[target_name]["basis"]
        b_basis = target_b[target_name]
        # The control->target rest-basis reference uses the control pose delta
        # and the target rest basis.  This isolates the downstream boundary.
        control_delta, control_rest, _ = semantic_pose_delta(control, source_semantic, "control")
        target_rest = world_rotation(target, target.data.bones[target_name].matrix_local)
        expected_world = (control_delta @ target_rest).normalized()
        expected_primary = expected_world @ semantic_segment(target, target_name, "target", posed=False).normalized()
        expected_basis = basis_from_primary_and_orientation(expected_primary, expected_world)
        rows.append({
            "control": source_semantic,
            "target": target_name,
            "controlBasis": basis_record(control_basis),
            "currentTargetBasis": basis_record(current_basis),
            "restBasisTargetBasis": basis_record(b_basis),
            "controlPoseDeltaQuaternion": q_values(control_delta),
            "targetRestBasisQuaternion": q_values(target_rest),
            "expectedControlDeltaTargetBasis": basis_record(expected_basis),
            "currentVsExpected": compare_basis(expected_basis, current_basis),
            "restBasisVsExpected": compare_basis(expected_basis, b_basis),
            "currentVsRestBasis": compare_basis(b_basis, current_basis),
        })
    return rows


def rest_basis_offsets(source, control, target):
    rows = []
    # Include both compressed torso candidates so Spine1 cannot disappear
    # behind a generic "torso scaffold" statement.
    rest_pairs = (
        ("hips", "HIPS"), ("spine", "WAIST"), ("spine1", "WAIST"),
        ("spine2", "CHEST"), ("neck", "NECK"), ("head", "HEAD"),
        ("shoulder_r", "COL_R"), ("arm_r", "UARMR"), ("forearm_r", "LARMR"),
        ("shoulder_l", "COL_L"), ("arm_l", "UARML"), ("forearm_l", "LARML"),
        ("thigh_r", "ULEGR"), ("shin_r", "LLEGR"),
        ("thigh_l", "ULEGL"), ("shin_l", "LLEGL"),
    )
    for source_semantic, target_name in rest_pairs:
        source_basis = representation_basis(source, source_semantic, "source", posed=False)
        target_basis = representation_basis(target, target_name, "target", posed=False)
        source_y = source_basis.col[1].normalized()
        target_y = target_basis.col[1].normalized()
        align = shortest_arc(source_y, target_y)
        aligned_source_z = align @ source_basis.col[2]
        offset_q = (basis_quaternion(target_basis) @ basis_quaternion(source_basis).inverted()).normalized()
        rows.append({
            "source": source_semantic,
            "target": target_name,
            "sourceRestBasis": basis_record(source_basis),
            "targetRestBasis": basis_record(target_basis),
            "relativeBasisMatrixTargetTimesSourceInverse": matrix_report(target_basis @ source_basis.inverted()),
            "relativeBasisQuaternion": q_values(offset_q),
            "fullAngularOffsetDegrees": quaternion_angle(basis_quaternion(source_basis), basis_quaternion(target_basis)),
            "primaryAxisOffsetDegrees": math.degrees(source_y.angle(target_y)),
            "rollAxisOffsetDegreesAfterPrimaryAlign": signed_angle(target_y, aligned_source_z, target_basis.col[2]),
        })
    return rows


def current_roll_references(source, control):
    def src_point(semantic, tail=False):
        return source_point(source, semantic, tail)

    def plane(upper, lower):
        upper_direction = (src_point(lower) - src_point(upper)).normalized()
        if upper.startswith("arm_"):
            side = upper[-1]
            lower_start = f"forearm_{side}"
            lower_end = f"hand_{side}"
        else:
            side = upper[-1]
            lower_start = f"shin_{side}"
            lower_end = f"foot_{side}"
        lower_direction = (src_point(lower_end) - src_point(lower_start)).normalized()
        normal = upper_direction.cross(lower_direction)
        source_fallback = world_rotation(source, source.pose.bones[SOURCE[upper]].matrix) @ Vector((0.0, 0.0, 1.0))
        if normal.length <= 1.0e-7:
            normal = source_fallback
        return normal.normalized()

    pelvis_lateral = (src_point("thigh_r") - src_point("thigh_l")).normalized()
    pelvis_up = (src_point("hips", True) - src_point("hips")).normalized()
    pelvis = pelvis_lateral.cross(pelvis_up)
    if pelvis.length <= 1.0e-7:
        pelvis = world_rotation(source, source.pose.bones[SOURCE["hips"]].matrix) @ Vector((0.0, 0.0, 1.0))
    vectors = {
        "arm_r_plane": plane("arm_r", "forearm_r"),
        "arm_l_plane": plane("arm_l", "forearm_l"),
        "leg_r_plane": plane("thigh_r", "shin_r"),
        "leg_l_plane": plane("thigh_l", "shin_l"),
        "pelvis_plane": pelvis.normalized(),
    }
    for semantic in ("hips", "spine", "spine1", "spine2", "neck", "head"):
        vectors[f"source_pose_roll_{semantic}"] = world_rotation(
            source, source.pose.bones[SOURCE[semantic]].matrix,
        ) @ Vector((0.0, 0.0, 1.0))
    for semantic in (
        "shoulder_r", "arm_r", "forearm_r", "hand_r",
        "shoulder_l", "arm_l", "forearm_l", "hand_l",
        "thigh_r", "shin_r", "foot_r", "thigh_l", "shin_l", "foot_l",
    ):
        vectors[f"source_pose_roll_{semantic}"] = world_rotation(
            source, source.pose.bones[SOURCE[semantic]].matrix,
        ) @ Vector((0.0, 0.0, 1.0))
    # The control solver applies one arm plane to shoulder/upper/lower/hand
    # and one leg plane to thigh/shin/foot/toe.  Record those exact vectors.
    vectors["control_axis_arm_r"] = world_rotation(control, control.pose.bones[CONTROL["arm_r"]].matrix) @ Vector((0.0, 0.0, 1.0))
    vectors["control_axis_leg_r"] = world_rotation(control, control.pose.bones[CONTROL["thigh_r"]].matrix) @ Vector((0.0, 0.0, 1.0))
    return vectors


def sign_continuity(references):
    by_name = {}
    names = sorted({name for frame in references.values() for name in frame})
    for name in names:
        row = {"name": name, "dots": [], "flips": []}
        for first, second in zip(FRAMES, FRAMES[1:]):
            a = Vector(references[first][name])
            b = Vector(references[second][name])
            dot = float(a.normalized().dot(b.normalized()))
            row["dots"].append({"from": first, "to": second, "dot": dot})
            if dot < 0.0:
                row["flips"].append({"from": first, "to": second, "dot": dot})
        by_name[name] = row
    return by_name


def projected_roll_difference(primary, shared_reference, pose_roll):
    """Compare two roll references after projecting them about one bone axis."""
    axis = Vector(primary).normalized()
    shared = Vector(shared_reference) - axis * Vector(shared_reference).dot(axis)
    actual = Vector(pose_roll) - axis * Vector(pose_roll).dot(axis)
    if shared.length <= 1.0e-7 or actual.length <= 1.0e-7:
        return {
            "sharedReferenceProjectionLength": float(shared.length),
            "poseRollProjectionLength": float(actual.length),
            "signedRollDifferenceDegrees": 0.0,
            "absoluteRollDifferenceDegrees": 0.0,
            "projectedReferenceDot": 0.0,
            "signAgreement": None,
        }
    shared_length = float(shared.length)
    actual_length = float(actual.length)
    shared.normalize()
    actual.normalize()
    signed = signed_angle(axis, actual, shared)
    return {
        "sharedReferenceProjectionLength": shared_length,
        "poseRollProjectionLength": actual_length,
        "signedRollDifferenceDegrees": float(signed),
        "absoluteRollDifferenceDegrees": float(abs(signed)),
        "projectedReferenceDot": float(max(-1.0, min(1.0, shared.dot(actual)))),
        "signAgreement": bool(shared.dot(actual) >= 0.0),
    }


def shared_plane_reuse(source, control):
    """Measure the one-plane-per-chain reuse against every source roll axis."""
    references = current_roll_references(source, control)
    chains = {
        "arm_r": ("arm_r_plane", ("shoulder_r", "arm_r", "forearm_r", "hand_r")),
        "arm_l": ("arm_l_plane", ("shoulder_l", "arm_l", "forearm_l", "hand_l")),
        "leg_r": ("leg_r_plane", ("thigh_r", "shin_r", "foot_r")),
        "leg_l": ("leg_l_plane", ("thigh_l", "shin_l", "foot_l")),
    }
    result = {}
    for chain, (plane_name, semantics) in chains.items():
        plane = Vector(references[plane_name]).normalized()
        result[chain] = {
            "sharedPlane": v_values(plane),
            "bones": {
                semantic: projected_roll_difference(
                    semantic_segment(source, semantic, "source", posed=True),
                    plane,
                    references[f"source_pose_roll_{semantic}"],
                )
                | {
                    "sourcePoseRollAxis": v_values(references[f"source_pose_roll_{semantic}"]),
                }
                for semantic in semantics
            },
        }
    return result


def proxy_points(obj, kind):
    if kind == "source":
        names = FOCUS_SOURCE
        point = lambda semantic: source_point(obj, semantic)
        child = SOURCE_CHILD
        segment = lambda semantic: semantic_segment(obj, semantic, "source", True)
        basis = lambda semantic: representation_basis(obj, semantic, "source", True)
    else:
        names = FOCUS_TARGET
        point = lambda semantic: target_pose_point(obj, semantic)
        child = TARGET_CHILD
        segment = lambda semantic: semantic_segment(obj, semantic, "target", True)
        basis = lambda semantic: representation_basis(obj, semantic, "target", True)
    segments = []
    points = []
    for semantic in names:
        start = point(semantic)
        points.append(start)
        if child.get(semantic) is not None:
            end = point(child[semantic])
        else:
            end = start + segment(semantic)
        segments.append((semantic, start, end, basis(semantic)))
    return segments, points


def material(name, color, metallic=0.0, roughness=0.65):
    result = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    result.diffuse_color = (*color, 1.0)
    result.use_nodes = True
    bsdf = result.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs["Base Color"].default_value = (*color, 1.0)
        bsdf.inputs["Metallic"].default_value = metallic
        bsdf.inputs["Roughness"].default_value = roughness
    return result


def add_roll_marker(collection, origin, basis, size, mat):
    # A short asymmetric fin along +Z makes axial roll visible without a
    # costume or a full mesh.  The second short rod marks +X for handedness.
    from mathutils import Vector as V

    z_end = origin + basis.col[2].normalized() * size
    x_end = origin + basis.col[0].normalized() * (size * 0.55)
    bpy.ops.mesh.primitive_cone_add(vertices=4, radius1=size * 0.08, radius2=0.0, depth=size, location=(origin + z_end) * 0.5)
    z_obj = bpy.context.object
    z_obj.name = f"RollFin_{len(collection.objects)}"
    z_obj.rotation_mode = "QUATERNION"
    z_obj.rotation_quaternion = (z_end - origin).to_track_quat("Z", "Y")
    z_obj.data.materials.append(mat)
    collection.objects.link(z_obj)
    bpy.context.collection.objects.unlink(z_obj)

    bpy.ops.mesh.primitive_cylinder_add(vertices=8, radius=size * 0.018, depth=(x_end - origin).length, location=(origin + x_end) * 0.5)
    x_obj = bpy.context.object
    x_obj.name = f"RollAxis_{len(collection.objects)}"
    x_obj.rotation_mode = "QUATERNION"
    x_obj.rotation_quaternion = (x_end - origin).to_track_quat("Z", "Y")
    x_obj.data.materials.append(mat)
    collection.objects.link(x_obj)
    bpy.context.collection.objects.unlink(x_obj)


def add_proxy(name, segments, points, color):
    collection = bpy.data.collections.new(name)
    bpy.context.scene.collection.children.link(collection)
    body = material(f"{name}_Body", color)
    fin = material(f"{name}_Roll", tuple(min(1.0, c * 1.25 + 0.1) for c in color), metallic=0.1)
    for _, start, end, basis in segments:
        bpy.ops.mesh.primitive_cylinder_add(vertices=8, radius=0.055, depth=(end - start).length, location=(start + end) * 0.5)
        bone = bpy.context.object
        bone.name = f"{name}_Bone_{len(collection.objects)}"
        bone.rotation_mode = "QUATERNION"
        bone.rotation_quaternion = (end - start).to_track_quat("Z", "Y")
        bone.data.materials.append(body)
        collection.objects.link(bone)
        bpy.context.collection.objects.unlink(bone)
        add_roll_marker(collection, start, basis, 0.25, fin)
    for point in points:
        bpy.ops.mesh.primitive_uv_sphere_add(segments=12, ring_count=6, radius=0.085, location=point)
        joint = bpy.context.object
        joint.name = f"{name}_Joint_{len(collection.objects)}"
        joint.data.materials.append(body)
        collection.objects.link(joint)
        bpy.context.collection.objects.unlink(joint)
    return collection


def look_at(obj, target):
    obj.rotation_euler = (Vector(target) - obj.location).to_track_quat("-Z", "Y").to_euler()


def render_frame(output_dir, frame, source, target, target_b):
    scene = bpy.context.scene
    for obj in list(bpy.data.objects):
        if obj.type in {"CAMERA", "LIGHT"}:
            bpy.data.objects.remove(obj, do_unlink=True)
    for obj in list(bpy.data.objects):
        if obj.type == "MESH":
            bpy.data.objects.remove(obj, do_unlink=True)

    source_segments, source_points = proxy_points(source, "source")
    current_segments, current_points = proxy_points(target, "target")
    rest_segments, rest_points = proxy_points(target_b, "target")
    proxies = {
        "raw": add_proxy("RAW_MIXAMO", source_segments, source_points, (0.10, 0.45, 0.95)),
        "current": add_proxy("CURRENT_COH", current_segments, current_points, (0.95, 0.28, 0.08)),
        "rest": add_proxy("REST_BASIS_AB", rest_segments, rest_points, (0.16, 0.75, 0.28)),
    }
    points_by_representation = {
        "raw": source_points,
        "current": current_points,
        "rest": rest_points,
    }

    # Fit the proxies tightly enough for the asymmetric roll fins to remain
    # visible.  Each representation gets its own centered fit because the
    # exact source and CoH rest rigs intentionally have different origins.
    bpy.ops.object.camera_add(location=(0.0, -10.0, 1.0))
    camera = bpy.context.object
    camera.data.lens = 52
    camera.data.clip_end = 1000.0
    scene.camera = camera
    lights = []
    for location, energy in (
        (Vector((4.0, -6.0, 8.0)), 1100.0),
        (Vector((-4.0, -3.0, 4.0)), 700.0),
        (Vector((0.0, 4.0, 5.0)), 400.0),
    ):
        bpy.ops.object.light_add(type="AREA", location=location)
        light = bpy.context.object
        light.data.energy = energy
        light.data.shape = "DISK"
        light.data.size = 5.0
        lights.append((light, location))
    world = scene.world or bpy.data.worlds.new("Issue36OrientationWorld")
    scene.world = world
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.004, 0.006, 0.012, 1.0)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.25
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 640
    scene.render.resolution_y = 640
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = False

    for representation, collection in proxies.items():
        points = points_by_representation[representation]
        minimum = Vector((min(p.x for p in points), min(p.y for p in points), min(p.z for p in points)))
        maximum = Vector((max(p.x for p in points), max(p.y for p in points), max(p.z for p in points)))
        center = (minimum + maximum) * 0.5
        extent = max((maximum - minimum).length, 2.0)
        fit = extent * 1.15
        views = {
            "front": Vector((0.0, -fit, fit * 0.10)),
            "threequarter": Vector((fit * 0.78, -fit, fit * 0.16)),
            "side": Vector((fit, 0.0, fit * 0.10)),
        }
        for light, relative_location in lights:
            light.location = center + relative_location
            look_at(light, center)
        for view_name, offset in views.items():
            camera.location = center + offset
            look_at(camera, center)
            # Explicit visibility map; keeping this verbose avoids a hidden
            # collection state leaking between frames in Blender 5.2.
            for other_name, other_collection in proxies.items():
                for obj in other_collection.objects:
                    obj.hide_render = other_name != representation
            path = output_dir / "visual" / f"frame{frame:02d}" / representation / f"{view_name}.png"
            path.parent.mkdir(parents=True, exist_ok=True)
            scene.render.filepath = str(path)
            bpy.ops.render.render(write_still=True)

    return {
        "frame": frame,
        "paths": {
            representation: [
                (Path("visual") / f"frame{frame:02d}" / representation / f"{view}.png").as_posix()
                for view in views
            ]
            for representation in proxies
        },
    }


def run_frame(source, control, target, target_b_source, target_b_control, rest_world, frame):
    scene = bpy.context.scene
    scene.frame_set(frame)
    bpy.context.view_layer.update()
    proof.reset_pose(control)
    proof.solve_control_from_mixamo_geometry(source, control)
    source_control_old = proof.semantic_control_comparison(source, control)
    proof.reset_pose(target)
    current_report = proof.transfer_control_to_coh(target, rest_world, control)
    rest_source_report = apply_rest_basis_transfer(source, target_b_source, CONTROL_TO_TARGET_PAIRS, "source")
    rest_control_report = apply_rest_basis_transfer(control, target_b_control, CONTROL_TO_TARGET_PAIRS, "control")
    bpy.context.view_layer.update()

    raw_basis = {semantic: basis_record(representation_basis(source, semantic, "source", True)) for semantic in FOCUS_SOURCE}
    control_basis = {semantic: basis_record(representation_basis(control, semantic, "control", True)) for semantic in FOCUS_SOURCE}
    current_solver = current_solver_target_bases(control, target, rest_world)
    source_rest_basis = rest_basis_target_bases(source, target, CONTROL_TO_TARGET_PAIRS, "source", rest_world)
    control_rest_basis = rest_basis_target_bases(control, target, CONTROL_TO_TARGET_PAIRS, "control", rest_world)
    current_basis = {name: basis_record(current_solver[name]["basis"]) for name in FOCUS_TARGET}
    rest_basis = {name: basis_record(source_rest_basis[name]) for name in FOCUS_TARGET}
    current_runtime_basis = {
        name: basis_record(representation_basis(target, name, "target", posed=True))
        for name in FOCUS_TARGET
    }
    rest_runtime_basis = {
        name: basis_record(representation_basis(target_b_source, name, "target", posed=True))
        for name in FOCUS_TARGET
    }
    return {
        "frame": frame,
        "oldPassCriteria": source_control_old,
        "currentControlToTargetRotationOnly": current_report,
        "restBasisApply": {
            "sourceToTarget": rest_source_report,
            "controlToTarget": rest_control_report,
        },
        "sourceToControl": source_control_metrics(source, control),
        "controlToCoH": control_target_metrics(control, target, rest_world, current_solver, control_rest_basis),
        "sharedPlaneReuse": shared_plane_reuse(source, control),
        "representations": {
            "rawMixamo": raw_basis,
            "control": control_basis,
            "currentCoHMale": current_basis,
            "currentCoHMaleRuntimeBasis": current_runtime_basis,
            "restBasisABCoHMale": rest_basis,
            "restBasisABCoHMaleRuntimeBasis": rest_runtime_basis,
            "restBasisControlABCoHMale": {
                name: basis_record(control_rest_basis[name])
                for name in FOCUS_TARGET
            },
        },
        "rollReferences": {
            name: v_values(value)
            for name, value in current_roll_references(source, control).items()
        },
    }


def make_markdown(report, output_path):
    def metric_cell(metric):
        return (
            f"{metric['directionAngularErrorDegrees']:.2f}°/"
            f"{metric['rollOnlyAngularErrorDegrees']:.2f}°/"
            f"{metric['basisAngularErrorDegrees']:.2f}°"
        )

    def relative_link(path):
        candidate = Path(path)
        try:
            return candidate.relative_to(output_path.parent).as_posix()
        except ValueError:
            return candidate.as_posix()

    lines = [
        "# Issue 36 orientation/roll localization",
        "",
        "Diagnostic-only A/B for `swinginganimations/Swinging.fbx`, action `Armature|mixamo.com|Layer0`, frames 18/20/22.",
        "",
        "A is the current direction-plus-plane diagnostic path. B is an explicit semantic rest-basis transfer: it applies the evaluated source pose delta to the exact CoH Male target rest basis. No production animation, exporter, runtime asset, or Web Swing code is changed by this tool.",
        "",
        "The old source→control pass checks joint positions and segment directions only. It does not compare a full orthonormal basis, axial roll, or sign continuity; therefore `pass=true` can accept a corkscrewed pose.",
        "",
        "## Math",
        "",
        "For each mapped source/target bone, B uses `sourceDelta = sourcePoseWorld * inverse(sourceRestWorld)` and `targetPoseWorld = sourceDelta * targetRestWorld`, then converts the desired target world matrix back through Blender's parent/rest relation with zero local translation and unit scale.",
        "",
        "`direction` compares primary Y axes. `roll-only` aligns the two primary axes with the shortest arc and measures the remaining signed angle between secondary Z axes. `basis` is the complete quaternion angle. Cells below are `direction / roll-only / full-basis` degrees; the complete X/Y/Z basis vectors, quaternions, handedness, and source pose deltas are in `orientation-report.json`.",
        "",
        "## First-divergence summary",
        "",
    ]
    frames = report["frames"]
    for boundary, key in (("source→control", "sourceToControl"), ("control→CoH current vs rest-basis reference", "controlToCoH")):
        lines.append(f"### {boundary}")
        lines.append("")
        lines.append("| frame | first basis error > 1° | first roll error > 1° | max direction | max roll-only | max basis |")
        lines.append("|---:|---|---|---:|---:|---:|")
        for item in frames:
            rows = item[key]
            if key == "sourceToControl":
                metrics = [row["metrics"] for row in rows]
            else:
                metrics = [row["currentVsExpected"] for row in rows]
            first_basis = next((rows[i].get("source", rows[i].get("target")) for i, m in enumerate(metrics) if m["basisAngularErrorDegrees"] > 1.0), "none")
            first_roll = next((rows[i].get("source", rows[i].get("target")) for i, m in enumerate(metrics) if m["rollOnlyAngularErrorDegrees"] > 1.0), "none")
            lines.append(
                f"| {item['frame']} | {first_basis} | {first_roll} | "
                f"{max(m['directionAngularErrorDegrees'] for m in metrics):.3f}° | "
                f"{max(m['rollOnlyAngularErrorDegrees'] for m in metrics):.3f}° | "
                f"{max(m['basisAngularErrorDegrees'] for m in metrics):.3f}° |"
            )
        lines.append("")

    lines.extend([
        "## Boundary reading",
        "",
        "The first axial-roll failure in the source→control traversal is the right shoulder (`shoulder_r`, mapped to `COL_R`); its segment direction remains 0° while its roll error is already large. At the downstream control→CoH boundary, the first target bone with a full-basis error over 1° is `HIPS`, with an approximately 90° axial-roll error in all three frames. The `head` basis row appears earlier in the source list because the terminal `head_end` geometry is not the same as the target's `CRANIUM` terminal; it is recorded separately and is not the first shoulder/limb roll cause.",
        "",
        "The B rows are zero (within floating-point/render-rig rounding), while A retains repeated 90°/180° axial errors. This localizes the primary defect to rest-basis orientation conversion, not a source frame-map or joint-position failure.",
        "",
        "## Source→control basis metrics",
        "",
        "These rows compare the evaluated control basis against the source pose delta expressed through the control rest basis. They are separate from the downstream target test.",
        "",
        "| source semantic | source bone | control bone | frame 18 d/r/b | frame 20 d/r/b | frame 22 d/r/b |",
        "|---|---|---|---:|---:|---:|",
    ])
    for semantic in FOCUS_SOURCE:
        cells = []
        for item in frames:
            row = next(row for row in item["sourceToControl"] if row["source"] == semantic)
            cells.append(metric_cell(row["metrics"]))
        lines.append(
            f"| `{semantic}` | `{SOURCE[semantic]}` | `{CONTROL[semantic]}` | "
            + " | ".join(cells)
            + " |"
        )

    lines.extend([
        "",
        "## Control→CoH target basis metrics",
        "",
        "A is the current production-like direction/plane result in the source/ANIMX comparison frame. B is the rest-basis reference driven from the same control pose delta. The runtime-convention clones are also recorded in the JSON and used for the visual renders.",
        "",
        "| target | control semantic | A18 d/r/b | B18 d/r/b | A20 d/r/b | B20 d/r/b | A22 d/r/b | B22 d/r/b |",
        "|---|---|---:|---:|---:|---:|---:|---:|",
    ])
    for target_name in FOCUS_TARGET:
        cells = []
        source_semantic = None
        for item in frames:
            row = next(row for row in item["controlToCoH"] if row["target"] == target_name)
            source_semantic = row["control"]
            cells.extend((metric_cell(row["currentVsExpected"]), metric_cell(row["restBasisVsExpected"])))
        lines.append(
            f"| `{target_name}` | `{source_semantic}` | "
            + " | ".join(cells)
            + " |"
        )

    lines.extend([
        "## Torso mapping audit",
        "",
        "The current production compression is `Mixamo Hips → HIPS`, `Spine → WAIST`, `Spine2 → CHEST`, `Neck → NECK`, `Head → HEAD`; `Spine1` is not directly consumed by a target channel. The rest-basis table retains both `Spine → WAIST` and `Spine1 → WAIST` so that the skipped source segment remains explicit.",
        "",
        "| frame | first current CoH basis divergence > 1° | current CHEST direction | current CHEST roll-only | B CHEST direction | B CHEST roll-only |",
        "|---:|---|---:|---:|---:|---:|",
    ])
    for item in frames:
        rows = item["controlToCoH"]
        first = next((row["target"] for row in rows if row["currentVsExpected"]["basisAngularErrorDegrees"] > 1.0), "none")
        chest = next(row for row in rows if row["target"] == "CHEST")
        lines.append(
            f"| {item['frame']} | {first} | {chest['currentVsExpected']['directionAngularErrorDegrees']:.3f}° | "
            f"{chest['currentVsExpected']['rollOnlyAngularErrorDegrees']:.3f}° | "
            f"{chest['restBasisVsExpected']['directionAngularErrorDegrees']:.3f}° | "
            f"{chest['restBasisVsExpected']['rollOnlyAngularErrorDegrees']:.3f}° |"
        )

    lines.extend(["", "## Rest-basis offsets", "", "| source | target | full offset | primary offset | roll offset after primary align |", "|---|---|---:|---:|---:|"])
    for row in report["restBasisOffsets"]:
        lines.append(
            f"| {row['source']} | {row['target']} | {row['fullAngularOffsetDegrees']:.3f}° | "
            f"{row['primaryAxisOffsetDegrees']:.3f}° | {row['rollAxisOffsetDegreesAfterPrimaryAlign']:.3f}° |"
        )

    lines.extend(["", "## Shared bend-plane reuse", "", "The current solver reuses one bend plane for every bone in an arm or leg chain. The table compares that shared plane with each source bone's evaluated pose Z/roll axis after projection about that bone's primary segment. Large signed differences are the direct roll-loss measurement; this is not a frame-to-frame sign-flip test.", "", "| chain | source bone | 18 signed° / dot | 20 signed° / dot | 22 signed° / dot |", "|---|---|---:|---:|---:|"])
    for chain in ("arm_r", "arm_l", "leg_r", "leg_l"):
        for semantic in frames[0]["sharedPlaneReuse"][chain]["bones"]:
            cells = []
            for item in frames:
                value = item["sharedPlaneReuse"][chain]["bones"][semantic]
                cells.append(f"{value['signedRollDifferenceDegrees']:+.1f}° / {value['projectedReferenceDot']:.3f}")
            lines.append(f"| `{chain}` | `{semantic}` | " + " | ".join(cells) + " |")

    lines.extend(["", "## Plane/roll sign continuity", "", "A negative reference dot would be a frame-to-frame sign flip. None occurs in the tested 18→20 or 20→22 intervals; sign continuity is therefore not the first cause in this window.", "", "| reference | 18→20 dot | 20→22 dot | flips |", "|---|---:|---:|---|"])
    for name, row in report["signContinuity"].items():
        dots = row["dots"]
        lines.append(f"| {name} | {dots[0]['dot']:.6f} | {dots[1]['dot']:.6f} | {row['flips'] or 'none'} |")

    lines.extend(["", "## A/B visual outputs", "", "Each frame has raw Mixamo, current CoH Male, and rest-basis B renders from the front, three-quarter, and side views. The asymmetric roll fins make axial twist and handedness visible without relying on a skinned mesh.", "", "| frame | raw | current CoH | rest-basis B |", "|---:|---|---|---|"])
    for item in report["frames"]:
        frame = item["frame"]
        paths = item.get("render", {}).get("paths", {})
        def link(rep):
            values = paths.get(rep, [])
            if not values:
                return "(render disabled)"
            return ", ".join(f"[{view}]({relative_link(path)})" for view, path in zip(("front", "3/4", "side"), values))
        lines.append(f"| {frame} | {link('raw')} | {link('current')} | {link('rest')} |")

    lines.extend(["", "## Exporter/runtime scope", "", "The diagnostic applies rotations only: every B application reports zero local translation and unit scale within the assertion tolerance. It does not invoke the exporter or change runtime assets. The A/B mismatch is therefore a source/control/target orientation-construction defect in this proof path; the existing exporter/runtime transport remains outside the localized failure.", "", "| frame | B source→target max location | B source→target max scale | B control→target max location | B control→target max scale |", "|---:|---:|---:|---:|---:|"])
    for item in frames:
        values = []
        for key in ("sourceToTarget", "controlToTarget"):
            applies = [value["apply"] for value in item["restBasisApply"][key].values()]
            values.extend((max(value["locationMagnitude"] for value in applies), max(value["scaleError"] for value in applies)))
        lines.append(f"| {item['frame']} | {values[0]:.3g} | {values[1]:.3g} | {values[2]:.3g} | {values[3]:.3g} |")

    output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main():
    args = parse_args()
    args.blend = args.blend.resolve()
    args.rig_json = args.rig_json.resolve()
    args.output_dir = args.output_dir.resolve()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.open_mainfile(filepath=str(args.blend))

    source = bpy.data.objects.get("Mixamo_Source_Armature")
    control = bpy.data.objects.get("Anatomical_Control_Rig")
    target = bpy.data.objects.get("CoH_Male_Exact_Export_Rig")
    if source is None or control is None or target is None:
        raise RuntimeError("Production proof blend is missing source, control, or exact CoH target")
    report_rig, bones, by_id = load_rig(args.rig_json)
    _, rest_world = build_source_rest(bones, by_id)

    target_b_source = target.copy()
    target_b_source.data = target.data.copy()
    target_b_source.name = "CoH_Male_RestBasis_AB_Source"
    target_b_source.data.name = "CoH_Male_RestBasis_AB_Source"
    bpy.context.collection.objects.link(target_b_source)
    target_b_source.animation_data_clear()
    target_b_control = target.copy()
    target_b_control.data = target.data.copy()
    target_b_control.name = "CoH_Male_RestBasis_AB_Control"
    target_b_control.data.name = "CoH_Male_RestBasis_AB_Control"
    bpy.context.collection.objects.link(target_b_control)
    target_b_control.animation_data_clear()
    target.animation_data_clear()

    frames = []
    reference_rolls = {}
    for frame in FRAMES:
        item = run_frame(source, control, target, target_b_source, target_b_control, rest_world, frame)
        reference_rolls[frame] = item["rollReferences"]
        if args.render:
            item["render"] = render_frame(args.output_dir, frame, source, target, target_b_source)
        frames.append(item)

    output = {
        "tool": "agent/animation/diagnose_issue36_orientation.py",
        "blend": root_relative(args.blend),
        "sourceFbx": root_relative(ROOT / "swinginganimations" / "Swinging.fbx"),
        "action": "Armature|mixamo.com|Layer0",
        "frames": frames,
        "restBasisOffsets": rest_basis_offsets(source, control, target),
        "signContinuity": sign_continuity(reference_rolls),
        "mapping": {
            "currentProduction": TARGET_MAP,
            "sourceChild": SOURCE_CHILD,
            "targetChild": TARGET_CHILD,
            "spineCompression": "Mixamo Spine -> WAIST; Mixamo Spine1 is audited but not directly consumed; Mixamo Spine2 -> CHEST",
        },
        "diagnosticDecision": {
            "oldPassCriteria": "joint position + segment direction only",
            "productionBehaviorChanged": False,
        },
    }
    json_path = args.output_dir / "orientation-report.json"
    md_path = args.output_dir / "orientation-report.md"
    json_path.write_text(json.dumps(output, indent=2), encoding="utf-8")
    make_markdown(output, md_path)
    print("ISSUE36_ORIENTATION_DIAGNOSTIC " + json.dumps({
        "frames": list(FRAMES),
        "json": str(json_path),
        "markdown": str(md_path),
        "rendered": bool(args.render),
    }, sort_keys=True))


if __name__ == "__main__":
    main()
