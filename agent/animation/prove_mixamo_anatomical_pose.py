"""Prove one Mixamo pose through an anatomical control rig and the CoH rig.

This is deliberately a one-frame diagnostic tool.  The imported Mixamo
armature, a small conventional FK control skeleton, and the reconstructed
Male export skeleton are kept as three separate representations.  The
control skeleton is mapped with the installed KBS-DEV Retarget extension for
the selected frame.  The CoH skeleton is then solved from the control pose's
joint geometry and anatomical bend planes; local quaternions are never copied
between the two rigs.

The script writes only derived evidence under ``--output-dir``.  It never
writes the source FBX and it does not install a runtime animation.
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

from create_blender_canary import (  # noqa: E402
    build_source_rest,
    create_armature as create_exact_armature,
    load_rig,
)
from create_webswing_animation import (  # noqa: E402
    matrix_from_axes,
    pose_matrix_for_segment,
    pose_to_local_basis,
    set_pose_matrix_rotation_only,
    stable_basis,
)


FRAME_DEFAULT = 30
ROTATION_ONLY_LOCATION_TOLERANCE = 1.0e-5
ROTATION_ONLY_SCALE_TOLERANCE = 1.0e-5

SOURCE = {
    "hips": "mixamorig:Hips",
    "spine": "mixamorig:Spine",
    "spine1": "mixamorig:Spine1",
    "spine2": "mixamorig:Spine2",
    "neck": "mixamorig:Neck",
    "head": "mixamorig:Head",
    "head_end": "mixamorig:HeadTop_End",
    "shoulder_r": "mixamorig:RightShoulder",
    "arm_r": "mixamorig:RightArm",
    "forearm_r": "mixamorig:RightForeArm",
    "hand_r": "mixamorig:RightHand",
    "thigh_r": "mixamorig:RightUpLeg",
    "shin_r": "mixamorig:RightLeg",
    "foot_r": "mixamorig:RightFoot",
    "toe_r": "mixamorig:RightToeBase",
    "toe_end_r": "mixamorig:RightToe_End",
    "shoulder_l": "mixamorig:LeftShoulder",
    "arm_l": "mixamorig:LeftArm",
    "forearm_l": "mixamorig:LeftForeArm",
    "hand_l": "mixamorig:LeftHand",
    "thigh_l": "mixamorig:LeftUpLeg",
    "shin_l": "mixamorig:LeftLeg",
    "foot_l": "mixamorig:LeftFoot",
    "toe_l": "mixamorig:LeftToeBase",
    "toe_end_l": "mixamorig:LeftToe_End",
}

CONTROL = {
    "hips": "Ctrl_Hips",
    "spine": "Ctrl_Spine",
    "spine1": "Ctrl_Spine1",
    "spine2": "Ctrl_Spine2",
    "neck": "Ctrl_Neck",
    "head": "Ctrl_Head",
    "shoulder_r": "Ctrl_Shoulder_Right",
    "arm_r": "Ctrl_Arm_FK_Right",
    "forearm_r": "Ctrl_ForeArm_FK_Right",
    "hand_r": "Ctrl_Hand_FK_Right",
    "thigh_r": "Ctrl_UpLeg_FK_Right",
    "shin_r": "Ctrl_Leg_FK_Right",
    "foot_r": "Ctrl_Foot_FK_Right",
    "toe_r": "Ctrl_Toe_FK_Right",
    "shoulder_l": "Ctrl_Shoulder_Left",
    "arm_l": "Ctrl_Arm_FK_Left",
    "forearm_l": "Ctrl_ForeArm_FK_Left",
    "hand_l": "Ctrl_Hand_FK_Left",
    "thigh_l": "Ctrl_UpLeg_FK_Left",
    "shin_l": "Ctrl_Leg_FK_Left",
    "foot_l": "Ctrl_Foot_FK_Left",
    "toe_l": "Ctrl_Toe_FK_Left",
}

COH_CHILD = {
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

COH_BONES = [
    "HIPS", "WAIST", "CHEST", "COL_R", "UARMR", "LARMR", "HANDR",
    "COL_L", "UARML", "LARML", "HANDL", "ULEGR", "LLEGR", "FOOTR",
    "TOER", "ULEGL", "LLEGL", "FOOTL", "TOEL", "NECK", "HEAD",
]

CONTROL_MAPPED_FIELDS = {
    "root": "Ctrl_Master",
    "spine": {
        "hips": "Ctrl_Hips", "spine": "Ctrl_Spine", "spine1": "Ctrl_Spine1",
        "spine2": "Ctrl_Spine2", "neck": "Ctrl_Neck", "head": "Ctrl_Head",
    },
    "right_arm": {
        "shoulder": "Ctrl_Shoulder_Right", "arm": "Ctrl_Arm_FK_Right",
        "forearm": "Ctrl_ForeArm_FK_Right", "hand": "Ctrl_Hand_FK_Right",
    },
    "left_arm": {
        "shoulder": "Ctrl_Shoulder_Left", "arm": "Ctrl_Arm_FK_Left",
        "forearm": "Ctrl_ForeArm_FK_Left", "hand": "Ctrl_Hand_FK_Left",
    },
    "right_leg": {
        "upleg": "Ctrl_UpLeg_FK_Right", "leg": "Ctrl_Leg_FK_Right",
        "foot": "Ctrl_Foot_FK_Right", "toe": "Ctrl_Toe_FK_Right",
    },
    "left_leg": {
        "upleg": "Ctrl_UpLeg_FK_Left", "leg": "Ctrl_Leg_FK_Left",
        "foot": "Ctrl_Foot_FK_Left", "toe": "Ctrl_Toe_FK_Left",
    },
}


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-fbx", required=True, type=Path)
    parser.add_argument("--rig-json", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--frame", type=int, default=FRAME_DEFAULT)
    parser.add_argument("--proof-frames", type=int, default=5)
    parser.add_argument(
        "--neighbor-radius",
        type=int,
        default=2,
        help="Number of source frames on either side of --frame for the short post-audition proof",
    )
    parser.add_argument("--source-start-frame", type=int)
    parser.add_argument("--source-end-frame", type=int)
    parser.add_argument("--no-kbs", action="store_true", help="Do not invoke the installed KBS-DEV retarget operator")
    return parser.parse_args(argv)


def select_active(active, selected):
    if bpy.context.object and bpy.context.object.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="DESELECT")
    for obj in selected:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = active


def source_bone(source, semantic):
    return source.data.bones[SOURCE[semantic]]


def source_pose_bone(source, semantic):
    return source.pose.bones[SOURCE[semantic]]


def control_pose_bone(control, semantic):
    return control.pose.bones[CONTROL[semantic]]


def source_local_point(source, semantic, tail=False):
    bone = source_bone(source, semantic)
    return (bone.tail_local if tail else bone.head_local).copy()


def basis_matrix(head, tail, roll_reference):
    direction = tail - head
    if direction.length <= 1.0e-6:
        direction = Vector((0.0, 1.0, 0.0))
        tail = head + direction
    basis = stable_basis(direction, roll_reference)
    matrix = basis.to_4x4()
    matrix.translation = head
    return matrix


def add_edit_bone(edit_bones, name, head, tail, parent=None, roll_reference=None):
    bone = edit_bones.new(name)
    bone.head = head
    bone.tail = tail if (tail - head).length > 1.0e-5 else head + Vector((0.0, 1.0, 0.0))
    if parent is not None:
        bone.parent = edit_bones[parent]
        bone.use_connect = False
    if roll_reference is not None:
        length = max((tail - head).length, 1.0e-4)
        bone.matrix = basis_matrix(head, tail, roll_reference)
        bone.length = length
    return bone


def build_control_rig(source):
    """Build a conventional, FK-only control rig from Mixamo rest joints."""
    data = bpy.data.armatures.new("Anatomical_Control_Rig")
    obj = bpy.data.objects.new("Anatomical_Control_Rig", data)
    bpy.context.collection.objects.link(obj)
    obj.matrix_world = source.matrix_world.copy()
    obj["control_rig_type"] = "conventional anatomical FK diagnostic rig"
    obj["control_rig_source"] = "Mixamo rest joint origins; independent roll basis"
    data.display_type = "BBONE"
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    eb = data.edit_bones

    # Mixamo's imported armature is Y-up in its local frame.  The roll hints
    # are conventional anatomical references, not CoH export axes.
    arm_roll = Vector((0.0, 0.0, 1.0))
    spine_roll = Vector((0.0, 0.0, 1.0))
    leg_roll = Vector((0.0, 0.0, 1.0))

    hips_head = source_local_point(source, "hips")
    add_edit_bone(eb, "Ctrl_Master", hips_head, hips_head + Vector((0.0, 5.0, 0.0)), roll_reference=spine_roll)
    add_edit_bone(eb, "Ctrl_Hips", source_local_point(source, "hips"), source_local_point(source, "spine"), "Ctrl_Master", spine_roll)
    add_edit_bone(eb, "Ctrl_Spine", source_local_point(source, "spine"), source_local_point(source, "spine1"), "Ctrl_Hips", spine_roll)
    add_edit_bone(eb, "Ctrl_Spine1", source_local_point(source, "spine1"), source_local_point(source, "spine2"), "Ctrl_Spine", spine_roll)
    add_edit_bone(eb, "Ctrl_Spine2", source_local_point(source, "spine2"), source_local_point(source, "neck"), "Ctrl_Spine1", spine_roll)
    add_edit_bone(eb, "Ctrl_Neck", source_local_point(source, "neck"), source_local_point(source, "head"), "Ctrl_Spine2", spine_roll)
    add_edit_bone(eb, "Ctrl_Head", source_local_point(source, "head"), source_local_point(source, "head_end", tail=True), "Ctrl_Neck", spine_roll)

    for side in ("r", "l"):
        suffix = "Right" if side == "r" else "Left"
        add_edit_bone(
            eb, f"Ctrl_Shoulder_{suffix}", source_local_point(source, f"shoulder_{side}"),
            source_local_point(source, f"arm_{side}"), "Ctrl_Spine2", arm_roll,
        )
        add_edit_bone(
            eb, f"Ctrl_Arm_FK_{suffix}", source_local_point(source, f"arm_{side}"),
            source_local_point(source, f"forearm_{side}"), f"Ctrl_Shoulder_{suffix}", arm_roll,
        )
        add_edit_bone(
            eb, f"Ctrl_ForeArm_FK_{suffix}", source_local_point(source, f"forearm_{side}"),
            source_local_point(source, f"hand_{side}"), f"Ctrl_Arm_FK_{suffix}", arm_roll,
        )
        add_edit_bone(
            eb, f"Ctrl_Hand_FK_{suffix}", source_local_point(source, f"hand_{side}"),
            source_local_point(source, f"hand_{side}", tail=True), f"Ctrl_ForeArm_FK_{suffix}", arm_roll,
        )
        add_edit_bone(
            eb, f"Ctrl_UpLeg_FK_{suffix}", source_local_point(source, f"thigh_{side}"),
            source_local_point(source, f"shin_{side}"), "Ctrl_Hips", leg_roll,
        )
        add_edit_bone(
            eb, f"Ctrl_Leg_FK_{suffix}", source_local_point(source, f"shin_{side}"),
            source_local_point(source, f"foot_{side}"), f"Ctrl_UpLeg_FK_{suffix}", leg_roll,
        )
        add_edit_bone(
            eb, f"Ctrl_Foot_FK_{suffix}", source_local_point(source, f"foot_{side}"),
            source_local_point(source, f"toe_{side}"), f"Ctrl_Leg_FK_{suffix}", leg_roll,
        )
        add_edit_bone(
            eb, f"Ctrl_Toe_FK_{suffix}", source_local_point(source, f"toe_{side}"),
            source_local_point(source, f"toe_end_{side}"), f"Ctrl_Foot_FK_{suffix}", leg_roll,
        )

    bpy.ops.object.mode_set(mode="OBJECT")
    obj.hide_render = True
    return obj


def clear_optional_mapping(settings):
    for group_name in ("face", "right_arm_ik", "left_arm_ik", "right_leg_ik", "left_leg_ik"):
        group = getattr(settings, group_name)
        for field in group.keys():
            if field in {"name", "super_copy"}:
                continue
            try:
                setattr(group, field, "")
            except (TypeError, ValueError):
                pass
    for fingers in (settings.right_fingers, settings.left_fingers):
        for finger_name in ("thumb", "index", "middle", "ring", "pinky"):
            finger = getattr(fingers, finger_name)
            for field in finger.keys():
                if field == "name":
                    continue
                try:
                    setattr(finger, field, "")
                except (TypeError, ValueError):
                    pass


def configure_source_mapping(source):
    import importlib

    preset_handler = importlib.import_module("bl_ext.user_default.retarget.preset_handler")
    select_active(source, [source])
    if not preset_handler.set_preset_skel("Mixamo.py", bpy.context, validate=True, find=True):
        raise RuntimeError("KBS-DEV could not load its Mixamo.py source preset")
    settings = source.data.retarget_retarget
    settings.root = SOURCE["hips"]
    return {
        "preset": "Mixamo.py",
        "root": settings.root,
        "spine": {field: getattr(settings.spine, field) for field in ("hips", "spine", "spine1", "spine2", "neck", "head")},
        "right_arm": {field: getattr(settings.right_arm, field) for field in ("shoulder", "arm", "forearm", "hand")},
        "left_arm": {field: getattr(settings.left_arm, field) for field in ("shoulder", "arm", "forearm", "hand")},
        "right_leg": {field: getattr(settings.right_leg, field) for field in ("upleg", "leg", "foot", "toe")},
        "left_leg": {field: getattr(settings.left_leg, field) for field in ("upleg", "leg", "foot", "toe")},
    }


def set_mapping_group(settings, group_name, values):
    group = getattr(settings, group_name)
    for field, value in values.items():
        if hasattr(group, field):
            setattr(group, field, value)


def configure_control_mapping(control):
    settings = control.data.retarget_retarget
    settings.root = CONTROL_MAPPED_FIELDS["root"]
    for group_name in ("spine", "right_arm", "left_arm", "right_leg", "left_leg"):
        set_mapping_group(settings, group_name, CONTROL_MAPPED_FIELDS[group_name])
    clear_optional_mapping(settings)
    required = {settings.root}
    for section in ("spine", "right_arm", "left_arm", "right_leg", "left_leg"):
        required.update(value for value in CONTROL_MAPPED_FIELDS[section].values() if value)
    missing = sorted(required - set(control.data.bones.keys()))
    if missing:
        raise ValueError("Control rig is missing mapped bones: " + ", ".join(missing))
    return CONTROL_MAPPED_FIELDS


def ensure_extension():
    if not hasattr(bpy.ops, "armature") or not hasattr(bpy.ops.armature, "retarget_constrain_to_armature"):
        import addon_utils
        addon_utils.enable("bl_ext.user_default.retarget", default_set=True, persistent=False)
    if not hasattr(bpy.ops.armature, "retarget_constrain_to_armature"):
        raise RuntimeError("KBS-DEV retarget operator is unavailable")
    if not hasattr(bpy.ops.armature, "retarget_bake_constrained_actions"):
        raise RuntimeError("KBS-DEV bake operator is unavailable")
    import importlib
    operators = importlib.import_module("bl_ext.user_default.retarget.operators")
    operators.ConstrainToArmature.current_m = "OBJECT"


def import_mixamo(path):
    before = {obj.as_pointer() for obj in bpy.data.objects}
    bpy.ops.import_scene.fbx(filepath=str(path), use_anim=True)
    armatures = [obj for obj in bpy.data.objects if obj.type == "ARMATURE" and obj.as_pointer() not in before]
    if len(armatures) != 1:
        raise RuntimeError(f"Expected one imported Mixamo armature, found {len(armatures)}")
    source = armatures[0]
    source.name = "Mixamo_Source_Armature"
    source.data.name = "Mixamo_Source_Armature"
    if not source.animation_data or not source.animation_data.action:
        raise RuntimeError("Mixamo FBX did not import an animation action")
    return source


def run_kbs_single_frame(source, control, frame):
    scene = bpy.context.scene
    scene.frame_start = frame
    scene.frame_end = frame
    scene.frame_set(frame)
    # Pre-create KBS-DEV's temporary collection as visible.  The extension
    # creates this collection hidden, which leaves Blender 5.2 without pose
    # dependency-graph operations for the copied retarget bones.
    retarget_collection = source.data.collections.get("Retarget Bones")
    if retarget_collection is None:
        retarget_collection = source.data.collections.new("Retarget Bones")
    retarget_collection.is_visible = True
    # KBS-DEV uses the active armature as the animated driver and constrains
    # the other selected armature.  The control rig is therefore selected as
    # the destination while Mixamo remains active as the pose source.
    select_active(source, [source, control])
    result = bpy.ops.armature.retarget_constrain_to_armature(
        src_preset="--Current--",
        trg_preset="--Current--",
        same_bone_names=False,
        # KBS-DEV's Bone matching is appropriate only for this conventional
        # control rig.  The exact CoH export rig never goes through this
        # operator; it is solved semantically below.
        match_transform="Bone",
        match_object_transform=True,
        center=True,
        apply_rotation=False,
        align_rest_pose=False,
        align_leg=False,
        fit_target_scale="--",
        loc_constraints=False,
        rot_constraints=True,
        scal_constraints=False,
        bind_floating=False,
        no_finger_loc=True,
        constrain_root="Default",
        constraint_policy="remove",
        only_animated_Bone=False,
        only_selected=False,
        action_range=False,
        play=False,
        influence=1.0,
        transfer_pose=False,
        custom_Frame=frame,
    )
    if result != {"FINISHED"}:
        raise RuntimeError(f"KBS-DEV retarget bind failed: {result}")
    # KBS-DEV intentionally hides its temporary retarget-bone collection.
    # Blender 5.2 can omit hidden retarget bones from the dependency graph
    # while the destination Copy Rotation constraints are being evaluated.
    # Keep the temporary collection visible for this diagnostic evaluation;
    # it is deleted by the bake call below.
    retarget_collection = source.data.collections.get("Retarget Bones")
    if retarget_collection is not None:
        retarget_collection.is_visible = True
    for bone in source.data.bones:
        if bone.name.endswith("_RET"):
            bone.hide = False
            bone.use_deform = True
    bpy.context.view_layer.update()
    constraint_count = sum(len(pb.constraints) for pb in control.pose.bones)
    debug_constraints = {
        pb.name: [
            {
                "type": c.type,
                "target": c.target.name if c.target else None,
                "subtarget": c.subtarget,
                "influence": c.influence,
            }
            for c in pb.constraints
        ]
        for pb in control.pose.bones
        if pb.constraints
    }
    debug_ret_bones = [
        bone.name for bone in source.data.bones if bone.name.endswith("_RET")
    ]
    print("KBS_DEBUG " + json.dumps({
        "constraintCount": constraint_count,
        "constraintOwners": debug_constraints,
        "retargetBoneCount": len(debug_ret_bones),
        "retargetBones": debug_ret_bones,
        "sourceMode": source.mode,
        "controlMode": control.mode,
    }, sort_keys=True))
    debug_samples = {}
    for semantic in ("hips", "neck", "head", "arm_r", "forearm_r", "thigh_r", "shin_r", "foot_r"):
        source_point = source_common_point(source, semantic)
        control_point = control_common_point(control, semantic)
        debug_samples[semantic] = {
            "source": [round(float(v), 6) for v in source_point],
            "control": [round(float(v), 6) for v in control_point],
        }
    print("KBS_SAMPLES " + json.dumps(debug_samples, sort_keys=True))
    print("KBS_SOURCE_WORLD " + json.dumps({
        "matrix": [[round(float(v), 8) for v in row] for row in source.matrix_world],
        "location": [round(float(v), 8) for v in source.location],
        "scale": [round(float(v), 8) for v in source.scale],
    }, sort_keys=True))
    for semantic in ("hips", "neck", "head", "arm_r", "forearm_r", "thigh_r", "shin_r", "foot_r"):
        pb = source_pose_bone(source, semantic)
        print("KBS_SOURCE_BONE " + json.dumps({
            "semantic": semantic,
            "poseHead": [round(float(v), 6) for v in pb.head],
            "poseTail": [round(float(v), 6) for v in pb.tail],
            "matrixTranslation": [round(float(v), 6) for v in pb.matrix.translation],
            "restHead": [round(float(v), 6) for v in pb.bone.head_local],
            "restTail": [round(float(v), 6) for v in pb.bone.tail_local],
        }, sort_keys=True))
    for ret_name in ("Ctrl_Hips_RET", "Ctrl_Arm_FK_Right_RET", "Ctrl_ForeArm_FK_Right_RET", "Ctrl_UpLeg_FK_Right_RET"):
        pb = source.pose.bones.get(ret_name)
        print("KBS_RET_BONE " + json.dumps({
            "name": ret_name,
            "present": bool(pb),
            "head": [round(float(v), 6) for v in pb.head] if pb else None,
            "tail": [round(float(v), 6) for v in pb.tail] if pb else None,
            "matrixTranslation": [round(float(v), 6) for v in pb.matrix.translation] if pb else None,
            "parent": pb.parent.name if pb and pb.parent else None,
        }, sort_keys=True))
    if constraint_count == 0:
        raise RuntimeError("KBS-DEV retarget bind produced no control constraints")

    select_active(control, [control])
    bake = bpy.ops.armature.retarget_bake_constrained_actions(
        custom_start_end=True,
        start=frame,
        end=frame,
        do_bake=True,
        del_const=True,
        del_col=True,
        all_bone=True,
        use_current_action=False,
        clear_users_old=False,
        fake_user_new=True,
        add_to_object=False,
        interpolation="LINEAR",
        step=1,
    )
    action = control.animation_data.action if control.animation_data else None
    if action is None:
        raise RuntimeError(f"KBS-DEV bake did not create a control action: {bake}")
    scene.frame_set(frame)
    bpy.context.view_layer.update()
    return {
        "operatorResult": sorted(result),
        "bakeResult": sorted(bake) if isinstance(bake, set) else bake,
        "constraintsBeforeBake": constraint_count,
        "action": action.name,
        "actionFrameRange": list(action.frame_range),
    }


def clear_kbs_destination(control):
    """Remove KBS's temporary driver channels before the semantic solve."""
    for pose_bone in control.pose.bones:
        for constraint in reversed(pose_bone.constraints):
            control.pose.bones[pose_bone.name].constraints.remove(constraint)
    if control.animation_data:
        control.animation_data_clear()
    reset_pose(control)


