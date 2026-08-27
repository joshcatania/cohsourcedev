"""Verify that the Issue 36 grip asset is a bounded finger-only delta."""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path


BASE_ASSET = "MALE/COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL"
GRIP_ASSET = "MALE/COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL_GRIP"
GRIP_BONES = ("F1_L", "F2_L", "T1_L", "T2_L", "T3_L")
GRIP_START = 16
GRIP_END = 30
EXPECTED_BONE_COUNT = 68


def load_json(path: Path):
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)


def vector_error(left, right):
    return math.sqrt(sum((float(a) - float(b)) ** 2 for a, b in zip(left, right)))


def quaternion_normalize(values):
    magnitude = math.sqrt(sum(float(value) ** 2 for value in values))
    if magnitude <= 1.0e-12:
        return [0.0, 0.0, 0.0, 1.0]
    return [float(value) / magnitude for value in values]


def quaternion_error_degrees(left, right):
    left = quaternion_normalize(left)
    right = quaternion_normalize(right)
    dot = abs(sum(a * b for a, b in zip(left, right)))
    dot = max(-1.0, min(1.0, dot))
    return math.degrees(2.0 * math.acos(dot))


def bone_map(report):
    return {bone["name"]: bone for bone in report.get("bones", [])}


def samples_by_frame(bone):
    return {int(sample["frame"]): sample for sample in bone.get("samples", [])}


