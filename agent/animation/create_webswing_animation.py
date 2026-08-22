"""Author purpose-built Web Swing phase animations in Blender.

The script reconstructs one of the shipped CoH player rigs from a
GetAnimation2 runtime report, creates a real Blender armature, authors
semantic pose-space targets, and saves a normal .blend source artifact.  The
existing evaluated-pose ANIMX exporter and GetAnimation2 compiler remain the
only path from this authoring source to a runtime .anim file.

``--authoring v1`` retains the original arbitrary local-axis FK authoring as a
negative control.  The default ``v2`` path is intentionally narrow: it
currently authors only Male STRETCH through a geometric two-bone reach solve.
It does not use Blender IK on the reconstructed export bones; their edit tails
are synthetic and are not anatomical segment lengths.  V2 converts each
desired pose-space orientation back through the parent/rest hierarchy and
rejects any authored arm location or scale channel that is not rotation-only.
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Matrix, Quaternion, Vector


AUTHORED_BONES = [
    "HIPS", "WAIST", "CHEST",
    "COL_R", "UARMR", "LARMR", "HANDR",
    "COL_L", "UARML", "LARML", "HANDL",
    "ULEGR", "LLEGR", "FOOTR",
    "ULEGL", "LLEGL", "FOOTL",
    "NECK", "HEAD",
]

ARM_BONES = {
    "R": ("COL_R", "UARMR", "LARMR", "HANDR"),
    "L": ("COL_L", "UARML", "LARML", "HANDL"),
}

ROTATION_ONLY_LOCATION_TOLERANCE = 1.0e-5
ROTATION_ONLY_SCALE_TOLERANCE = 1.0e-5
REACH_VALIDATION_TOLERANCE = 2.0e-3

ACTION_SPECS = {
    "stretch": {
        "logical": "COHSOURCEDEV_WEBSWING_STRETCH",
        "frames": 30,
        "keyframes": [1, 6, 15, 24, 30],
        "profiles": [0.0, 0.65, -0.70, 0.45, 0.0],
        "description": "long asymmetric hang with a raised tether arm and trailing legs",
    },
    "tuck": {
        "logical": "COHSOURCEDEV_WEBSWING_TUCK",
        "frames": 24,
        "keyframes": [1, 5, 12, 19, 24],
        "profiles": [0.0, 0.45, -0.35, 0.30, 0.0],
        "description": "compact athletic bottom-of-arc compression",
    },
    "ascend": {
        "logical": "COHSOURCEDEV_WEBSWING_ASCEND",
        "frames": 28,
        "keyframes": [1, 6, 14, 22, 28],
        "profiles": [0.0, 0.55, -0.45, 0.35, 0.0],
        "description": "open outbound extension after the bottom tuck",
    },
}


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--rig-json", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--action", required=True, choices=sorted(ACTION_SPECS))
    parser.add_argument("--rig-type", required=True, choices=["male", "fem", "huge"])
    parser.add_argument(
        "--authoring", choices=["v1", "v2"], default="v2",
        help="authoring layer to use; v1 is the preserved arbitrary-FK negative control",
    )
    parser.add_argument(
        "--candidate", choices=["A", "B", "C"], default="B",
        help="bounded V2 reach candidate used by the visual iteration gate",
    )
    return parser.parse_args(argv)


def normalize_quaternion(values):
    q = Quaternion((values[3], values[0], values[1], values[2]))
    if q.magnitude <= 1.0e-12:
        return Quaternion((1.0, 0.0, 0.0, 0.0))
    q.normalize()
    return q


def game_quat_to_source(values):
    """Invert process_animx.c's runtime-to-ANIMX axis conversion."""
    game_q = normalize_quaternion(values)
    if game_q.w < 0.0:
        game_q.negate()
    angle = 2.0 * math.acos(max(-1.0, min(1.0, game_q.w)))
    sine = math.sin(angle * 0.5)
    if abs(sine) <= 1.0e-6:
        return Quaternion((1.0, 0.0, 0.0, 0.0))
    game_axis = Vector((game_q.x / sine, game_q.y / sine, game_q.z / sine))
    source_axis = Vector((game_axis.x, game_axis.z, -game_axis.y))
    return Quaternion(source_axis, angle)


def game_position_to_source(values):
    """Exact inverse of ConvertCoordsFrom3DSMAX in processanim.c."""
    return Vector((-values[0], -values[2], values[1]))


def load_rig(path):
    report = json.loads(path.read_text(encoding="utf-8"))
    bones = {bone["name"]: bone for bone in report["bones"]}
    by_id = {bone["id"]: bone for bone in report["bones"]}
    missing = [name for name in AUTHORED_BONES if name not in bones]
    if missing:
        raise ValueError(f"{path} is missing common Web Swing bones: {', '.join(missing)}")
    return report, bones, by_id


def ordered_bones(bones, by_id):
    ordered = []
    pending = dict(bones)
    while pending:
        progressed = False
        for name, bone in list(pending.items()):
            parent = by_id.get(bone["parent"])
            if parent is None or parent["name"] not in pending:
                ordered.append(bone)
                del pending[name]
                progressed = True
        if not progressed:
            raise ValueError("Rig hierarchy contains an unresolved parent")
    return ordered


def build_source_rest(bones, by_id):
    world = {}
    for bone in ordered_bones(bones, by_id):
        name = bone["name"]
        local_rotation = game_quat_to_source(bone["frame0LocalRotation"])
        local_translation = game_position_to_source(bone["frame0LocalTranslation"])
        parent = by_id.get(bone["parent"])
        if parent is None:
            world[name] = (local_rotation.copy(), local_translation.copy())
        else:
            parent_rotation, parent_translation = world[parent["name"]]
            world[name] = (
                local_rotation @ parent_rotation,
                parent_translation + (parent_rotation @ local_translation),
            )
    return world