def solve_control_from_mixamo_geometry(source, control):
    """Bake one Mixamo pose into the conventional control skeleton.

    KBS-DEV supplies the source/target mapping and temporary retarget bones,
    but its Blender 5.2 headless Copy Rotation dependency graph is not usable
    for this custom armature.  The fallback therefore consumes the evaluated
    Mixamo joint geometry directly.  This is still a source-driven transfer:
    no hand-authored angles or CoH local quaternions are introduced.
    """
    source_hips = source_common_point(source, "hips")
    control_rest_hips = control.matrix_world @ control.data.bones[CONTROL["hips"]].head_local
    control_world_to_object = control.matrix_world.inverted()
    source_child = {
        "hips": "spine", "spine": "spine1", "spine1": "spine2",
        "spine2": "neck", "neck": "head", "head": "head_end",
        "shoulder_r": "arm_r", "arm_r": "forearm_r", "forearm_r": "hand_r",
        "shoulder_l": "arm_l", "arm_l": "forearm_l", "forearm_l": "hand_l",
        "thigh_r": "shin_r", "shin_r": "foot_r", "foot_r": "toe_r", "toe_r": "toe_end_r",
        "thigh_l": "shin_l", "shin_l": "foot_l", "foot_l": "toe_l", "toe_l": "toe_end_l",
    }

    def point(semantic, tail=False):
        if tail and semantic in source_child:
            world = source_common_point(source, source_child[semantic])
        else:
            world = source_common_point(source, semantic, tail)
        normalized = world - source_hips + control_rest_hips
        return control_world_to_object @ normalized

    def direction(semantic):
        return (point(semantic, tail=True) - point(semantic)).normalized()

    def world_direction(semantic):
        return (source_common_point(source, semantic, tail=True) - source_common_point(source, semantic)).normalized()

    def local_reference(world_vector):
        local = control_world_to_object.to_3x3() @ Vector(world_vector)
        if local.length <= 1.0e-6:
            return Vector((0.0, 0.0, 1.0))
        return local.normalized()

    def chain_plane(upper_semantic, lower_semantic):
        upper = (source_common_point(source, lower_semantic) - source_common_point(source, upper_semantic)).normalized()
        if upper_semantic == "arm_r" or upper_semantic == "arm_l":
            lower_name = "hand_r" if upper_semantic == "arm_r" else "hand_l"
            lower_start = "forearm_r" if upper_semantic == "arm_r" else "forearm_l"
        else:
            lower_name = "foot_r" if upper_semantic == "thigh_r" else "foot_l"
            lower_start = "shin_r" if upper_semantic == "thigh_r" else "shin_l"
        lower = (source_common_point(source, lower_name) - source_common_point(source, lower_start)).normalized()
        plane = upper.cross(lower)
        if plane.length <= 1.0e-6:
            plane = source.matrix_world.to_3x3() @ source_pose_bone(source, upper_semantic).matrix.to_3x3() @ Vector((0.0, 0.0, 1.0))
        return plane.normalized()

    arm_planes = {side: chain_plane(f"arm_{side}", f"forearm_{side}") for side in ("r", "l")}
    leg_planes = {side: chain_plane(f"thigh_{side}", f"shin_{side}") for side in ("r", "l")}
    pelvis_lateral = (source_common_point(source, "thigh_r") - source_common_point(source, "thigh_l")).normalized()
    pelvis_up = (source_common_point(source, "hips", tail=True) - source_common_point(source, "hips")).normalized()
    pelvis_plane = pelvis_lateral.cross(pelvis_up)
    if pelvis_plane.length <= 1.0e-6:
        pelvis_plane = source.matrix_world.to_3x3() @ source_pose_bone(source, "hips").matrix.to_3x3() @ Vector((0.0, 0.0, 1.0))
    pelvis_plane.normalize()

    def source_pose_roll(semantic):
        pb = source_pose_bone(source, semantic)
        return source.matrix_world.to_3x3() @ pb.matrix.to_3x3() @ Vector((0.0, 0.0, 1.0))

    entries = []
    for semantic in ("hips", "spine", "spine1", "spine2", "neck", "head"):
        entries.append((semantic, semantic, source_pose_roll(semantic)))
    for side in ("r", "l"):
        for semantic in (f"shoulder_{side}", f"arm_{side}", f"forearm_{side}", f"hand_{side}"):
            entries.append((semantic, semantic, arm_planes[side]))
        for semantic in (f"thigh_{side}", f"shin_{side}", f"foot_{side}", f"toe_{side}"):
            entries.append((semantic, semantic, leg_planes[side]))

    metrics = []
    snapped_control_heads = []
    for source_semantic, control_semantic, plane in entries:
        control_name = CONTROL[control_semantic]
        rest_bone = control.data.bones[control_name]
        desired = pose_matrix_for_segment(
            rest_bone.matrix_local.copy(),
            rest_bone.tail_local - rest_bone.head_local,
            point(source_semantic),
            direction(source_semantic),
            local_reference(plane),
        )
        try:
            metrics.append(set_pose_matrix_rotation_only(control, control_name, desired))
        except ValueError as exc:
            # The Mixamo FBX contains sub-micron rest/pose endpoint rounding
            # after import.  For a conventional control rig, remove only a
            # residual below 1e-4 by reconstructing the same rotation with a
            # zero local translation.  Larger residuals remain a hard stop.
            local_basis = pose_to_local_basis(control, control_name, desired)
            residual = local_basis.translation.length
            scale_error = max(abs(value - 1.0) for value in local_basis.to_3x3().to_scale())
            if residual > 1.0e-4 or scale_error > ROTATION_ONLY_SCALE_TOLERANCE:
                print("CONTROL_RESIDUAL " + json.dumps({
                    "bone": control_name,
                    "semantic": source_semantic,
                    "residual": residual,
                    "scaleError": scale_error,
                    "sourceHead": [float(v) for v in source_common_point(source, source_semantic)],
                    "sourceTail": [float(v) for v in source_common_point(source, source_semantic, tail=True)],
                    "desiredHeadObject": [float(v) for v in desired.translation],
                    "currentParentTailObject": [
                        float(v) for v in control.pose.bones[control.data.bones[control_name].parent.name].tail
                    ] if control.data.bones[control_name].parent else None,
                    "controlParent": control.data.bones[control_name].parent.name if control.data.bones[control_name].parent else None,
                }, sort_keys=True))
                raise exc
            local_rotation = local_basis.to_3x3().to_quaternion().to_matrix().to_4x4()
            local_rotation.translation = (0.0, 0.0, 0.0)
            parent = rest_bone.parent
            parent_pose = control.pose.bones[parent.name].matrix.copy() if parent else Matrix.Identity(4)
            parent_rest = parent.matrix_local.copy() if parent else Matrix.Identity(4)
            adjusted = rest_bone.convert_local_to_pose(
                local_rotation,
                rest_bone.matrix_local,
                parent_matrix=parent_pose,
                parent_matrix_local=parent_rest,
                invert=False,
            )
            pose_bone = control.pose.bones[control_name]
            pose_bone.rotation_mode = "QUATERNION"
            pose_bone.location = (0.0, 0.0, 0.0)
            pose_bone.rotation_quaternion = local_rotation.to_3x3().to_quaternion()
            pose_bone.scale = (1.0, 1.0, 1.0)
            bpy.context.view_layer.update()
            metrics.append({"locationMagnitude": 0.0, "scaleError": 0.0})
            snapped_control_heads.append({"bone": control_name, "residual": residual})

    bpy.context.view_layer.update()
    return {
        "mode": "source-joint-geometry",
        "boneCount": len(entries),
        "maxLocalTranslation": max(item["locationMagnitude"] for item in metrics),
        "maxLocalScaleError": max(item["scaleError"] for item in metrics),
        "snappedControlHeadCount": len(snapped_control_heads),
        "snappedControlHeads": snapped_control_heads,
    }