def compare(base, grip, math_report, angle_tolerance, position_tolerance):
    failed = []
    base_bones = bone_map(base)
    grip_bones = bone_map(grip)
    missing = sorted(set(base_bones) - set(grip_bones))
    unexpected = sorted(set(grip_bones) - set(base_bones))
    if missing:
        failed.append({"kind": "missing-runtime-bones", "bones": missing})
    if unexpected:
        failed.append({"kind": "unexpected-runtime-bones", "bones": unexpected})

    if base.get("animation") != BASE_ASSET:
        failed.append({"kind": "base-animation-identity", "value": base.get("animation"), "expected": BASE_ASSET})
    if grip.get("animation") != GRIP_ASSET:
        failed.append({"kind": "grip-animation-identity", "value": grip.get("animation"), "expected": GRIP_ASSET})
    for report_name, report in (("base", base), ("grip", grip)):
        if report.get("length") != 60:
            failed.append({"kind": "runtime-length", "report": report_name, "value": report.get("length"), "expected": 60})
        if report.get("maxSampleFrame") != 60:
            failed.append({"kind": "runtime-max-sample-frame", "report": report_name, "value": report.get("maxSampleFrame"), "expected": 60})
        if report.get("boneCount") != EXPECTED_BONE_COUNT:
            failed.append({"kind": "runtime-bone-count", "report": report_name, "value": report.get("boneCount"), "expected": EXPECTED_BONE_COUNT})
        if report.get("baseAnimName", "").upper() != "MALE/SKEL_READY2":
            failed.append({"kind": "runtime-base-animation", "report": report_name, "value": report.get("baseAnimName"), "expected": "MALE/SKEL_READY2"})

    hierarchy_mismatches = []
    for name in sorted(set(base_bones) & set(grip_bones)):
        for field in ("id", "parent", "child", "sibling"):
            if base_bones[name].get(field) != grip_bones[name].get(field):
                hierarchy_mismatches.append({
                    "bone": name,
                    "field": field,
                    "base": base_bones[name].get(field),
                    "grip": grip_bones[name].get(field),
                })
    if hierarchy_mismatches:
        failed.extend({"kind": "hierarchy", **row} for row in hierarchy_mismatches)

    max_rotation_delta = 0.0
    max_rotation_delta_bone = None
    max_rotation_delta_frame = None
    max_translation_delta = 0.0
    max_translation_delta_bone = None
    max_translation_delta_frame = None
    changed_bones = set()
    changed_frames = set()
    changed_rows = []

    for name in sorted(set(base_bones) & set(grip_bones)):
        base_bone = base_bones[name]
        grip_bone = grip_bones[name]
        bind_delta = vector_error(
            base_bone["frame0LocalTranslation"], grip_bone["frame0LocalTranslation"],
        )
        max_translation_delta = max(max_translation_delta, bind_delta)
        if bind_delta > position_tolerance:
            failed.append({"kind": "bind-translation", "bone": name, "error": bind_delta})

        base_samples = samples_by_frame(base_bone)
        grip_samples = samples_by_frame(grip_bone)
        for frame in sorted(set(base_samples) | set(grip_samples)):
            left = base_samples.get(frame)
            right = grip_samples.get(frame)
            if left is None or right is None:
                failed.append({"kind": "sample-coverage", "bone": name, "frame": frame})
                continue
            translation_delta = vector_error(left["translation"], right["translation"])
            if translation_delta > max_translation_delta:
                max_translation_delta = translation_delta
                max_translation_delta_bone = name
                max_translation_delta_frame = frame
            if translation_delta > position_tolerance:
                failed.append({
                    "kind": "translation-changed",
                    "bone": name,
                    "frame": frame,
                    "error": translation_delta,
                })
            rotation_delta = quaternion_error_degrees(left["rotation"], right["rotation"])
            if rotation_delta > max_rotation_delta:
                max_rotation_delta = rotation_delta
                max_rotation_delta_bone = name
                max_rotation_delta_frame = frame
            if rotation_delta > 1.0e-5:
                changed_bones.add(name)
                changed_frames.add(frame)
                changed_rows.append({"bone": name, "frame": frame, "deltaDegrees": rotation_delta})
                if name not in GRIP_BONES:
                    failed.append({"kind": "non-grip-rotation-changed", "bone": name, "frame": frame, "deltaDegrees": rotation_delta})
                elif frame < GRIP_START or frame > GRIP_END:
                    failed.append({"kind": "out-of-window-rotation-changed", "bone": name, "frame": frame, "deltaDegrees": rotation_delta})
            elif name not in GRIP_BONES and rotation_delta > angle_tolerance:
                failed.append({"kind": "non-grip-rotation-tolerance", "bone": name, "frame": frame, "deltaDegrees": rotation_delta})

    math_target = math_report.get("target", {})
    modification = math_report.get("modification", {})
    proof = math_report.get("proof", {})
    if math_target.get("asset") != GRIP_ASSET:
        failed.append({"kind": "math-asset-identity", "value": math_target.get("asset"), "expected": GRIP_ASSET})
    if math_target.get("authoredFrameRange") != [1, 60] or math_target.get("authoredFrames") != 60:
        failed.append({"kind": "math-authored-range", "value": math_target.get("authoredFrameRange"), "frames": math_target.get("authoredFrames")})
    if modification.get("bones") != list(GRIP_BONES):
        failed.append({"kind": "math-grip-bones", "value": modification.get("bones"), "expected": list(GRIP_BONES)})
    if modification.get("authoredFrameRange") != [GRIP_START, GRIP_END]:
        failed.append({"kind": "math-grip-range", "value": modification.get("authoredFrameRange"), "expected": [GRIP_START, GRIP_END]})
    if not proof.get("pass") or not proof.get("nonGripTransformsUnchanged"):
        failed.append({"kind": "blender-proof", "value": proof})

    if not changed_bones:
        failed.append({"kind": "no-grip-delta"})
    if not changed_bones.issubset(set(GRIP_BONES)):
        failed.append({"kind": "changed-bone-scope", "bones": sorted(changed_bones)})
    if not changed_frames.issubset(set(range(GRIP_START, GRIP_END + 1))):
        failed.append({"kind": "changed-frame-scope", "frames": sorted(changed_frames)})

    return {
        "passed": not failed,
        "baseAsset": base.get("animation"),
        "gripAsset": grip.get("animation"),
        "runtimeLength": grip.get("length"),
        "runtimeBoneCount": grip.get("boneCount"),
        "allowedBones": list(GRIP_BONES),
        "allowedFrameRange": [GRIP_START, GRIP_END],
        "changedBones": sorted(changed_bones),
        "changedFrames": sorted(changed_frames),
        "changedRows": changed_rows,
        "maxRotationDeltaDegrees": max_rotation_delta,
        "maxRotationDeltaBone": max_rotation_delta_bone,
        "maxRotationDeltaFrame": max_rotation_delta_frame,
        "maxTranslationDelta": max_translation_delta,
        "maxTranslationDeltaBone": max_translation_delta_bone,
        "maxTranslationDeltaFrame": max_translation_delta_frame,
        "hierarchyMismatches": hierarchy_mismatches,
        "failedChecks": failed,
        "angleToleranceDegrees": angle_tolerance,
        "positionTolerance": position_tolerance,
        "nonGripRotationsUnchangedWithinScope": not any(
            row.get("kind") == "non-grip-rotation-changed" for row in failed
        ),
        "translationsUnchanged": not any(
            row.get("kind") in {"bind-translation", "translation-changed"}
            for row in failed
        ),
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--base-runtime-report", required=True, type=Path)
    parser.add_argument("--grip-runtime-report", required=True, type=Path)
    parser.add_argument("--math-report", required=True, type=Path)
    parser.add_argument("--output-json", required=True, type=Path)
    parser.add_argument("--angle-tolerance-deg", type=float, default=0.01)
    parser.add_argument("--position-tolerance", type=float, default=0.00006)
    args = parser.parse_args()
    result = compare(
        load_json(args.base_runtime_report),
        load_json(args.grip_runtime_report),
        load_json(args.math_report),
        args.angle_tolerance_deg,
        args.position_tolerance,
    )
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    args.output_json.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print("ISSUE36_REST_BASIS_GRIP_RUNTIME_PROOF " + json.dumps({
        "passed": result["passed"],
        "baseAsset": result["baseAsset"],
        "gripAsset": result["gripAsset"],
        "changedBones": result["changedBones"],
        "changedFrames": result["changedFrames"],
        "maxRotationDeltaDegrees": result["maxRotationDeltaDegrees"],
        "maxTranslationDelta": result["maxTranslationDelta"],
        "failedChecks": len(result["failedChecks"]),
    }, sort_keys=True))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