def matrix_from_transform(rotation, translation):
    matrix = rotation.to_matrix().to_4x4()
    matrix.translation = translation
    return matrix


def create_armature(report, bones, by_id, rest_world, rig_type, logical_name):
    if bpy.context.object and bpy.context.object.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)

    armature_data = bpy.data.armatures.new(f"CoH_{rig_type.title()}_AnimationRig")
    armature_object = bpy.data.objects.new(armature_data.name, armature_data)
    bpy.context.collection.objects.link(armature_object)
    bpy.context.view_layer.objects.active = armature_object
    armature_object.select_set(True)
    armature_data.display_type = "BBONE"
    armature_data.axes_position = 0

    bpy.ops.object.mode_set(mode="EDIT")
    edit_bones = {}
    for bone in ordered_bones(bones, by_id):
        name = bone["name"]
        edit_bone = armature_data.edit_bones.new(name)
        parent = by_id.get(bone["parent"])
        if parent is not None:
            edit_bone.parent = edit_bones[parent["name"]]
            edit_bone.use_connect = False
        transform = matrix_from_transform(*rest_world[name])
        head = transform.translation
        tail = head + (transform.to_3x3() @ Vector((0.0, 0.25, 0.0)))
        edit_bone.head = head
        edit_bone.tail = tail if (tail - head).length > 1.0e-4 else head + Vector((0.0, 0.25, 0.0))
        edit_bone.matrix = transform
        edit_bones[name] = edit_bone
    bpy.ops.object.mode_set(mode="OBJECT")

    armature_object["coh_animation_source"] = "ANIMX/3ds-Max source frame"
    armature_object["coh_source_animation"] = f"{rig_type}/{logical_name}"
    armature_object["coh_reference_animation"] = report["animation"]
    armature_object["coh_rig_type"] = rig_type
    armature_object["coh_bone_count"] = len(bones)
    armature_object["coh_authored_bones"] = ",".join(AUTHORED_BONES)
    return armature_object


def clamp(value, lower, upper):
    return max(lower, min(upper, value))


def matrix_from_axes(x_axis, y_axis, z_axis):
    """Build a column-basis matrix from three orthonormal pose-space axes."""
    matrix = Matrix.Identity(3)
    matrix.col[0] = x_axis
    matrix.col[1] = y_axis
    matrix.col[2] = z_axis
    return matrix


def stable_basis(direction, up_reference):
    """Return a stable right-handed basis whose Y axis follows direction."""
    y_axis = direction.normalized()
    z_axis = up_reference - y_axis * up_reference.dot(y_axis)
    if z_axis.length <= 1.0e-6:
        fallback = Vector((0.0, 1.0, 0.0)) if abs(y_axis.y) < 0.9 else Vector((1.0, 0.0, 0.0))
        z_axis = fallback - y_axis * fallback.dot(y_axis)
    z_axis.normalize()
    x_axis = y_axis.cross(z_axis)
    if x_axis.length <= 1.0e-6:
        raise ValueError("Could not construct a stable pose-space basis")
    x_axis.normalize()
    z_axis = x_axis.cross(y_axis)
    z_axis.normalize()
    return matrix_from_axes(x_axis, y_axis, z_axis)


def rotation_between(source, target, max_angle=None):
    """Return a shortest-arc quaternion, optionally bounded in radians."""
    source = source.normalized()
    target = target.normalized()
    dot = clamp(source.dot(target), -1.0, 1.0)
    if dot > 1.0 - 1.0e-7:
        return Quaternion((1.0, 0.0, 0.0, 0.0))
    if dot < -1.0 + 1.0e-7:
        axis = source.cross(Vector((1.0, 0.0, 0.0)))
        if axis.length <= 1.0e-6:
            axis = source.cross(Vector((0.0, 1.0, 0.0)))
        axis.normalize()
        angle = math.pi
    else:
        axis = source.cross(target)
        axis.normalize()
        angle = math.acos(dot)
    if max_angle is not None:
        angle = min(angle, max_angle)
    return Quaternion(axis, angle)


def pose_matrix_for_segment(rest_matrix, rest_segment, target_head, target_segment, roll_reference):
    """Map a real rest-pose joint segment onto a target pose-space segment.

    This deliberately uses the reconstructed CoH child-origin vector rather
    than Blender's synthetic edit-bone tail.  The returned matrix is an
    object/pose-space matrix suitable for PoseBone.matrix.
    """
    rest_direction = rest_segment.normalized()
    rest_up = rest_matrix.to_3x3() @ Vector((0.0, 0.0, 1.0))
    if abs(rest_up.normalized().dot(rest_direction)) > 0.98:
        rest_up = rest_matrix.to_3x3() @ Vector((1.0, 0.0, 0.0))
    rest_basis = stable_basis(rest_direction, rest_up)
    target_basis = stable_basis(target_segment, roll_reference)
    delta_rotation = target_basis @ rest_basis.inverted()
    target_rotation = delta_rotation @ rest_matrix.to_3x3()
    result = target_rotation.to_4x4()
    result.translation = target_head
    return result


def rest_origin(rest_world, name):
    return rest_world[name][1].copy()


def pose_origin(armature_object, name):
    return armature_object.pose.bones[name].matrix.translation.copy()


def parent_pose_matrices(armature_object, name):
    """Return the evaluated parent pose and its rest matrix for conversion."""
    bone = armature_object.data.bones[name]
    if bone.parent is None:
        return Matrix.Identity(4), Matrix.Identity(4)
    parent_pose = armature_object.pose.bones[bone.parent.name].matrix.copy()
    parent_rest = bone.parent.matrix_local.copy()
    return parent_pose, parent_rest