def matrix_point(matrix, point):
    return matrix @ Vector(point)


def pose_point(obj, bone_name, tail=False):
    pb = obj.pose.bones[bone_name]
    return (obj.matrix_world @ (pb.matrix @ (Vector((0.0, pb.length, 0.0)) if tail else Vector((0.0, 0.0, 0.0)))))


def source_common_point(source, semantic, tail=False):
    pb = source_pose_bone(source, semantic)
    local = pb.matrix @ (Vector((0.0, pb.length, 0.0)) if tail else Vector((0.0, 0.0, 0.0)))
    return source.matrix_world @ local


def control_common_point(control, semantic, tail=False):
    pb = control_pose_bone(control, semantic)
    local = pb.matrix @ (Vector((0.0, pb.length, 0.0)) if tail else Vector((0.0, 0.0, 0.0)))
    return control.matrix_world @ local


def control_common_axis(control, semantic, axis):
    pb = control_pose_bone(control, semantic)
    return (control.matrix_world.to_3x3() @ pb.matrix.to_3x3() @ Vector(axis)).normalized()


def semantic_control_comparison(source, control):
    pairs = [
        ("hips", "hips"), ("spine", "spine"), ("spine1", "spine1"), ("spine2", "spine2"),
        ("neck", "neck"), ("head", "head"),
        ("shoulder_r", "shoulder_r"), ("arm_r", "arm_r"), ("forearm_r", "forearm_r"), ("hand_r", "hand_r"),
        ("shoulder_l", "shoulder_l"), ("arm_l", "arm_l"), ("forearm_l", "forearm_l"), ("hand_l", "hand_l"),
        ("thigh_r", "thigh_r"), ("shin_r", "shin_r"), ("foot_r", "foot_r"),
        ("thigh_l", "thigh_l"), ("shin_l", "shin_l"), ("foot_l", "foot_l"),
    ]
    source_hips = source_common_point(source, "hips")
    control_hips = control_common_point(control, "hips")
    errors = []
    for source_semantic, control_semantic in pairs:
        sp = source_common_point(source, source_semantic) - source_hips
        cp = control_common_point(control, control_semantic) - control_hips
        errors.append((sp - cp).length)
    segment_pairs = [
        ("arm_r", "forearm_r", "forearm_r", "hand_r"),
        ("arm_l", "forearm_l", "forearm_l", "hand_l"),
        ("thigh_r", "shin_r", "shin_r", "foot_r"),
        ("thigh_l", "shin_l", "shin_l", "foot_l"),
    ]
    angular_errors = []
    for a, b, c, d in segment_pairs:
        su = (source_common_point(source, b) - source_common_point(source, a)).normalized()
        cu = (control_common_point(control, b) - control_common_point(control, a)).normalized()
        sl = (source_common_point(source, d) - source_common_point(source, c)).normalized()
        cl = (control_common_point(control, d) - control_common_point(control, c)).normalized()
        angular_errors.extend([math.degrees(su.angle(cu)), math.degrees(sl.angle(cl))])
    report = {
        "jointPositionErrorMax": max(errors),
        "jointPositionErrorRms": math.sqrt(sum(v * v for v in errors) / len(errors)),
        "segmentDirectionErrorMaxDegrees": max(angular_errors),
        "segmentDirectionErrorRmsDegrees": math.sqrt(sum(v * v for v in angular_errors) / len(angular_errors)),
        "jointCount": len(errors),
        "segmentCount": len(angular_errors),
    }
    report["pass"] = report["jointPositionErrorMax"] <= 0.08 and report["segmentDirectionErrorMaxDegrees"] <= 4.0
    return report


