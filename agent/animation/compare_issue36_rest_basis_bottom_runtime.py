"""Verify the corrected Issue 36 BOTTOM asset after runtime packing.

The Blender proof records the expected CoH runtime-local quaternion for every
Male bone at authored samples 1..5.  This checker compares those samples with
the decoded GetAnimation2 report, while also checking the stock 68-bone
hierarchy and fixed bind translations.  Samples after 5 can exist because the
runtime report resolves the base ``SKEL_READY2`` length; only the five authored
samples are the bounded BOTTOM asset under test.
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path


def load_json(path: Path):
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)


def vector_length(values):
    return math.sqrt(sum(float(value) * float(value) for value in values))


def position_error(left, right):
    return vector_length([float(a) - float(b) for a, b in zip(left, right)])


def quaternion_normalize(values):
    magnitude = vector_length(values)
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
    return {bone["name"]: bone for bone in report["bones"]}


def samples_by_frame(bone):
    return {int(sample["frame"]): sample for sample in bone["samples"]}


def compare(math_report, runtime_report, stock_report, angle_tolerance, position_tolerance):
    runtime_bones = bone_map(runtime_report)
    stock_bones = bone_map(stock_report)
    target = math_report["target"]
    pre_export = math_report["preExportProof"]
    expected_by_frame = {
        int(frame): values
        for frame, values in pre_export["expectedRuntimeLocalQuaternionByAuthoredFrame"].items()
    }
    authored_frames = sorted(expected_by_frame)

    missing_runtime = sorted(set(stock_bones) - set(runtime_bones))
    unexpected_runtime = sorted(set(runtime_bones) - set(stock_bones))
    hierarchy_mismatches = []
    for name in sorted(set(stock_bones) & set(runtime_bones)):
        for field in ("id", "parent", "child", "sibling"):
            if runtime_bones[name].get(field) != stock_bones[name].get(field):
                hierarchy_mismatches.append({
                    "bone": name,
                    "field": field,
                    "runtime": runtime_bones[name].get(field),
                    "stock": stock_bones[name].get(field),
                })

    max_bind_translation_error = 0.0
    max_authored_translation_drift = 0.0
    max_local_rotation_error = 0.0
    max_local_error_bone = None
    max_local_error_frame = None
    max_authored_rotation_delta = 0.0
    max_authored_rotation_delta_bone = None
    max_authored_rotation_delta_pair = None
    failed_checks = []
    per_bone = []

    for name, runtime_bone in sorted(runtime_bones.items()):
        stock_bone = stock_bones.get(name)
        if stock_bone is None:
            continue
        runtime_samples = samples_by_frame(runtime_bone)
        stock_frame0 = stock_bone["frame0LocalTranslation"]
        bind_error = position_error(runtime_bone["frame0LocalTranslation"], stock_frame0)
        max_bind_translation_error = max(max_bind_translation_error, bind_error)
        if bind_error > position_tolerance:
            failed_checks.append({
                "kind": "bind-translation",
                "bone": name,
                "error": bind_error,
            })

        expected_bone = {
            frame: expected_by_frame[frame].get(name)
            for frame in authored_frames
        }
        bone_local_error = 0.0
        bone_translation_drift = 0.0
        missing_frames = []
        for frame in authored_frames:
            sample = runtime_samples.get(frame)
            expected = expected_bone[frame]
            if sample is None:
                missing_frames.append(frame)
                failed_checks.append({"kind": "missing-authored-sample", "bone": name, "frame": frame})
                continue
            translation_drift = position_error(sample["translation"], stock_frame0)
            bone_translation_drift = max(bone_translation_drift, translation_drift)
            max_authored_translation_drift = max(max_authored_translation_drift, translation_drift)
            if translation_drift > position_tolerance:
                failed_checks.append({
                    "kind": "authored-bind-translation-drift",
                    "bone": name,
                    "frame": frame,
                    "error": translation_drift,
                })
            if expected is not None:
                local_error = quaternion_error_degrees(sample["rotation"], expected)
                bone_local_error = max(bone_local_error, local_error)
                if local_error > max_local_rotation_error:
                    max_local_rotation_error = local_error
                    max_local_error_bone = name
                    max_local_error_frame = frame
                if local_error > angle_tolerance:
                    failed_checks.append({
                        "kind": "decoded-runtime-local-rotation",
                        "bone": name,
                        "frame": frame,
                        "errorDegrees": local_error,
                    })

        for left, right in zip(authored_frames, authored_frames[1:]):
            left_sample = runtime_samples.get(left)
            right_sample = runtime_samples.get(right)
            if left_sample is None or right_sample is None:
                continue
            delta = quaternion_error_degrees(left_sample["rotation"], right_sample["rotation"])
            if delta > max_authored_rotation_delta:
                max_authored_rotation_delta = delta
                max_authored_rotation_delta_bone = name
                max_authored_rotation_delta_pair = [left, right]

        per_bone.append({
            "bone": name,
            "maxDecodedLocalErrorDegrees": bone_local_error,
            "maxAuthoredTranslationDrift": bone_translation_drift,
            "missingAuthoredFrames": missing_frames,
            "authoredRotationChanged": any(
                quaternion_error_degrees(
                    runtime_samples[left]["rotation"], runtime_samples[right]["rotation"]
                ) > 1.0e-5
                for left, right in zip(authored_frames, authored_frames[1:])
                if left in runtime_samples and right in runtime_samples
            ),
        })

    expected_asset = target["asset"]
    identity_ok = runtime_report.get("animation") == expected_asset
    if not identity_ok:
        failed_checks.append({
            "kind": "animation-identity",
            "runtime": runtime_report.get("animation"),
            "expected": expected_asset,
        })
    if runtime_report.get("boneCount") != stock_report.get("boneCount"):
        failed_checks.append({
            "kind": "bone-count",
            "runtime": runtime_report.get("boneCount"),
            "stock": stock_report.get("boneCount"),
        })
    if runtime_report.get("baseAnimName", "").upper() != "MALE/SKEL_READY2":
        failed_checks.append({
            "kind": "base-animation",
            "runtime": runtime_report.get("baseAnimName"),
            "expected": "MALE/SKEL_READY2",
        })
    if not pre_export.get("pass"):
        failed_checks.append({"kind": "pre-export-proof", "value": pre_export.get("pass")})
    for frame in authored_frames:
        if frame not in [item["authoredFrame"] for item in target["authoredFrameMap"]]:
            failed_checks.append({"kind": "authored-frame-map", "frame": frame})

    passed = not (
        missing_runtime
        or unexpected_runtime
        or hierarchy_mismatches
        or failed_checks
    )
    return {
        "passed": passed,
        "asset": expected_asset,
        "runtimeAnimation": runtime_report.get("animation"),
        "baseAnimation": runtime_report.get("baseAnimName"),
        "runtimeBoneCount": runtime_report.get("boneCount"),
        "runtimeLength": runtime_report.get("length"),
        "runtimeMaxSampleFrame": runtime_report.get("maxSampleFrame"),
        "sourceFrameRange": target.get("sourceFrameRange"),
        "authoredFrameMap": target.get("authoredFrameMap"),
        "authoredRuntimeFrames": authored_frames,
        "missingRuntimeBones": missing_runtime,
        "unexpectedRuntimeBones": unexpected_runtime,
        "hierarchyMismatches": hierarchy_mismatches,
        "failedChecks": failed_checks,
        "angleToleranceDegrees": angle_tolerance,
        "positionTolerance": position_tolerance,
        "maxBindTranslationError": max_bind_translation_error,
        "maxAuthoredTranslationDrift": max_authored_translation_drift,
        "maxDecodedRuntimeLocalErrorDegrees": max_local_rotation_error,
        "maxDecodedRuntimeLocalErrorBone": max_local_error_bone,
        "maxDecodedRuntimeLocalErrorFrame": max_local_error_frame,
        "maxAuthoredRotationDeltaDegrees": max_authored_rotation_delta,
        "maxAuthoredRotationDeltaBone": max_authored_rotation_delta_bone,
        "maxAuthoredRotationDeltaFramePair": max_authored_rotation_delta_pair,
        "preExportMaxFullBasisDeltaDegrees": pre_export.get("maxFullBasisDeltaDegrees"),
        "preExportMaxRuntimeWorldQuaternionErrorDegrees": pre_export.get(
            "maxRuntimeWorldQuaternionErrorDegrees"
        ),
        "bones": per_bone,
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--math-report", required=True, type=Path)
    parser.add_argument("--runtime-report", required=True, type=Path)
    parser.add_argument("--stock-report", required=True, type=Path)
    parser.add_argument("--output-json", required=True, type=Path)
    parser.add_argument("--angle-tolerance-deg", type=float, default=0.1)
    parser.add_argument("--position-tolerance", type=float, default=0.00006)
    args = parser.parse_args()

    result = compare(
        load_json(args.math_report),
        load_json(args.runtime_report),
        load_json(args.stock_report),
        args.angle_tolerance_deg,
        args.position_tolerance,
    )
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    args.output_json.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print("ISSUE36_REST_BASIS_BOTTOM_RUNTIME_PROOF " + json.dumps({
        "passed": result["passed"],
        "asset": result["asset"],
        "sourceFrameRange": result["sourceFrameRange"],
        "runtimeBoneCount": result["runtimeBoneCount"],
        "maxDecodedRuntimeLocalErrorDegrees": result["maxDecodedRuntimeLocalErrorDegrees"],
        "maxBindTranslationError": result["maxBindTranslationError"],
        "maxAuthoredTranslationDrift": result["maxAuthoredTranslationDrift"],
        "failedChecks": len(result["failedChecks"]),
    }, sort_keys=True))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