def pose_to_local_basis(armature_object, name, pose_matrix):
    """Convert an armature-space pose matrix to Blender's local pose basis."""
    bone = armature_object.data.bones[name]
    parent_pose, parent_rest = parent_pose_matrices(armature_object, name)
    return bone.convert_local_to_pose(
        pose_matrix,
        bone.matrix_local,
        parent_matrix=parent_pose,
        parent_matrix_local=parent_rest,
        invert=True,
    )


def set_pose_matrix_rotation_only(armature_object, name, pose_matrix):
    """Apply an armature-space target using only a local quaternion channel.

    Assigning ``PoseBone.matrix`` directly is allowed to synthesize local
    location channels when the requested child origin is not already implied
    by its parent's rotation.  The desired target is therefore converted back
    through Blender's parent/rest pose relation first.  The local basis is
    checked before its rotation is assigned; the caller's hard gate then
    checks the evaluated channels again after the complete arm is solved.
    """
    pose_bone = armature_object.pose.bones[name]
    local_basis = pose_to_local_basis(armature_object, name, pose_matrix)
    local_location = local_basis.translation.copy()
    local_scale = local_basis.to_3x3().to_scale()
    location_error = local_location.length
    scale_error = max(abs(value - 1.0) for value in local_scale)
    if location_error > ROTATION_ONLY_LOCATION_TOLERANCE:
        raise ValueError(
            f"{name} pose target requires local translation "
            f"{location_error:.9g}; target head is not parent-implied"
        )
    if scale_error > ROTATION_ONLY_SCALE_TOLERANCE:
        raise ValueError(
            f"{name} pose target requires local scale error "
            f"{scale_error:.9g}; rotation-only conversion is invalid"
        )

    pose_bone.rotation_mode = "QUATERNION"
    pose_bone.location = (0.0, 0.0, 0.0)
    pose_bone.rotation_quaternion = local_basis.to_3x3().to_quaternion()
    pose_bone.scale = (1.0, 1.0, 1.0)
    bpy.context.view_layer.update()
    return {
        "locationMagnitude": location_error,
        "scaleError": scale_error,
    }


def rotate_current_pose_bone(armature_object, name, axis, angle):
    """Apply a bounded semantic joint rotation around its current origin."""
    pose_bone = armature_object.pose.bones[name]
    current = pose_bone.matrix.copy()
    pivot = current.translation.copy()
    delta = Matrix.Translation(pivot) @ Matrix.Rotation(angle, 4, axis) @ Matrix.Translation(-pivot)
    return set_pose_matrix_rotation_only(armature_object, name, delta @ current)


def clamp_target_to_reach(shoulder, target, upper_length, lower_length, fallback_direction):
    """Clamp an unreachable hand target without changing either limb length."""
    vector = target - shoulder
    distance = vector.length
    if distance <= 1.0e-6:
        direction = fallback_direction.normalized()
        distance = 0.0
    else:
        direction = vector.normalized()
    minimum = abs(upper_length - lower_length) + 1.0e-4
    maximum = upper_length + lower_length - 1.0e-4
    clamped_distance = clamp(distance, minimum, maximum)
    return shoulder + direction * clamped_distance, distance, clamped_distance


def solve_two_bone_reach(shoulder, target, upper_length, lower_length, elbow_pole, fallback_pole):
    """Solve a clamped two-bone chain with a stable pose-space pole plane."""
    direction = target - shoulder
    distance = direction.length
    if distance <= 1.0e-6:
        direction = Vector((1.0, 0.0, 0.0))
        distance = 1.0
    direction.normalize()

    pole_vector = elbow_pole - shoulder
    pole_projection = pole_vector - direction * pole_vector.dot(direction)
    if pole_projection.length <= 1.0e-6:
        pole_vector = fallback_pole - shoulder
        pole_projection = pole_vector - direction * pole_vector.dot(direction)
    if pole_projection.length <= 1.0e-6:
        for fallback_axis in (
            Vector((0.0, 0.0, 1.0)),
            Vector((1.0, 0.0, 0.0)),
            Vector((0.0, 1.0, 0.0)),
        ):
            candidate_projection = fallback_axis - direction * fallback_axis.dot(direction)
            if candidate_projection.length > 1.0e-6:
                pole_projection = candidate_projection
                break
    if pole_projection.length <= 1.0e-6:
        raise ValueError("Could not construct a stable elbow pole plane")
    pole_direction = pole_projection.normalized()

    along = (upper_length * upper_length + distance * distance - lower_length * lower_length) / (2.0 * distance)
    along = clamp(along, -upper_length, upper_length)
    bend_height = math.sqrt(max(0.0, upper_length * upper_length - along * along))
    elbow = shoulder + direction * along + pole_direction * bend_height
    return elbow, pole_direction, distance


def arm_names(side):
    side = side.upper()
    if side not in ("R", "L"):
        raise ValueError(f"Unsupported arm side: {side}")
    return {
        "collar": f"COL_{side}",
        "upper": f"UARM{side}",
        "lower": f"LARM{side}",
        "hand": f"HAND{side}",
        "hand_marker": f"WEP{side}",
    }


def derive_roll_reference(upper_direction, lower_direction, pole_direction, previous):
    """Derive one continuous roll normal from the solved elbow plane.

    The pole direction is the anatomical bend direction.  Its cross product
    with the solved upper-arm direction is the normal of the complete limb
    plane, so the upper arm and forearm share one roll reference instead of
    each independently using a global up axis.  The previous keyframe only
    resolves the sign ambiguity at a nearly straight or poorly conditioned
    pose; it is never the source of the limb plane.
    """
    plane_normal = upper_direction.cross(lower_direction)
    if plane_normal.length <= 1.0e-6:
        plane_normal = pole_direction.cross(upper_direction)
    if plane_normal.length <= 1.0e-6:
        plane_normal = pole_direction.copy()
    if plane_normal.length <= 1.0e-6:
        raise ValueError("Could not construct a stable solved arm roll reference")
    plane_normal.normalize()
    if previous is not None and plane_normal.dot(previous) < 0.0:
        plane_normal.negate()
    return plane_normal