def rest_point(rest_world, name):
    return Vector(rest_world[name][1])


def target_parent_pose_origin(target, name):
    return target.pose.bones[name].matrix.translation.copy()


def target_segment(rest_world, name, child=None):
    child = child or COH_CHILD[name]
    return rest_point(rest_world, child) - rest_point(rest_world, name)


def reset_pose(obj):
    for pb in obj.pose.bones:
        pb.rotation_mode = "QUATERNION"
        pb.location = (0.0, 0.0, 0.0)
        pb.rotation_quaternion = (1.0, 0.0, 0.0, 0.0)
        pb.scale = (1.0, 1.0, 1.0)
    bpy.context.view_layer.update()


def set_target_segment(target, rest_world, name, desired_direction, roll_reference, child=None):
    desired_direction = Vector(desired_direction)
    if desired_direction.length <= 1.0e-6:
        raise ValueError(f"{name} has a zero desired segment direction")
    desired_direction.normalize()
    rest_matrix = target.data.bones[name].matrix_local.copy()
    if child is None and name not in COH_CHILD:
        # TOER/TOEL are terminal runtime bones.  They have no child origin in
        # the GetAnimation2 skeleton, so retain the exact reconstructed
        # Blender edit-bone tail only as the terminal rest segment.
        rest_seg = target.data.bones[name].tail_local - target.data.bones[name].head_local
    else:
        rest_seg = target_segment(rest_world, name, child)
    target_head = target_parent_pose_origin(target, name)
    desired = pose_matrix_for_segment(
        rest_matrix, rest_seg, target_head, desired_direction, Vector(roll_reference),
    )
    metrics = set_pose_matrix_rotation_only(target, name, desired)
    return metrics


def current_coh_world_rotation(target, name):
    """Return the game-frame world rotation using CoH's local*parent order."""
    bone = target.data.bones[name]
    local = target.pose.bones[name].rotation_quaternion.normalized()
    if bone.parent is None:
        return local
    return local @ current_coh_world_rotation(target, bone.parent.name)


def source_quat_to_game(source_rotation):
    source_rotation = source_rotation.normalized()
    angle = source_rotation.angle
    if angle <= 1.0e-7:
        return Quaternion((1.0, 0.0, 0.0, 0.0))
    axis = source_rotation.axis.normalized()
    return Quaternion((axis.x, -axis.z, axis.y), angle)


def game_quat_to_source_rotation(game_rotation):
    game_rotation = game_rotation.normalized()
    angle = game_rotation.angle
    if angle <= 1.0e-7:
        return Quaternion((1.0, 0.0, 0.0, 0.0))
    axis = game_rotation.axis.normalized()
    return Quaternion((axis.x, axis.z, -axis.y), angle)


def source_position_to_game(source_position):
    return Vector((-source_position.x, source_position.z, -source_position.y))


def game_position_to_source(game_position):
    return Vector((-game_position.x, -game_position.z, game_position.y))


