"""Verify the installed full corrected Issue 36 Male swing runtime asset."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from compare_issue36_rest_basis_bottom_runtime import (
    bone_map,
    compare,
    load_json,
    position_error,
    quaternion_error_degrees,
    samples_by_frame,
)


ASSET = "MALE/COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL"
BOTTOM_ASSET = "MALE/COHSOURCEDEV_RETARGET_RESTBASIS_BOTTOM"
BOTTOM_SOURCE_FRAMES = tuple(range(18, 23))
EXPECTED_SOURCE_FRAMES = tuple(range(1, 61))
EXPECTED_BONE_COUNT = 68


def compare_full(math_report, runtime_report, stock_report, bottom_report,
                 angle_tolerance, position_tolerance):
    result = compare(
        math_report, runtime_report, stock_report,
        angle_tolerance, position_tolerance,
    )
    failed = result["failedChecks"]
    target = math_report["target"]
    expected_by_frame = {
        int(frame): values
        for frame, values in math_report["preExportProof"][
            "expectedRuntimeLocalQuaternionByAuthoredFrame"
        ].items()
    }

    if target.get("asset") != ASSET:
        failed.append({"kind": "full-asset-identity", "value": target.get("asset"), "expected": ASSET})
    if target.get("sourceFrameRange") != [1, 60]:
        failed.append({"kind": "source-frame-range", "value": target.get("sourceFrameRange"), "expected": [1, 60]})
    if target.get("authoredFrames") != 60:
        failed.append({"kind": "authored-sample-count", "value": target.get("authoredFrames"), "expected": 60})
    if runtime_report.get("length") != 60:
        failed.append({"kind": "runtime-length", "value": runtime_report.get("length"), "expected": 60})
    if runtime_report.get("maxSampleFrame") != 60:
        failed.append({"kind": "runtime-max-sample-frame", "value": runtime_report.get("maxSampleFrame"), "expected": 60})
    if runtime_report.get("boneCount") != EXPECTED_BONE_COUNT:
        failed.append({"kind": "full-runtime-bone-count", "value": runtime_report.get("boneCount"), "expected": EXPECTED_BONE_COUNT})
    if stock_report.get("boneCount") != EXPECTED_BONE_COUNT:
        failed.append({"kind": "stock-runtime-bone-count", "value": stock_report.get("boneCount"), "expected": EXPECTED_BONE_COUNT})
    if target.get("acceptedBottomSourceFrames") != list(BOTTOM_SOURCE_FRAMES):
        failed.append({"kind": "accepted-bottom-source-range", "value": target.get("acceptedBottomSourceFrames"), "expected": list(BOTTOM_SOURCE_FRAMES)})
    if bottom_report.get("animation") != BOTTOM_ASSET:
        failed.append({"kind": "bottom-runtime-identity", "value": bottom_report.get("animation"), "expected": BOTTOM_ASSET})

    full_bones = bone_map(runtime_report)
    bottom_bones = bone_map(bottom_report)
    stock_bones = bone_map(stock_report)
    expected_bone_names = set(stock_bones)
    if set(expected_by_frame) != set(EXPECTED_SOURCE_FRAMES):
        failed.append({
            "kind": "pre-export-frame-coverage",
            "missing": sorted(set(EXPECTED_SOURCE_FRAMES) - set(expected_by_frame)),
            "unexpected": sorted(set(expected_by_frame) - set(EXPECTED_SOURCE_FRAMES)),
        })
    for frame in sorted(expected_by_frame):
        frame_bones = set(expected_by_frame[frame])
        if frame_bones != expected_bone_names:
            failed.append({
                "kind": "pre-export-bone-coverage",
                "frame": frame,
                "missing": sorted(expected_bone_names - frame_bones),
                "unexpected": sorted(frame_bones - expected_bone_names),
            })
    if set(bottom_bones) != expected_bone_names:
        failed.append({
            "kind": "bottom-runtime-bone-coverage",
            "missing": sorted(expected_bone_names - set(bottom_bones)),
            "unexpected": sorted(set(bottom_bones) - expected_bone_names),
        })
    max_bottom_rotation_error = 0.0
    max_bottom_translation_error = 0.0
    max_bottom_bone = None
    max_bottom_source_frame = None
    for name in sorted(set(full_bones) & set(bottom_bones)):
        full_samples = samples_by_frame(full_bones[name])
        bottom_samples = samples_by_frame(bottom_bones[name])
        for bottom_frame, source_frame in enumerate(BOTTOM_SOURCE_FRAMES, start=1):
            full_sample = full_samples.get(source_frame)
            bottom_sample = bottom_samples.get(bottom_frame)
            if full_sample is None or bottom_sample is None:
                failed.append({
                    "kind": "bottom-correspondence-missing-sample",
                    "bone": name,
                    "sourceFrame": source_frame,
                    "bottomFrame": bottom_frame,
                })
                continue
            rotation_error = quaternion_error_degrees(
                full_sample["rotation"], bottom_sample["rotation"],
            )
            translation_error = position_error(
                full_sample["translation"], bottom_sample["translation"],
            )
            if rotation_error > max_bottom_rotation_error:
                max_bottom_rotation_error = rotation_error
                max_bottom_bone = name
                max_bottom_source_frame = source_frame
            max_bottom_translation_error = max(max_bottom_translation_error, translation_error)
            if rotation_error > angle_tolerance:
                failed.append({
                    "kind": "accepted-bottom-rotation-correspondence",
                    "bone": name,
                    "sourceFrame": source_frame,
                    "bottomFrame": bottom_frame,
                    "errorDegrees": rotation_error,
                })
            if translation_error > position_tolerance:
                failed.append({
                    "kind": "accepted-bottom-translation-correspondence",
                    "bone": name,
                    "sourceFrame": source_frame,
                    "bottomFrame": bottom_frame,
                    "error": translation_error,
                })

    changed_bones = [bone["bone"] for bone in result["bones"] if bone["authoredRotationChanged"]]
    if not changed_bones or result["maxAuthoredRotationDeltaDegrees"] <= 1.0e-5:
        failed.append({"kind": "static-full-clip", "changedBones": changed_bones})

    result.update({
        "passed": not (
            result["missingRuntimeBones"]
            or result["unexpectedRuntimeBones"]
            or result["hierarchyMismatches"]
            or failed
        ),
        "acceptedBottomRuntimeAsset": bottom_report.get("animation"),
        "acceptedBottomSourceFrames": list(BOTTOM_SOURCE_FRAMES),
        "maxAcceptedBottomRotationErrorDegrees": max_bottom_rotation_error,
        "maxAcceptedBottomTranslationError": max_bottom_translation_error,
        "maxAcceptedBottomErrorBone": max_bottom_bone,
        "maxAcceptedBottomErrorSourceFrame": max_bottom_source_frame,
        "animatedBoneCount": len(changed_bones),
        "animatedBones": changed_bones,
    })
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--math-report", required=True, type=Path)
    parser.add_argument("--runtime-report", required=True, type=Path)
    parser.add_argument("--stock-report", required=True, type=Path)
    parser.add_argument("--bottom-runtime-report", required=True, type=Path)
    parser.add_argument("--output-json", required=True, type=Path)
    parser.add_argument("--angle-tolerance-deg", type=float, default=0.1)
    parser.add_argument("--position-tolerance", type=float, default=0.00006)
    args = parser.parse_args()
    result = compare_full(
        load_json(args.math_report),
        load_json(args.runtime_report),
        load_json(args.stock_report),
        load_json(args.bottom_runtime_report),
        args.angle_tolerance_deg,
        args.position_tolerance,
    )
    args.output_json.parent.mkdir(parents=True, exist_ok=True)
    args.output_json.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print("ISSUE36_REST_BASIS_FULL_RUNTIME_PROOF " + json.dumps({
        "passed": result["passed"],
        "asset": result["asset"],
        "sourceFrameRange": result["sourceFrameRange"],
        "runtimeBoneCount": result["runtimeBoneCount"],
        "animatedBoneCount": result["animatedBoneCount"],
        "maxDecodedRuntimeLocalErrorDegrees": result["maxDecodedRuntimeLocalErrorDegrees"],
        "maxBindTranslationError": result["maxBindTranslationError"],
        "maxAuthoredTranslationDrift": result["maxAuthoredTranslationDrift"],
        "maxAcceptedBottomRotationErrorDegrees": result["maxAcceptedBottomRotationErrorDegrees"],
        "failedChecks": len(result["failedChecks"]),
    }, sort_keys=True))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