def derive_hand_orientation(forearm_direction, tether_direction, roll_reference, up_hint=None):
    """Build a stable wrist frame from forearm, tether, and elbow-plane data."""
    tether = tether_direction.copy()
    if tether.length <= 1.0e-6:
        tether = forearm_direction.copy()
    tether.normalize()

    # The hand points predominantly along the implied tether, while a small
    # forearm contribution prevents a visibly broken wrist at the attachment.
    hand_forward = (tether * 0.78 + forearm_direction * 0.22)
    if hand_forward.length <= 1.0e-6:
        hand_forward = tether
    hand_forward.normalize()

    hand_up = roll_reference - hand_forward * roll_reference.dot(hand_forward)
    if hand_up.length <= 1.0e-6 and up_hint is not None:
        hand_up = up_hint - hand_forward * up_hint.dot(hand_forward)
    if hand_up.length <= 1.0e-6:
        hand_up = forearm_direction.cross(hand_forward)
    if hand_up.length <= 1.0e-6:
        hand_up = Vector((0.0, 0.0, 1.0))
    hand_up.normalize()
    if hand_up.dot(roll_reference) < 0.0:
        hand_up.negate()
    return hand_forward, hand_up


def pose_reach_arm(
    armature_object,
    rest_world,
    side,
    hand_target,
    elbow_pole,
    shoulder_weight=0.0,
    hand_orientation=None,
    previous_roll_reference=None,
):
    """Place one arm from semantic targets rather than local Euler angles.

    ``hand_target`` and ``elbow_pole`` are object/pose-space points.  The
    shoulder weight is deliberately bounded and only affects a small
    clavicle-opening adjustment before the two-bone solve.  ``hand_orientation``
    is an optional ``(tether_direction, up_hint)`` pair in pose space for the
    palm/wrist.  The final hand frame is derived from that direction, the
    solved forearm, and the common elbow-plane normal.
    """
    names = arm_names(side)
    upper = names["upper"]
    lower = names["lower"]
    hand = names["hand"]
    collar = names["collar"]
    marker = names["hand_marker"]
    shoulder_weight = clamp(float(shoulder_weight), 0.0, 1.0)

    # COL_R/COL_L is allowed to contribute only a bounded shoulder opening.
    # It is driven by the target direction, never by arbitrary collar-axis
    # numbers and never used to hide a bad upper/lower-arm solve.
    collar_bone = armature_object.pose.bones[collar]
    upper_bone = armature_object.pose.bones[upper]
    collar_head = collar_bone.matrix.translation.copy()
    current_offset = upper_bone.matrix.translation - collar_head
    target_direction = hand_target - collar_head
    if shoulder_weight > 1.0e-6 and target_direction.length > 1.0e-6:
        current_direction = current_offset.normalized()
        desired_direction = current_direction.lerp(target_direction.normalized(), 0.18 * shoulder_weight)
        desired_direction += Vector((0.0, 0.0, 0.08 * shoulder_weight))
        if desired_direction.length > 1.0e-6:
            delta = rotation_between(current_direction, desired_direction, math.radians(10.0) * shoulder_weight)
            pivot = collar_head
            collar_matrix = collar_bone.matrix.copy()
            collar_delta = Matrix.Translation(pivot) @ delta.to_matrix().to_4x4() @ Matrix.Translation(-pivot)
            set_pose_matrix_rotation_only(armature_object, collar, collar_delta @ collar_matrix)
            bpy.context.view_layer.update()

    shoulder = pose_origin(armature_object, upper)
    upper_length = (rest_origin(rest_world, lower) - rest_origin(rest_world, upper)).length
    lower_length = (rest_origin(rest_world, hand) - rest_origin(rest_world, lower)).length
    fallback_direction = rest_origin(rest_world, lower) - rest_origin(rest_world, upper)
    target, requested_distance, clamped_distance = clamp_target_to_reach(
        shoulder, hand_target, upper_length, lower_length, fallback_direction,
    )
    fallback_pole = shoulder + Vector((0.0, 0.0, 1.0))
    elbow, pole_direction, solved_distance = solve_two_bone_reach(
        shoulder, target, upper_length, lower_length, elbow_pole, fallback_pole,
    )

    upper_direction = (elbow - shoulder).normalized()
    lower_direction = (target - elbow).normalized()
    roll_reference = derive_roll_reference(
        upper_direction,
        lower_direction,
        pole_direction,
        previous_roll_reference,
    )

    rest_upper_matrix = armature_object.data.bones[upper].matrix_local.copy()
    rest_lower_matrix = armature_object.data.bones[lower].matrix_local.copy()
    rest_hand_matrix = armature_object.data.bones[hand].matrix_local.copy()
    rest_upper_segment = rest_origin(rest_world, lower) - rest_origin(rest_world, upper)
    rest_lower_segment = rest_origin(rest_world, hand) - rest_origin(rest_world, lower)
    set_pose_matrix_rotation_only(
        armature_object,
        upper,
        pose_matrix_for_segment(
            rest_upper_matrix,
            rest_upper_segment,
            shoulder,
            elbow - shoulder,
            roll_reference,
        ),
    )
    bpy.context.view_layer.update()
    set_pose_matrix_rotation_only(
        armature_object,
        lower,
        pose_matrix_for_segment(
            rest_lower_matrix,
            rest_lower_segment,
            elbow,
            target - elbow,
            roll_reference,
        ),
    )
    bpy.context.view_layer.update()

    if hand_orientation is None:
        tether_direction = lower_direction
        up_hint = None
    else:
        tether_direction, up_hint = hand_orientation
    hand_forward, hand_up = derive_hand_orientation(
        lower_direction,
        tether_direction,
        roll_reference,
        up_hint,
    )
    if marker in rest_world:
        rest_hand_segment = rest_origin(rest_world, marker) - rest_origin(rest_world, hand)
    else:
        rest_hand_segment = rest_lower_segment
    set_pose_matrix_rotation_only(
        armature_object,
        hand,
        pose_matrix_for_segment(
            rest_hand_matrix,
            rest_hand_segment,
            target,
            hand_forward,
            hand_up,
        ),
    )
    bpy.context.view_layer.update()

    return {
        "side": side.upper(),
        "shoulder": tuple(shoulder),
        "elbow": tuple(elbow),
        "hand": tuple(target),
        "pole": tuple(pole_direction),
        "rollReference": tuple(roll_reference),
        "upperDirection": tuple(upper_direction),
        "lowerDirection": tuple(lower_direction),
        "handForward": tuple(hand_forward),
        "handUp": tuple(hand_up),
        "upperLength": upper_length,
        "lowerLength": lower_length,
        "requestedDistance": requested_distance,
        "clampedDistance": clamped_distance,
        "solvedDistance": solved_distance,
        "shoulderWeight": shoulder_weight,
    }