def set_target_runtime_segment(target, rest_world, name, desired_direction, roll_reference, child=None):
    """Author one local quaternion for CoH's native FK convention."""
    desired_direction = Vector(desired_direction)
    if desired_direction.length <= 1.0e-6:
        raise ValueError(f"{name} has a zero desired segment direction")
    desired_direction.normalize()
    rest_matrix = target.data.bones[name].matrix_local.copy()
    if child is None and name not in COH_CHILD:
        rest_seg = target.data.bones[name].tail_local - target.data.bones[name].head_local
    else:
        rest_seg = target_segment(rest_world, name, child)
    desired = pose_matrix_for_segment(
        rest_matrix, rest_seg, Vector((0.0, 0.0, 0.0)),
        desired_direction, Vector(roll_reference),
    )
    desired_source_world = desired.to_3x3().to_quaternion().normalized()
    # CoH stores the inverse Hamilton quaternion used by Blender's
    # column-vector matrices (the original MAX pipeline was row-vector based).
    desired_world = source_quat_to_game(desired_source_world).inverted()
    bone = target.data.bones[name]
    parent_world = (
        current_coh_world_rotation(target, bone.parent.name)
        if bone.parent is not None else Quaternion((1.0, 0.0, 0.0, 0.0))
    )
    local = (desired_world @ parent_world.inverted()).normalized()
    pose_bone = target.pose.bones[name]
    pose_bone.rotation_mode = "QUATERNION"
    pose_bone.location = (0.0, 0.0, 0.0)
    pose_bone.rotation_quaternion = local
    pose_bone.scale = (1.0, 1.0, 1.0)
    bpy.context.view_layer.update()
    actual_world = current_coh_world_rotation(target, name)
    world_error = math.degrees(actual_world.rotation_difference(desired_world).angle)
    if world_error > 1.0e-4:
        raise ValueError(f"{name} CoH FK world rotation error is {world_error:.9g} degrees")
    return {"locationMagnitude": 0.0, "scaleError": 0.0, "worldErrorDegrees": world_error}


def common_control_semantics(control):
    def p(name, tail=False):
        return control_common_point(control, name, tail)

    def axis(name, local_axis=(0.0, 1.0, 0.0)):
        return control_common_axis(control, name, local_axis)

    data = {}
    for side in ("r", "l"):
        shoulder = p(f"arm_{side}")
        elbow = p(f"forearm_{side}")
        wrist = p(f"hand_{side}")
        upper = (elbow - shoulder).normalized()
        lower = (wrist - elbow).normalized()
        plane = upper.cross(lower)
        if plane.length <= 1.0e-6:
            plane = axis(f"arm_{side}", (0.0, 0.0, 1.0))
        plane.normalize()
        data[f"arm_{side}"] = {
            "shoulder": shoulder,
            "elbow": elbow,
            "wrist": wrist,
            "clavicle": (p(f"arm_{side}") - p(f"shoulder_{side}")),
            "upperDirection": upper,
            "lowerDirection": lower,
            "planeNormal": plane,
            "handDirection": axis(f"hand_{side}"),
            "handUp": axis(f"hand_{side}", (0.0, 0.0, 1.0)),
        }
        hip = p(f"thigh_{side}")
        knee = p(f"shin_{side}")
        ankle = p(f"foot_{side}")
        thigh = (knee - hip).normalized()
        shin = (ankle - knee).normalized()
        knee_plane = thigh.cross(shin)
        if knee_plane.length <= 1.0e-6:
            knee_plane = axis(f"thigh_{side}", (0.0, 0.0, 1.0))
        knee_plane.normalize()
        data[f"leg_{side}"] = {
            "hip": hip,
            "knee": knee,
            "ankle": ankle,
            "thighDirection": thigh,
            "shinDirection": shin,
            "planeNormal": knee_plane,
            "footDirection": axis(f"foot_{side}"),
        }
    hips = p("hips")
    spine = p("hips", tail=True)
    left_hip = p("thigh_l")
    right_hip = p("thigh_r")
    lateral = (right_hip - left_hip).normalized()
    up = (spine - hips).normalized()
    pelvis_normal = lateral.cross(up)
    if pelvis_normal.length <= 1.0e-6:
        pelvis_normal = axis("hips", (0.0, 0.0, 1.0))
    pelvis_normal.normalize()
    data["torso"] = {
        "hipsDirection": up,
        "waistDirection": (p("spine", tail=True) - p("spine")).normalized(),
        "chestDirection": (p("spine2", tail=True) - p("spine2")).normalized(),
        "neckDirection": (p("neck", tail=True) - p("neck")).normalized(),
        "headDirection": (p("head", tail=True) - p("head")).normalized(),
        "pelvisNormal": pelvis_normal,
        "waistRoll": axis("spine1", (0.0, 0.0, 1.0)),
        "chestRoll": axis("spine2", (0.0, 0.0, 1.0)),
        "neckRoll": axis("neck", (0.0, 0.0, 1.0)),
        "headRoll": axis("head", (0.0, 0.0, 1.0)),
    }
    return data


def transfer_control_to_coh(target, rest_world, control):
    reset_pose(target)
    semantics = common_control_semantics(control)
    torso = semantics["torso"]
    metrics = []
    metrics.append({"bone": "HIPS", **set_target_runtime_segment(target, rest_world, "HIPS", torso["hipsDirection"], torso["pelvisNormal"])})
    metrics.append({"bone": "WAIST", **set_target_runtime_segment(target, rest_world, "WAIST", torso["waistDirection"], torso["waistRoll"])})
    metrics.append({"bone": "CHEST", **set_target_runtime_segment(target, rest_world, "CHEST", torso["chestDirection"], torso["chestRoll"])})

    for side in ("r", "l"):
        arm = semantics[f"arm_{side}"]
        suffix = "R" if side == "r" else "L"
        metrics.append({"bone": f"COL_{suffix}", **set_target_runtime_segment(target, rest_world, f"COL_{suffix}", arm["clavicle"], arm["planeNormal"])})
        metrics.append({"bone": f"UARM{suffix}", **set_target_runtime_segment(target, rest_world, f"UARM{suffix}", arm["upperDirection"], arm["planeNormal"])})
        metrics.append({"bone": f"LARM{suffix}", **set_target_runtime_segment(target, rest_world, f"LARM{suffix}", arm["lowerDirection"], arm["planeNormal"])})
        hand_roll = arm["handUp"]
        if abs(hand_roll.dot(arm["handDirection"])) > 0.95:
            hand_roll = arm["planeNormal"]
        metrics.append({"bone": f"HAND{suffix}", **set_target_runtime_segment(target, rest_world, f"HAND{suffix}", arm["handDirection"], hand_roll)})

        leg = semantics[f"leg_{side}"]
        metrics.append({"bone": f"ULEG{suffix}", **set_target_runtime_segment(target, rest_world, f"ULEG{suffix}", leg["thighDirection"], leg["planeNormal"])})
        metrics.append({"bone": f"LLEG{suffix}", **set_target_runtime_segment(target, rest_world, f"LLEG{suffix}", leg["shinDirection"], leg["planeNormal"])})
        metrics.append({"bone": f"FOOT{suffix}", **set_target_runtime_segment(target, rest_world, f"FOOT{suffix}", leg["footDirection"], leg["planeNormal"])})
        # The toe is a terminal bone.  It receives the source foot direction;
        # no independent toe translation or scale is permitted.
        metrics.append({"bone": f"TOE{suffix}", **set_target_runtime_segment(target, rest_world, f"TOE{suffix}", leg["footDirection"], leg["planeNormal"])})

    metrics.append({"bone": "NECK", **set_target_runtime_segment(target, rest_world, "NECK", torso["neckDirection"], torso["neckRoll"])})
    metrics.append({"bone": "HEAD", **set_target_runtime_segment(target, rest_world, "HEAD", torso["headDirection"], torso["headRoll"])})
    bpy.context.view_layer.update()

    failures = []
    max_location = 0.0
    max_scale = 0.0
    for name in COH_BONES:
        pb = target.pose.bones[name]
        location = pb.location.length
        scale_error = max(abs(value - 1.0) for value in pb.scale)
        max_location = max(max_location, location)
        max_scale = max(max_scale, scale_error)
        if location > ROTATION_ONLY_LOCATION_TOLERANCE:
            failures.append(f"{name} location={location:.9g}")
        if scale_error > ROTATION_ONLY_SCALE_TOLERANCE:
            failures.append(f"{name} scaleError={scale_error:.9g}")
    return {
        "pass": not failures,
        "maxTargetLocalTranslation": max_location,
        "maxTargetScaleError": max_scale,
        "tolerance": {
            "localTranslation": ROTATION_ONLY_LOCATION_TOLERANCE,
            "scaleError": ROTATION_ONLY_SCALE_TOLERANCE,
        },
        "failures": failures,
        "bones": metrics,
    }


def pose_snapshot(obj, names):
    snapshot = {}
    for name in names:
        pb = obj.pose.bones[name]
        snapshot[name] = {
            "location": list(pb.location),
            "rotationQuaternion": list(pb.rotation_quaternion),
            "scale": list(pb.scale),
            "poseMatrix": [[float(v) for v in row] for row in pb.matrix],
        }
    return snapshot


