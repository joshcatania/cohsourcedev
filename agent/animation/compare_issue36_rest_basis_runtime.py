"""Verify the Issue 36 frame-20 rest-basis asset after runtime packing.

The proof blend is checked before export by
``create_issue36_rest_basis_frame20.py``.  This bounded checker performs the
next boundary: it compares the decoded GetAnimation2 local channels with the
pre-export runtime-local channels, confirms that all 68 bones retain the stock
hierarchy and bind translations, and confirms that the authored pose is static
through every decoded sample.
"""

from __future__ import annotations

import argparse
import json
import math
import sys
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
    return {sample["frame"]: sample for sample in bone["samples"]}


def compare(math_report, runtime_report, stock_report, angle_tolerance, position_tolerance):
    runtime_bones = bone_map(runtime_report)
    stock_bones = bone_map(stock_report)
    focused = list(math_report["target"]["focusedBones"])
    diagnostic_apply = math_report["diagnosticApply"]
    pre_export = math_report["preExportProof"]

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
    max_static_translation_drift = 0.0
    max_static_rotation_drift = 0.0
    max_local_rotation_error = 0.0
    max_local_error_bone = None
    max_local_error_frame = None
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

        if 1 not in runtime_samples:
            failed_checks.append({"kind": "missing-authored-sample", "bone": name, "frame": 1})
            continue
        first = runtime_samples[1]
        bone_max_rotation_drift = 0.0
        bone_max_translation_drift = 0.0
        for frame, sample in sorted(runtime_samples.items()):
            if frame < 1:
                continue
            rotation_drift = quaternion_error_degrees(sample["rotation"], first["rotation"])
            translation_drift = position_error(sample["translation"], first["translation"])
            bone_max_rotation_drift = max(bone_max_rotation_drift, rotation_drift)
            bone_max_translation_drift = max(bone_max_translation_drift, translation_drift)
            max_static_rotation_drift = max(max_static_rotation_drift, rotation_drift)
            max_static_translation_drift = max(max_static_translation_drift, translation_drift)
            if rotation_drift > angle_tolerance:
                failed_checks.append({
                    "kind": "non-static-rotation",
                    "bone": name,
                    "frame": frame,
                    "errorDegrees": rotation_drift,
                })
            if translation_drift > position_tolerance:
                failed_checks.append({
                    "kind": "non-static-translation",
                    "bone": name,
                    "frame": frame,
                    "error": translation_drift,
                })

        if name in diagnostic_apply:
            expected_local = diagnostic_apply[name]["runtimeLocalQuaternion"]
            for frame, sample in sorted(runtime_samples.items()):
                if frame < 1:
                    continue
                local_error = quaternion_error_degrees(sample["rotation"], expected_local)
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
            per_bone.append({
                "bone": name,
                "expectedRuntimeLocalQuaternion": expected_local,
                "decodedFrame1Quaternion": first["rotation"],
                "maxDecodedLocalErrorDegrees": max(
                    quaternion_error_degrees(sample["rotation"], expected_local)
                    for frame, sample in runtime_samples.items()
                    if frame >= 1
                ),
                "maxStaticRotationDriftDegrees": bone_max_rotation_drift,
                "maxStaticTranslationDrift": bone_max_translation_drift,
                "preExportDirectionDeltaDegrees": next(
                    row["directionDeltaDegrees"]
                    for row in pre_export["bones"]
                    if row["bone"] == name
                ),
                "preExportRollDeltaDegrees": next(
                    row["rollDeltaDegrees"]
                    for row in pre_export["bones"]
                    if row["bone"] == name
                ),
                "preExportFullBasisDeltaDegrees": next(
                    row["fullBasisDeltaDegrees"]
                    for row in pre_export["bones"]
                    if row["bone"] == name
                ),
            })

    expected_asset = math_report["target"]["asset"]
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
    if runtime_report.get("baseAnimName") != stock_report.get("baseAnimName"):
        failed_checks.append({
            "kind": "base-animation",
            "runtime": runtime_report.get("baseAnimName"),
            "stock": stock_report.get("baseAnimName"),
        })

    if not pre_export.get("pass"):
        failed_checks.append({"kind": "pre-export-proof", "value": pre_export.get("pass")})

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
        "focusedBones": focused,
        "missingRuntimeBones": missing_runtime,
        "unexpectedRuntimeBones": unexpected_runtime,
        "hierarchyMismatches": hierarchy_mismatches,
        "failedChecks": failed_checks,
        "angleToleranceDegrees": angle_tolerance,
        "positionTolerance": position_tolerance,
        "maxBindTranslationError": max_bind_translation_error,
        "maxStaticTranslationDrift": max_static_translation_drift,
        "maxStaticRotationDriftDegrees": max_static_rotation_drift,
        "maxDecodedRuntimeLocalErrorDegrees": max_local_rotation_error,
        "maxDecodedRuntimeLocalErrorBone": max_local_error_bone,
        "maxDecodedRuntimeLocalErrorFrame": max_local_error_frame,
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
    print("ISSUE36_REST_BASIS_RUNTIME_PROOF " + json.dumps({
        "passed": result["passed"],
        "asset": result["asset"],
        "runtimeBoneCount": result["runtimeBoneCount"],
        "maxDecodedRuntimeLocalErrorDegrees": result["maxDecodedRuntimeLocalErrorDegrees"],
        "maxStaticRotationDriftDegrees": result["maxStaticRotationDriftDegrees"],
        "maxBindTranslationError": result["maxBindTranslationError"],
        "failedChecks": len(result["failedChecks"]),
    }, sort_keys=True))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