def delta_quaternion(*angles):
    """Compose local source-frame axis-angle deltas in authoring order."""
    result = Quaternion((1.0, 0.0, 0.0, 0.0))
    for axis, degrees in angles:
        if abs(degrees) > 1.0e-6:
            result = Quaternion(axis, math.radians(degrees)) @ result
    return result


def add_profile(base, amount, profile):
    return base + amount * profile


def phase_pose_v1(action, profile):
    """Return the preserved V1 arbitrary local rotations.

    The source frame is the ANIMX/3ds-Max frame used by Blender here: source Z
    is game up, source Y is the inverse of game forward, and source X is the
    inverse of game right.  The signs below are therefore deliberately kept
    in this source-frame authoring layer instead of being scattered through
    the exporter.
    """
    if action == "stretch":
        values = {
            "HIPS": ((1, 0, 0), add_profile(15, 2, profile)),
            "WAIST": ((1, 0, 0), add_profile(10, 3, profile)),
            "CHEST": ((1, 0, 0), add_profile(8, 4, profile)),
            "COL_R": ((0, 1, 0), add_profile(38, 3, profile)),
            "UARMR": ((0, 1, 0), add_profile(57, 4, profile)),
            "LARMR": ((0, 1, 0), add_profile(29, 5, profile)),
            "HANDR": ((0, 1, 0), add_profile(8, 2, profile)),
            "COL_L": ((0, 1, 0), add_profile(22, 3, profile)),
            "UARML": ((1, 0, 0), add_profile(-20, 4, profile)),
            "LARML": ((0, 1, 0), add_profile(12, 3, profile)),
            "HANDL": ((1, 0, 0), add_profile(-10, 3, profile)),
            "ULEGR": ((1, 0, 0), add_profile(-16, 4, profile)),
            "LLEGR": ((1, 0, 0), add_profile(10, 6, profile)),
            "FOOTR": ((1, 0, 0), add_profile(-8, 3, profile)),
            "ULEGL": ((1, 0, 0), add_profile(-8, 3, profile)),
            "LLEGL": ((1, 0, 0), add_profile(20, 5, profile)),
            "FOOTL": ((1, 0, 0), add_profile(-4, 3, profile)),
            "NECK": ((1, 0, 0), add_profile(6, 2, profile)),
            "HEAD": ((1, 0, 0), add_profile(5, 2, profile)),
        }
    elif action == "tuck":
        values = {
            "HIPS": ((1, 0, 0), add_profile(8, 2, profile)),
            "WAIST": ((1, 0, 0), add_profile(18, 3, profile)),
            "CHEST": ((1, 0, 0), add_profile(25, 4, profile)),
            "COL_R": ((0, 1, 0), add_profile(31, 3, profile)),
            "UARMR": ((0, 1, 0), add_profile(50, 4, profile)),
            "LARMR": ((0, 1, 0), add_profile(53, 6, profile)),
            "HANDR": ((0, 1, 0), add_profile(7, 2, profile)),
            "COL_L": ((0, 1, 0), add_profile(28, 4, profile)),
            "UARML": ((1, 0, 0), add_profile(-30, 5, profile)),
            "LARML": ((1, 0, 0), add_profile(-45, 7, profile)),
            "HANDL": ((1, 0, 0), add_profile(-20, 4, profile)),
            "ULEGR": ((1, 0, 0), add_profile(-72, 5, profile)),
            "LLEGR": ((1, 0, 0), add_profile(108, 7, profile)),
            "FOOTR": ((1, 0, 0), add_profile(-18, 5, profile)),
            "ULEGL": ((1, 0, 0), add_profile(-58, 4, profile)),
            "LLEGL": ((1, 0, 0), add_profile(90, 6, profile)),
            "FOOTL": ((1, 0, 0), add_profile(-10, 4, profile)),
            "NECK": ((1, 0, 0), add_profile(11, 3, profile)),
            "HEAD": ((1, 0, 0), add_profile(8, 3, profile)),
        }
    else:
        values = {
            "HIPS": ((1, 0, 0), add_profile(18, 3, profile)),
            "WAIST": ((1, 0, 0), add_profile(13, 3, profile)),
            "CHEST": ((1, 0, 0), add_profile(7, 4, profile)),
            "COL_R": ((0, 1, 0), add_profile(43, 4, profile)),
            "UARMR": ((0, 1, 0), add_profile(63, 5, profile)),
            "LARMR": ((0, 1, 0), add_profile(34, 5, profile)),
            "HANDR": ((0, 1, 0), add_profile(8, 2, profile)),
            "COL_L": ((0, 1, 0), add_profile(25, 4, profile)),
            "UARML": ((1, 0, 0), add_profile(-25, 5, profile)),
            "LARML": ((1, 0, 0), add_profile(-34, 6, profile)),
            "HANDL": ((1, 0, 0), add_profile(-12, 3, profile)),
            "ULEGR": ((1, 0, 0), add_profile(-20, 5, profile)),
            "LLEGR": ((1, 0, 0), add_profile(-8, 5, profile)),
            "FOOTR": ((1, 0, 0), add_profile(-10, 3, profile)),
            "ULEGL": ((1, 0, 0), add_profile(-14, 4, profile)),
            "LLEGL": ((1, 0, 0), add_profile(-19, 5, profile)),
            "FOOTL": ((1, 0, 0), add_profile(-5, 3, profile)),
            "NECK": ((1, 0, 0), add_profile(6, 2, profile)),
            "HEAD": ((1, 0, 0), add_profile(5, 2, profile)),
        }

    # A small roll wave keeps the silhouette alive without introducing root
    # translation or a fake runtime orientation system.
    roll = profile * 3.0
    pose = {}
    for bone, (axis, angle) in values.items():
        if bone in ("HIPS", "WAIST", "CHEST"):
            pose[bone] = delta_quaternion((axis, angle), ((0, 0, 1), roll))
        else:
            pose[bone] = delta_quaternion((axis, angle))
    return pose