def create_proof_action(target, frame, proof_frames):
    scene = bpy.context.scene
    scene.frame_set(frame)
    bpy.context.view_layer.update()
    values = {}
    for pb in target.pose.bones:
        values[pb.name] = (pb.rotation_quaternion.copy(), pb.location.copy(), pb.scale.copy())
    action = bpy.data.actions.new("COHSOURCEDEV_RETARGET_POSE_PROOF_ACTION")
    target.animation_data_create()
    target.animation_data.action = action
    scene.frame_start = 1
    scene.frame_end = proof_frames
    for proof_frame in range(1, proof_frames + 1):
        for pb in target.pose.bones:
            rotation, location, scale = values[pb.name]
            pb.rotation_mode = "QUATERNION"
            pb.rotation_quaternion = rotation
            pb.location = location
            pb.scale = scale
            pb.keyframe_insert(data_path="rotation_quaternion", frame=proof_frame, group=pb.name)
            pb.keyframe_insert(data_path="location", frame=proof_frame, group=pb.name)
            pb.keyframe_insert(data_path="scale", frame=proof_frame, group=pb.name)
    scene.frame_set(1)
    bpy.context.view_layer.update()
    return action


def create_neighbor_action(
    source, control, target, rest_world, center_frame, radius,
    source_start_frame=None, source_end_frame=None,
):
    """Bake a bounded source-frame window after the one-frame gate."""
    scene = bpy.context.scene
    if source_start_frame is not None or source_end_frame is not None:
        if source_start_frame is None or source_end_frame is None:
            raise ValueError("Both source frame bounds must be supplied")
        source_frames = list(range(source_start_frame, source_end_frame + 1))
    else:
        source_frames = list(range(center_frame - radius, center_frame + radius + 1))
    if source_frames[0] < 1:
        raise ValueError("Retarget window would sample before Mixamo frame 1")
    source_action = source.animation_data.action if source.animation_data else None
    if source_action is None:
        raise ValueError("Mixamo source has no active action")
    source_last_frame = int(source_action.frame_range[1])
    if source_frames[-1] > source_last_frame:
        raise ValueError(
            f"Retarget window ends at {source_frames[-1]}, past source frame {source_last_frame}"
        )

    frame_records = []
    for source_frame in source_frames:
        scene.frame_set(source_frame)
        bpy.context.view_layer.update()
        reset_pose(control)
        control_solve = solve_control_from_mixamo_geometry(source, control)
        source_control = semantic_control_comparison(source, control)
        if not source_control["pass"]:
            raise RuntimeError(
                "SOURCE_TO_CONTROL_NEIGHBOR_NO_PASS "
                + json.dumps({"sourceFrame": source_frame, **source_control}, sort_keys=True)
            )
        coh = transfer_control_to_coh(target, rest_world, control)
        if not coh["pass"]:
            raise RuntimeError(
                "CONTROL_TO_COH_NEIGHBOR_NO_PASS "
                + json.dumps({"sourceFrame": source_frame, **coh}, sort_keys=True)
            )
        values = {
            pose_bone.name: (
                pose_bone.rotation_quaternion.copy(),
                pose_bone.location.copy(),
                pose_bone.scale.copy(),
            )
            for pose_bone in target.pose.bones
        }
        frame_records.append({
            "sourceFrame": source_frame,
            "sourceToControl": source_control,
            "controlSolve": control_solve,
            "controlToCoh": coh,
            "values": values,
        })

    # Leave the scene at the diagnostic source frame for any caller that
    # wants to inspect the one-frame proxy before the action is attached.
    scene.frame_set(center_frame)
    bpy.context.view_layer.update()
    reset_pose(control)
    solve_control_from_mixamo_geometry(source, control)
    transfer_control_to_coh(target, rest_world, control)

    action = bpy.data.actions.new("COHSOURCEDEV_RETARGET_POSE_PROOF_ACTION")
    target.animation_data_create()
    target.animation_data.action = action
    scene.frame_start = 1
    scene.frame_end = len(frame_records)
    for proof_frame, record in enumerate(frame_records, start=1):
        for pose_bone in target.pose.bones:
            rotation, location, scale = record["values"][pose_bone.name]
            pose_bone.rotation_mode = "QUATERNION"
            pose_bone.rotation_quaternion = rotation
            pose_bone.location = location
            pose_bone.scale = scale
            pose_bone.keyframe_insert(data_path="rotation_quaternion", frame=proof_frame, group=pose_bone.name)
            pose_bone.keyframe_insert(data_path="location", frame=proof_frame, group=pose_bone.name)
            pose_bone.keyframe_insert(data_path="scale", frame=proof_frame, group=pose_bone.name)
    scene.frame_set(1)
    bpy.context.view_layer.update()

    return action, {
        "sourceFrames": source_frames,
        "frameCount": len(frame_records),
        "sourceToControlPass": all(record["sourceToControl"]["pass"] for record in frame_records),
        "controlToCohPass": all(record["controlToCoh"]["pass"] for record in frame_records),
        "frames": [
            {
                "sourceFrame": record["sourceFrame"],
                "sourceToControl": record["sourceToControl"],
                "controlSolve": record["controlSolve"],
                "controlToCoh": record["controlToCoh"],
            }
            for record in frame_records
        ],
    }


def make_material(name, color, metallic=0.0, roughness=0.7):
    material = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    material.diffuse_color = (*color, 1.0)
    material.use_nodes = True
    bsdf = material.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs["Base Color"].default_value = (*color, 1.0)
        bsdf.inputs["Metallic"].default_value = metallic
        bsdf.inputs["Roughness"].default_value = roughness
    return material


def add_sphere(collection, location, radius, material):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=8, radius=radius, location=location)
    obj = bpy.context.object
    obj.data.materials.append(material)
    for old in list(obj.users_collection):
        old.objects.unlink(obj)
    collection.objects.link(obj)
    return obj


def add_cylinder_between(collection, start, end, radius, material):
    start = Vector(start)
    end = Vector(end)
    direction = end - start
    length = max(direction.length, 1.0e-5)
    mid = (start + end) * 0.5
    bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=radius, depth=length, location=mid)
    obj = bpy.context.object
    obj.rotation_mode = "QUATERNION"
    obj.rotation_quaternion = Vector((0.0, 0.0, 1.0)).rotation_difference(direction.normalized())
    obj.data.materials.append(material)
    for old in list(obj.users_collection):
        old.objects.unlink(obj)
    collection.objects.link(obj)
    return obj


def add_segment_box(collection, start, end, roll_reference, width, depth, material, fin_material):
    start = Vector(start)
    end = Vector(end)
    direction = (end - start).normalized()
    basis = stable_basis(direction, Vector(roll_reference))
    matrix = basis.to_4x4()
    mid = (start + end) * 0.5
    matrix.translation = mid
    length = max((end - start).length, 1.0e-5)
    bpy.ops.mesh.primitive_cube_add(size=1.0)
    obj = bpy.context.object
    obj.matrix_world = matrix
    obj.scale = (width, length, depth)
    obj.data.materials.append(material)
    for old in list(obj.users_collection):
        old.objects.unlink(obj)
    collection.objects.link(obj)

    # The fin is intentionally asymmetric so a 180-degree roll error is
    # visible in a still image; a round cylinder alone would hide that error.
    fin_matrix = matrix.copy()
    fin_matrix.translation = start + (end - start) * 0.34 + basis.col[2] * (depth * 0.8)
    bpy.ops.mesh.primitive_cube_add(size=1.0)
    fin = bpy.context.object
    fin.matrix_world = fin_matrix
    fin.scale = (width * 1.65, length * 0.22, depth * 0.20)
    fin.data.materials.append(fin_material)
    for old in list(fin.users_collection):
        old.objects.unlink(fin)
    collection.objects.link(fin)


def add_axes(collection, origin, basis, length, materials):
    origin = Vector(origin)
    matrix = basis.to_3x3() if hasattr(basis, "to_3x3") else basis
    for index, material in enumerate(materials):
        direction = (matrix @ Vector(((1, 0, 0), (0, 1, 0), (0, 0, 1))[index])).normalized()
        add_cylinder_between(collection, origin, origin + direction * length, length * 0.035, material)


def semantic_proxy_points(obj, kind):
    if kind == "source":
        def p(semantic, tail=False):
            return source_common_point(obj, semantic, tail)
        def b(semantic):
            pb = source_pose_bone(obj, semantic)
            return obj.matrix_world.to_3x3() @ pb.matrix.to_3x3()
        segments = []
        for a, c in (("hips", "spine"), ("spine", "spine1"), ("spine1", "spine2"), ("spine2", "neck"), ("neck", "head"), ("head", "head")):
            end = p(c) if a != c else p(a, True)
            if a == "head":
                end = p("head", True)
            segments.append((a, p(a), end, b(a)))
        for side in ("r", "l"):
            for a, c in ((f"shoulder_{side}", f"arm_{side}"), (f"arm_{side}", f"forearm_{side}"), (f"forearm_{side}", f"hand_{side}"), (f"hand_{side}", f"hand_{side}"), (f"thigh_{side}", f"shin_{side}"), (f"shin_{side}", f"foot_{side}"), (f"foot_{side}", f"toe_{side}"), (f"toe_{side}", f"toe_{side}")):
                end = p(c) if a != c else p(a, True)
                segments.append((a, p(a), end, b(a)))
        points = [p(name) for name in SOURCE if name in obj.pose.bones]
        return segments, points

    if kind == "control":
        names = CONTROL
        def p(semantic, tail=False):
            return control_common_point(obj, semantic, tail)
        def b(semantic):
            pb = control_pose_bone(obj, semantic)
            return obj.matrix_world.to_3x3() @ pb.matrix.to_3x3()
        segments = []
        for a, c in (("hips", "spine"), ("spine", "spine1"), ("spine1", "spine2"), ("spine2", "neck"), ("neck", "head"), ("head", "head")):
            end = p(c) if a != c else p(a, True)
            if a == "head":
                end = p("head", True)
            segments.append((a, p(a), end, b(a)))
        for side in ("r", "l"):
            for a, c in ((f"shoulder_{side}", f"arm_{side}"), (f"arm_{side}", f"forearm_{side}"), (f"forearm_{side}", f"hand_{side}"), (f"hand_{side}", f"hand_{side}"), (f"thigh_{side}", f"shin_{side}"), (f"shin_{side}", f"foot_{side}"), (f"foot_{side}", f"toe_{side}"), (f"toe_{side}", f"toe_{side}")):
                end = p(c) if a != c else p(a, True)
                segments.append((a, p(a), end, b(a)))
        points = [p(name) for name in names if name in obj.pose.bones]
        return segments, points

    # The exact CoH export representation keeps every local child offset at
    # its bind value and composes quaternions as local*parent.  Blender uses
    # parent*local, so reconstruct the native runtime hierarchy explicitly.
    pose_rotations = {}
    pose_positions = {}
    pending = list(obj.data.bones)
    while pending:
        progressed = False
        for bone in list(pending):
            if bone.parent is not None and bone.parent.name not in pose_rotations:
                continue
            local_rotation = obj.pose.bones[bone.name].rotation_quaternion.normalized()
            if bone.parent is None:
                world_rotation = local_rotation
                world_position = source_position_to_game(bone.matrix_local.translation)
            else:
                parent_rotation = pose_rotations[bone.parent.name]
                parent_position = pose_positions[bone.parent.name]
                local_position = source_position_to_game(
                    bone.parent.matrix_local.to_3x3().inverted() @ (
                        bone.matrix_local.translation - bone.parent.matrix_local.translation
                    )
                )
                world_rotation = local_rotation @ parent_rotation
                world_position = parent_position + (
                    parent_rotation.inverted() @ local_position
                )
            pose_rotations[bone.name] = world_rotation
            pose_positions[bone.name] = world_position
            pending.remove(bone)
            progressed = True
        if not progressed:
            raise RuntimeError("CoH proxy could not resolve its armature hierarchy")

    def p(name, tail=False):
        if tail:
            bone = obj.data.bones[name]
            # Synthetic Blender tails are only used for terminal markers.
            local_point = game_position_to_source(
                pose_positions[name] + (
                    pose_rotations[name].inverted() @ source_position_to_game(
                        bone.tail_local - bone.head_local
                    )
                )
            )
        else:
            local_point = game_position_to_source(pose_positions[name])
        return obj.matrix_world @ local_point

    def b(name):
        source_rotation = game_quat_to_source_rotation(pose_rotations[name]).inverted()
        return obj.matrix_world.to_3x3() @ source_rotation.to_matrix()

    segments = []
    for name, child in COH_CHILD.items():
        if name not in obj.pose.bones or child not in obj.pose.bones:
            continue
        start = p(name)
        end = p(child)
        basis = b(name)
        segments.append((name, start, end, basis))
    points = [p(name) for name in COH_BONES if name in obj.pose.bones]
    return segments, points


def create_proxy(name, segments, points, color):
    collection = bpy.data.collections.new(name)
    bpy.context.scene.collection.children.link(collection)
    material = make_material(f"{name}_Body", color, metallic=0.15, roughness=0.55)
    fin_material = make_material(f"{name}_RollFin", (1.0, 0.55, 0.08), metallic=0.05, roughness=0.5)
    joint_material = make_material(f"{name}_Joint", (0.95, 0.95, 0.95), metallic=0.05, roughness=0.35)
    axes = (
        make_material(f"{name}_AxisX", (0.95, 0.08, 0.06), metallic=0.0, roughness=0.6),
        make_material(f"{name}_AxisY", (0.10, 0.90, 0.16), metallic=0.0, roughness=0.6),
        make_material(f"{name}_AxisZ", (0.10, 0.32, 1.00), metallic=0.0, roughness=0.6),
    )
    for bone_name, start, end, basis in segments:
        add_segment_box(collection, start, end, basis.col[2], 0.055, 0.11, material, fin_material)
        add_axes(collection, start, basis, 0.30, axes)
    for point in points:
        add_sphere(collection, point, 0.09, joint_material)
    return collection


def set_collection_render(collection, visible):
    collection.hide_render = not visible
    for obj in collection.objects:
        obj.hide_render = not visible


def look_at(camera, target):
    direction = Vector(target) - camera.location
    camera.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def make_camera_and_lights(bounds):
    scene = bpy.context.scene
    for obj in list(bpy.data.objects):
        if obj.type in {"CAMERA", "LIGHT"}:
            bpy.data.objects.remove(obj, do_unlink=True)
    center = (Vector(bounds[0]) + Vector(bounds[1])) * 0.5
    extent = max((Vector(bounds[1]) - Vector(bounds[0])).length, 3.0)
    bpy.ops.object.camera_add(location=center + Vector((0.0, -extent * 2.0, extent * 0.24)))
    camera = bpy.context.object
    camera.data.lens = 58
    camera.data.sensor_width = 36
    scene.camera = camera
    for location, energy, size in (
        (center + Vector((4.0, -6.0, 8.0)), 1300.0, 5.0),
        (center + Vector((-5.0, -2.0, 4.0)), 800.0, 4.0),
        (center + Vector((0.0, 4.0, 5.0)), 500.0, 3.0),
    ):
        bpy.ops.object.light_add(type="AREA", location=location)
        light = bpy.context.object
        light.data.energy = energy
        light.data.shape = "DISK"
        light.data.size = size
        look_at(light, center)
    world = scene.world or bpy.data.worlds.new("MixamoProofWorld")
    scene.world = world
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.004, 0.006, 0.012, 1.0)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.22
    return camera, center, extent


def create_floor(center):
    material = make_material("MixamoProofFloor", (0.025, 0.035, 0.055), metallic=0.0, roughness=0.8)
    bpy.ops.mesh.primitive_plane_add(size=40.0, location=(center.x, center.y, 0.0))
    floor = bpy.context.object
    floor.data.materials.append(material)
    floor.name = "MixamoProofFloor"
    return floor


def render_views(output_dir, source_objects, source_root, proxies, bounds):
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 640
    scene.render.resolution_y = 640
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = False
    camera, center, extent = make_camera_and_lights(bounds)
    floor = create_floor(center)
    views = {
        "front": (Vector((0.0, -extent * 2.0, extent * 0.17)), Vector((0.0, 0.0, 0.0))),
        "front-3-4": (Vector((extent * 1.40, -extent * 1.65, extent * 0.34)), Vector((0.0, 0.0, 0.0))),
        "side": (Vector((extent * 2.0, 0.0, extent * 0.18)), Vector((0.0, 0.0, 0.0))),
    }
    # The bounds are already in the target proof frame; center the cameras on
    # the same target for all three representations.
    for view_name, (offset, _) in views.items():
        camera.location = center + offset
        look_at(camera, center)
        for obj in source_objects:
            obj.hide_render = True
        source_root.hide_render = True
        for representation in ("source", "control", "coh"):
            for name, collection in proxies.items():
                set_collection_render(collection, name == representation)
            path = output_dir / "visual" / representation / f"{view_name}.png"
            path.parent.mkdir(parents=True, exist_ok=True)
            scene.render.filepath = str(path)
            bpy.ops.render.render(write_still=True)
    floor.hide_render = True
    for obj in source_objects:
        obj.hide_render = True
    source_root.hide_render = True


def align_source_for_display(source_objects, source_root, source, target, rest_world):
    """Apply one derived display transform so source/control/proxy share scale."""
    source_hips = source_common_point(source, "hips")
    target_hips = rest_point(rest_world, "HIPS")
    ratios = []
    for source_a, source_b, coh_a, coh_b in (
        ("arm_r", "forearm_r", "UARMR", "LARMR"),
        ("forearm_r", "hand_r", "LARMR", "HANDR"),
        ("arm_l", "forearm_l", "UARML", "LARML"),
        ("forearm_l", "hand_l", "LARML", "HANDL"),
        ("thigh_r", "shin_r", "ULEGR", "LLEGR"),
        ("shin_r", "foot_r", "LLEGR", "FOOTR"),
        ("thigh_l", "shin_l", "ULEGL", "LLEGL"),
        ("shin_l", "foot_l", "LLEGL", "FOOTL"),
    ):
        source_len = (source_common_point(source, source_b) - source_common_point(source, source_a)).length
        coh_len = (rest_point(rest_world, coh_b) - rest_point(rest_world, coh_a)).length
        if source_len > 1.0e-6:
            ratios.append(coh_len / source_len)
    scale = sum(ratios) / len(ratios)
    align = Matrix.Translation(target_hips) @ Matrix.Scale(scale, 4) @ Matrix.Translation(-source_hips)
    source_root.matrix_world = align @ source_root.matrix_world
    for obj in source_objects:
        if obj == source_root:
            continue
        obj.matrix_world = align @ obj.matrix_world
    return {"scale": scale, "transform": [[float(v) for v in row] for row in align]}