V2_CANDIDATES = {
    # Candidate A intentionally makes the elbow pole compete with the head.
    # It is retained as a bounded negative iteration, not as a final pose.
    "A": {
        "right_hand_offset": Vector((-1.42, -0.18, 1.52)),
        "right_pole_offset": Vector((-0.18, 0.22, 0.90)),
        "left_hand_offset": Vector((1.42, 0.40, -0.10)),
        "left_pole_offset": Vector((0.10, -0.05, 0.85)),
        "right_hand_forward": Vector((-0.02, -0.08, 1.0)),
        "left_hand_forward": Vector((0.0, -0.30, -1.0)),
    },
    # Candidate B is the primary solve: a slight down/ahead pole keeps the
    # right elbow below the reach line and separates it from the head.
    "B": {
        "right_hand_offset": Vector((-1.50, -0.48, 1.35)),
        "right_pole_offset": Vector((-0.10, -0.65, -0.90)),
        "left_hand_offset": Vector((1.45, 0.45, -0.12)),
        "left_pole_offset": Vector((0.10, -0.20, 0.82)),
        "right_hand_forward": Vector((-0.02, -0.10, 1.0)),
        "left_hand_forward": Vector((0.0, -0.30, -1.0)),
    },
    # Candidate C keeps the same plane but lowers the hand slightly for a
    # clearer silhouette when the side camera is used.
    "C": {
        "right_hand_offset": Vector((-1.45, -0.58, 1.25)),
        "right_pole_offset": Vector((-0.12, -0.82, -0.62)),
        "left_hand_offset": Vector((1.40, 0.50, -0.16)),
        "left_pole_offset": Vector((0.12, -0.22, 0.78)),
        "right_hand_forward": Vector((-0.02, -0.12, 1.0)),
        "left_hand_forward": Vector((0.0, -0.28, -1.0)),
    },
}


def reset_authored_pose(armature_object):
    """Reset keyed bones to the reconstructed rest pose before each key."""
    for bone_name in AUTHORED_BONES:
        pose_bone = armature_object.pose.bones[bone_name]
        pose_bone.rotation_mode = "QUATERNION"
        pose_bone.location = (0.0, 0.0, 0.0)
        pose_bone.rotation_quaternion = (1.0, 0.0, 0.0, 0.0)
        pose_bone.scale = (1.0, 1.0, 1.0)
    bpy.context.view_layer.update()


def keyframe_current_pose(armature_object, frame):
    """Key the validated local quaternion/location/scale channels."""
    for bone_name in AUTHORED_BONES:
        pose_bone = armature_object.pose.bones[bone_name]
        pose_bone.rotation_mode = "QUATERNION"
        # Location and scale are deliberately keyed as zero/identity so the
        # ANIMX source records the proven rotation-only contract explicitly.
        pose_bone.keyframe_insert(data_path="location", frame=frame, group=bone_name)
        pose_bone.keyframe_insert(data_path="rotation_quaternion", frame=frame, group=bone_name)
        pose_bone.keyframe_insert(data_path="scale", frame=frame, group=bone_name)


def validate_reach_pose(armature_object, metrics):
    """Reject a pose if evaluated joint origins lose the solved geometry."""
    names = arm_names(metrics["side"])
    upper_actual = (pose_origin(armature_object, names["lower"]) -
                    pose_origin(armature_object, names["upper"])).length
    lower_actual = (pose_origin(armature_object, names["hand"]) -
                    pose_origin(armature_object, names["lower"])).length
    elbow_error = (pose_origin(armature_object, names["lower"]) -
                   Vector(metrics["elbow"])).length
    hand_error = (pose_origin(armature_object, names["hand"]) -
                  Vector(metrics["hand"])).length
    validation = {
        "upperLengthError": abs(upper_actual - metrics["upperLength"]),
        "forearmLengthError": abs(lower_actual - metrics["lowerLength"]),
        "elbowTargetError": elbow_error,
        "handTargetError": hand_error,
    }
    metrics["validation"] = validation
    print(
        "WEBSWING_REACH "
        f"side={metrics['side']} "
        f"upperLengthError={validation['upperLengthError']:.9g} "
        f"forearmLengthError={validation['forearmLengthError']:.9g} "
        f"elbowTargetError={elbow_error:.9g} "
        f"handTargetError={hand_error:.9g}"
    )
    if abs(upper_actual - metrics["upperLength"]) > REACH_VALIDATION_TOLERANCE:
        raise ValueError(f"{metrics['side']} upper-arm origin length drifted during solve")
    if abs(lower_actual - metrics["lowerLength"]) > REACH_VALIDATION_TOLERANCE:
        raise ValueError(f"{metrics['side']} forearm origin length drifted during solve")
    if elbow_error > REACH_VALIDATION_TOLERANCE or hand_error > REACH_VALIDATION_TOLERANCE:
        raise ValueError(f"{metrics['side']} evaluated joint origins missed the reach target")