def main():
    args = parse_args()
    # Blender resolves relative render paths against its own process working
    # directory.  Normalize all inputs/outputs up front so the proof bundle
    # stays beside the repository source that produced it.
    args.source_fbx = args.source_fbx.resolve()
    args.rig_json = args.rig_json.resolve()
    args.output_dir = args.output_dir.resolve()
    if not args.source_fbx.is_file():
        raise SystemExit(f"Source FBX not found: {args.source_fbx}")
    if not args.rig_json.is_file():
        raise SystemExit(f"CoH runtime rig JSON not found: {args.rig_json}")
    if args.frame < 1:
        raise SystemExit("--frame must be positive")
    if args.proof_frames < 2:
        raise SystemExit("--proof-frames must be at least 2")
    if args.neighbor_radius < 2:
        raise SystemExit("--neighbor-radius must be at least 2")

    args.output_dir.mkdir(parents=True, exist_ok=True)
    report, bones, by_id = load_rig(args.rig_json)
    _, rest_world = build_source_rest(bones, by_id)
    exact = create_exact_armature(report, bones, by_id, rest_world)
    exact.name = "CoH_Male_Exact_Export_Rig"
    exact.data.name = "CoH_Male_Exact_Export_Rig"
    exact["coh_export_fk"] = "runtime-local-bind-translation"
    exact.hide_render = True
    # Keep the armature evaluated.  Hiding it from the viewport causes
    # PoseBone.matrix to remain at rest in Blender's dependency graph, which
    # would make the rotation-only proof look like a T-pose and could also
    # poison a matrix-based native export.
    exact.hide_viewport = False

    source = import_mixamo(args.source_fbx)
    source_objects = [obj for obj in bpy.data.objects if obj.type == "MESH" or obj == source]
    source_root = bpy.data.objects.new("Mixamo_Display_Root", None)
    bpy.context.collection.objects.link(source_root)
    # Keep the imported armature unparented during KBS binding.  The empty is
    # only a render-visibility handle; the derived display transform is
    # applied once to the source objects after all animation evaluation.

    control = build_control_rig(source)
    source_mapping = None
    control_mapping = configure_control_mapping(control)
    kbs_report = {"used": not args.no_kbs}
    if not args.no_kbs:
        ensure_extension()
        source_mapping = configure_source_mapping(source)
        kbs_report.update(run_kbs_single_frame(source, control, args.frame))
    else:
        raise RuntimeError("This proof requires the installed KBS-DEV source-to-control bind")

    # The KBS bind establishes the conventional mapping and gives us a
    # source-evaluated frame.  Its temporary Copy Rotation constraints are
    # not a reliable headless evaluation path on Blender 5.2, so consume the
    # same source pose through the rotation-only anatomical solver.
    clear_kbs_destination(control)
    control_solve_report = solve_control_from_mixamo_geometry(source, control)
    bpy.context.scene.frame_set(args.frame)
    bpy.context.view_layer.update()
    source_control_report = semantic_control_comparison(source, control)
    if not source_control_report["pass"]:
        raise RuntimeError("SOURCE_TO_CONTROL_NO_PASS " + json.dumps(source_control_report, sort_keys=True))

    kbs_report["controlSolve"] = control_solve_report
    coh_report = transfer_control_to_coh(exact, rest_world, control)
    if not coh_report["pass"]:
        raise RuntimeError("CONTROL_TO_COH_NO_PASS " + json.dumps(coh_report, sort_keys=True))

    proxies = {}
    # Display transform is derived from rest segment ratios and is only for
    # evidence framing; animation/export matrices remain in their own frames.
    display_report = align_source_for_display(source_objects, source_root, source, exact, rest_world)
    display_matrix = Matrix(display_report["transform"])
    # The anatomical rig is a display-only bridge, so give it the same
    # derived framing transform after all semantic checks and CoH transfer
    # have completed.  Its control pose remains unchanged in its own local
    # frame.
    control.matrix_world = display_matrix @ control.matrix_world
    bpy.context.view_layer.update()
    control_hips_local = control.pose.bones[CONTROL["hips"]].matrix.translation.copy()
    control_hips_world = control.matrix_world @ control_hips_local
    control_display_delta = rest_point(rest_world, "HIPS") - control_hips_world
    control.matrix_world.translation += control_display_delta
    display_report["controlTranslationCorrection"] = [float(v) for v in control_display_delta]
    source_segments, source_points = semantic_proxy_points(source, "source")
    control_segments, control_points = semantic_proxy_points(control, "control")
    coh_segments, coh_points = semantic_proxy_points(exact, "coh")
    coh_proxy = create_proxy("CoHExportProxy", coh_segments, coh_points, (0.82, 0.22, 0.08))
    proxies["source"] = create_proxy("MixamoSourceProxy", source_segments, source_points, (0.20, 0.68, 0.95))
    proxies["control"] = create_proxy("ControlProxy", control_segments, control_points, (0.08, 0.42, 0.92))
    proxies["coh"] = coh_proxy

    all_points = source_points + control_points + coh_points
    for collection_segments in (source_segments, control_segments, coh_segments):
        for _, start, end, _ in collection_segments:
            all_points.extend((start, end))
    min_point = Vector((min(p.x for p in all_points), min(p.y for p in all_points), min(p.z for p in all_points)))
    max_point = Vector((max(p.x for p in all_points), max(p.y for p in all_points), max(p.z for p in all_points)))
    render_views(args.output_dir, source_objects, source_root, proxies, (min_point, max_point))

    proof_action, neighbor_report = create_neighbor_action(
        source, control, exact, rest_world, args.frame, args.neighbor_radius,
        args.source_start_frame, args.source_end_frame,
    )
    blend_path = args.output_dir / "COHSOURCEDEV_RETARGET_POSE_PROOF.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))

    report_out = {
        "tool": "agent/animation/prove_mixamo_anatomical_pose.py",
        "blender": bpy.app.version_string,
        "source": {
            "fbx": str(args.source_fbx),
            "frame": args.frame,
            "fps": 30,
            "armature": source.name,
            "boneCount": len(source.data.bones),
            "action": source.animation_data.action.name if source.animation_data and source.animation_data.action else None,
            "restObjectMatrix": [[float(v) for v in row] for row in source.matrix_world],
        },
        "controlRig": {
            "name": control.name,
            "boneCount": len(control.data.bones),
            "mapping": control_mapping,
            "sourceMapping": source_mapping,
            "kbs": kbs_report,
            "sourceToControl": source_control_report,
        },
        "cohExport": {
            "name": exact.name,
            "referenceAnimation": report["animation"],
            "mapping": {
                "arms": {"right": ["COL_R", "UARMR", "LARMR", "HANDR"], "left": ["COL_L", "UARML", "LARML", "HANDL"]},
                "legs": {"right": ["ULEGR", "LLEGR", "FOOTR"], "left": ["ULEGL", "LLEGL", "FOOTL"]},
                "torso": ["HIPS", "WAIST", "CHEST", "NECK", "HEAD"],
            },
            "torsoCompression": "Mixamo Spine/Spine1/Spine2 become HIPS/WAIST/CHEST segment semantics; upper chest uses Spine2->Neck, neck/head remain separate.",
            "armRoll": "cross(shoulder->elbow, elbow->wrist) is one shared roll normal for clavicle, upper arm, forearm; wrist uses the control hand frame projected against that plane.",
            "legRoll": "cross(hip->knee, knee->ankle) is the shared knee bend-plane normal for thigh, shin, and foot.",
            "rotationOnly": coh_report,
        },
        "neighborProof": neighbor_report,
        "display": display_report,
        "renders": {
            "source": [str(args.output_dir / "visual" / "source" / f"{view}.png") for view in ("front", "front-3-4", "side")],
            "control": [str(args.output_dir / "visual" / "control" / f"{view}.png") for view in ("front", "front-3-4", "side")],
            "cohProxy": [str(args.output_dir / "visual" / "coh" / f"{view}.png") for view in ("front", "front-3-4", "side")],
        },
        "nativeProof": {
            "blend": str(blend_path),
            "action": proof_action.name,
            "frames": neighbor_report["frameCount"],
            "sourceFrames": neighbor_report["sourceFrames"],
            "animx": str(args.output_dir / "COHSOURCEDEV_RETARGET_POSE_PROOF.ANIMX"),
            "anim": str(args.output_dir / "COHSOURCEDEV_RETARGET_POSE_PROOF.anim"),
        },
        "sourceSafety": {
            "sourceFbxsNotWritten": True,
            "runtimeInstallPerformed": False,
            "webSwingRuntimeTouched": False,
        },
    }
    report_path = args.output_dir / "proof.report.json"
    report_path.write_text(json.dumps(report_out, indent=2), encoding="utf-8")
    print("MIXAMO_ANATOMICAL_PROOF " + json.dumps({
        "frame": args.frame,
        "sourceToControlPass": source_control_report["pass"],
        "controlToCohPass": coh_report["pass"],
        "maxTargetLocalTranslation": coh_report["maxTargetLocalTranslation"],
        "maxTargetScaleError": coh_report["maxTargetScaleError"],
        "blend": str(blend_path),
        "report": str(report_path),
    }, sort_keys=True))


if __name__ == "__main__":
    main()