def validate_rotation_only_pose(armature_object, frame):
    """Report and hard-fail if any authored arm channel cheats with TRS."""
    frame_report = {"frame": frame, "right": {}, "left": {}}
    failures = []
    for side, bone_names in ARM_BONES.items():
        side_key = "right" if side == "R" else "left"
        for bone_name in bone_names:
            pose_bone = armature_object.pose.bones[bone_name]
            location_magnitude = pose_bone.location.length
            scale_error = max(abs(value - 1.0) for value in pose_bone.scale)
            item = {
                "locationMagnitude": location_magnitude,
                "scaleError": scale_error,
            }
            frame_report[side_key][bone_name] = item
            print(
                "WEBSWING_ROTATION_ONLY "
                f"frame={frame} side={side_key} bone={bone_name} "
                f"locationMagnitude={location_magnitude:.9g} "
                f"scaleError={scale_error:.9g}"
            )
            if location_magnitude > ROTATION_ONLY_LOCATION_TOLERANCE:
                failures.append(
                    f"{bone_name} location {location_magnitude:.9g} "
                    f"> {ROTATION_ONLY_LOCATION_TOLERANCE:.9g}"
                )
            if scale_error > ROTATION_ONLY_SCALE_TOLERANCE:
                failures.append(
                    f"{bone_name} scale error {scale_error:.9g} "
                    f"> {ROTATION_ONLY_SCALE_TOLERANCE:.9g}"
                )
    if failures:
        raise ValueError(
            f"Rotation-only arm validation failed at frame {frame}: "
            + "; ".join(failures)
        )
    return frame_report


def semantic_stretch_targets(armature_object, candidate, profile):
    spec = V2_CANDIDATES[candidate]
    chest = pose_origin(armature_object, "CHEST")
    right_target = chest + spec["right_hand_offset"]
    right_target += Vector((-0.05, 0.06, 0.04)) * profile
    left_target = chest + spec["left_hand_offset"]
    left_target += Vector((0.04, -0.05, -0.03)) * profile
    right_pole = chest + spec["right_pole_offset"]
    left_pole = chest + spec["left_pole_offset"]
    return {
        "rightTarget": right_target,
        "rightPole": right_pole,
        "leftTarget": left_target,
        "leftPole": left_pole,
        "rightHandOrientation": (spec["right_hand_forward"], Vector((0.0, -1.0, 0.0))),
        "leftHandOrientation": (spec["left_hand_forward"], Vector((0.0, 1.0, 0.0))),
    }


def author_keyframes_v2(armature_object, rest_world, action_name, candidate):
    """Author V2 semantic poses; currently the narrow Male STRETCH gate."""
    if action_name != "stretch":
        raise ValueError("V2 authoring is intentionally gated to STRETCH until Male STRETCH passes visual review")
    if armature_object.get("coh_rig_type") != "male":
        raise ValueError("V2 authoring is intentionally gated to Male STRETCH; do not generate Fem/Huge yet")
    if candidate not in V2_CANDIDATES:
        raise ValueError(f"Unknown V2 candidate: {candidate}")

    spec = ACTION_SPECS[action_name]
    scene = bpy.context.scene
    scene.frame_start = 1
    scene.frame_end = spec["frames"]
    scene.render.fps = 30
    action = bpy.data.actions.new(
        f"{spec['logical']}_{armature_object['coh_rig_type'].upper()}_V2_BLENDER_ACTION",
    )
    armature_object.animation_data_create()
    armature_object.animation_data.action = action

    metrics = []
    rotation_only_metrics = []
    previous_roll_reference = {"R": None, "L": None}
    for frame, profile in zip(spec["keyframes"], spec["profiles"]):
        scene.frame_set(frame)
        reset_authored_pose(armature_object)

        # A small whole-body lean preserves the existing hanging/trailing read
        # without adding a runtime orientation system or root translation.
        rotate_current_pose_bone(
            armature_object,
            "HIPS",
            Vector((1.0, 0.0, 0.0)),
            math.radians(8.0 + 1.5 * profile),
        )
        bpy.context.view_layer.update()
        targets = semantic_stretch_targets(armature_object, candidate, profile)

        # The same semantic target->chain principle is used for the free arm;
        # it is not a second set of arbitrary local-axis commands.
        right_metrics = pose_reach_arm(
            armature_object,
            rest_world,
            "R",
            targets["rightTarget"],
            targets["rightPole"],
            shoulder_weight=0.35,
            hand_orientation=targets["rightHandOrientation"],
            previous_roll_reference=previous_roll_reference["R"],
        )
        previous_roll_reference["R"] = Vector(right_metrics["rollReference"])
        left_metrics = pose_reach_arm(
            armature_object,
            rest_world,
            "L",
            targets["leftTarget"],
            targets["leftPole"],
            shoulder_weight=0.20,
            hand_orientation=targets["leftHandOrientation"],
            previous_roll_reference=previous_roll_reference["L"],
        )
        previous_roll_reference["L"] = Vector(left_metrics["rollReference"])
        validate_reach_pose(armature_object, right_metrics)
        validate_reach_pose(armature_object, left_metrics)
        rotation_only_metrics.append(validate_rotation_only_pose(armature_object, frame))
        keyframe_current_pose(armature_object, frame)
        metrics.append({"frame": frame, "profile": profile, "right": right_metrics, "left": left_metrics})

    fcurves = list(getattr(action, "fcurves", []))
    if not fcurves and hasattr(action, "layers"):
        for layer in action.layers:
            for strip in layer.strips:
                for channelbag in strip.channelbags:
                    fcurves.extend(channelbag.fcurves)
    for fcurve in fcurves:
        for keyframe in fcurve.keyframe_points:
            keyframe.interpolation = "BEZIER"

    scene.frame_set(1)
    bpy.context.view_layer.update()
    fcurve_count, keyframe_point_count = action_metrics(action)
    right_rest_length = (rest_origin(rest_world, "LARMR") - rest_origin(rest_world, "UARMR")).length
    right_forearm_length = (rest_origin(rest_world, "HANDR") - rest_origin(rest_world, "LARMR")).length
    left_rest_length = (rest_origin(rest_world, "LARML") - rest_origin(rest_world, "UARML")).length
    left_forearm_length = (rest_origin(rest_world, "HANDL") - rest_origin(rest_world, "LARML")).length
    armature_object["coh_authoring_version"] = "V2_POSE_SPACE_GEOMETRIC_TWO_BONE"
    armature_object["coh_v2_candidate"] = candidate
    armature_object["coh_v2_rest_joint_source"] = "UARM/LARM/HAND origin distances; Blender edit tails intentionally ignored"
    armature_object["coh_v2_right_upper_arm_length"] = right_rest_length
    armature_object["coh_v2_right_forearm_length"] = right_forearm_length
    armature_object["coh_v2_left_upper_arm_length"] = left_rest_length
    armature_object["coh_v2_left_forearm_length"] = left_forearm_length
    armature_object["coh_v2_metrics"] = json.dumps(metrics, sort_keys=True)
    armature_object["coh_v2_rotation_only_metrics"] = json.dumps(rotation_only_metrics, sort_keys=True)
    armature_object["coh_v2_rotation_only_location_tolerance"] = ROTATION_ONLY_LOCATION_TOLERANCE
    armature_object["coh_v2_rotation_only_scale_tolerance"] = ROTATION_ONLY_SCALE_TOLERANCE
    armature_object["coh_v2_roll_method"] = "solved upper cross lower; pole-derived sign continuity"
    armature_object["coh_logical_animation"] = spec["logical"]
    armature_object["coh_clip_frames"] = spec["frames"]
    armature_object["coh_keyframe_frames"] = ",".join(str(frame) for frame in spec["keyframes"])
    armature_object["coh_keyframe_point_count"] = keyframe_point_count
    armature_object["coh_fcurve_count"] = fcurve_count
    armature_object["coh_pose_description"] = spec["description"]
    return metrics


def action_metrics(action):
    fcurves = list(getattr(action, "fcurves", []))
    if not fcurves and hasattr(action, "layers"):
        for layer in action.layers:
            for strip in layer.strips:
                for channelbag in strip.channelbags:
                    fcurves.extend(channelbag.fcurves)
    return len(fcurves), sum(len(curve.keyframe_points) for curve in fcurves)


def author_keyframes_v1(armature_object, action_name):
    spec = ACTION_SPECS[action_name]
    scene = bpy.context.scene
    scene.frame_start = 1
    scene.frame_end = spec["frames"]
    scene.render.fps = 30
    action = bpy.data.actions.new(f"{spec['logical']}_{armature_object['coh_rig_type'].upper()}_BLENDER_ACTION")
    armature_object.animation_data_create()
    armature_object.animation_data.action = action

    for frame, profile in zip(spec["keyframes"], spec["profiles"]):
        pose = phase_pose_v1(action_name, profile)
        for bone_name in AUTHORED_BONES:
            pose_bone = armature_object.pose.bones[bone_name]
            pose_bone.rotation_mode = "QUATERNION"
            pose_bone.rotation_quaternion = pose[bone_name]
            pose_bone.keyframe_insert(data_path="rotation_quaternion", frame=frame, group=bone_name)

    fcurves = list(getattr(action, "fcurves", []))
    if not fcurves and hasattr(action, "layers"):
        for layer in action.layers:
            for strip in layer.strips:
                for channelbag in strip.channelbags:
                    fcurves.extend(channelbag.fcurves)
    for fcurve in fcurves:
        for keyframe in fcurve.keyframe_points:
            keyframe.interpolation = "BEZIER"

    scene.frame_set(1)
    bpy.context.view_layer.update()
    fcurve_count, keyframe_point_count = action_metrics(action)
    armature_object["coh_logical_animation"] = spec["logical"]
    armature_object["coh_clip_frames"] = spec["frames"]
    armature_object["coh_keyframe_frames"] = ",".join(str(frame) for frame in spec["keyframes"])
    armature_object["coh_keyframe_point_count"] = keyframe_point_count
    armature_object["coh_fcurve_count"] = fcurve_count
    armature_object["coh_pose_description"] = spec["description"]


def main():
    args = parse_args()
    report, bones, by_id = load_rig(args.rig_json)
    spec = ACTION_SPECS[args.action]
    rest_world = build_source_rest(bones, by_id)
    armature_object = create_armature(report, bones, by_id, rest_world, args.rig_type, spec["logical"])
    if args.authoring == "v1":
        author_keyframes_v1(armature_object, args.action)
        armature_object["coh_authoring_version"] = "V1_ARBITRARY_LOCAL_AXIS_FK_NEGATIVE_CONTROL"
    else:
        metrics = author_keyframes_v2(armature_object, rest_world, args.action, args.candidate)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(args.output))
    print(
        "BLENDER_WEBSWING_CREATED "
        f"authoring={args.authoring} candidate={args.candidate if args.authoring == 'v2' else 'n/a'} "
        f"action={args.action} logical={spec['logical']} rig={args.rig_type} "
        f"blend={args.output} frames={spec['frames']} bones={len(bones)} "
        f"authored={len(AUTHORED_BONES)} keyframeFrames={armature_object['coh_keyframe_frames']} "
        f"keyframePoints={armature_object['coh_keyframe_point_count']} "
        f"fcurves={armature_object['coh_fcurve_count']} reference={report['animation']}"
    )


if __name__ == "__main__":
    main()
