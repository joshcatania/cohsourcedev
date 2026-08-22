"""Compare two GetAnimation2 runtime inspection reports.

Quaternion signs are equivalent, so rotation error is measured by the shortest
angular distance between normalized quaternions.  The report is deliberately
small enough to keep the round-trip check reviewable in CI or a local log.
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path


def length(values):
    return math.sqrt(sum(value * value for value in values))


def normalize(values):
    magnitude = length(values)
    if magnitude <= 1.0e-12:
        return [0.0, 0.0, 0.0, 1.0]
    return [value / magnitude for value in values]


def quaternion_error_degrees(left, right):
    left = normalize(left)
    right = normalize(right)
    dot = abs(sum(a * b for a, b in zip(left, right)))
    dot = max(-1.0, min(1.0, dot))
    return math.degrees(2.0 * math.acos(dot))


def position_error(left, right):
    return length([a - b for a, b in zip(left, right)])


def load_report(path):
    with Path(path).open("r", encoding="utf-8") as stream:
        return json.load(stream)


def compare(left, right, angle_tolerance, position_tolerance):
    left_bones = {bone["name"]: bone for bone in left["bones"]}
    right_bones = {bone["name"]: bone for bone in right["bones"]}
    names = sorted(set(left_bones) | set(right_bones))
    missing_left = sorted(set(right_bones) - set(left_bones))
    missing_right = sorted(set(left_bones) - set(right_bones))
    hierarchy_mismatches = []
    max_angle = 0.0
    max_position = 0.0
    max_angle_sample = None
    max_position_sample = None
    failed_samples = 0

    for name in names:
        left_bone = left_bones.get(name)
        right_bone = right_bones.get(name)
        if not left_bone or not right_bone:
            continue
        for field in ("id", "parent", "child", "sibling"):
            if left_bone.get(field) != right_bone.get(field):
                hierarchy_mismatches.append(
                    {"bone": name, "field": field,
                     "left": left_bone.get(field), "right": right_bone.get(field)}
                )
        left_samples = {sample["frame"]: sample for sample in left_bone["samples"]}
        right_samples = {sample["frame"]: sample for sample in right_bone["samples"]}
        for frame in sorted(set(left_samples) | set(right_samples)):
            left_sample = left_samples.get(frame)
            right_sample = right_samples.get(frame)
            if not left_sample or not right_sample:
                failed_samples += 1
                continue
            angle = quaternion_error_degrees(left_sample["rotation"], right_sample["rotation"])
            position = position_error(left_sample["translation"], right_sample["translation"])
            if angle > max_angle:
                max_angle = angle
                max_angle_sample = {"bone": name, "frame": frame}
            if position > max_position:
                max_position = position
                max_position_sample = {"bone": name, "frame": frame}
            if angle > angle_tolerance or position > position_tolerance:
                failed_samples += 1

    return {
        "passed": not missing_left and not missing_right and not hierarchy_mismatches and failed_samples == 0,
        "left": left.get("animation"),
        "right": right.get("animation"),
        "leftBoneCount": left.get("boneCount"),
        "rightBoneCount": right.get("boneCount"),
        "leftMaxSampleFrame": left.get("maxSampleFrame"),
        "rightMaxSampleFrame": right.get("maxSampleFrame"),
        "missingFromLeft": missing_left,
        "missingFromRight": missing_right,
        "hierarchyMismatches": hierarchy_mismatches,
        "failedSamples": failed_samples,
        "maxRotationErrorDegrees": max_angle,
        "maxRotationErrorSample": max_angle_sample,
        "maxPositionError": max_position,
        "maxPositionErrorSample": max_position_sample,
        "rotationToleranceDegrees": angle_tolerance,
        "positionTolerance": position_tolerance,
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("left")
    parser.add_argument("right")
    # Runtime rotations use the shipped 12-bit nonlinear quaternion packing.
    # Its observed stock round-trip bound is below 0.1 degrees.  Compressed
    # positions use 1/32000 units per component, whose Euclidean worst case is
    # sqrt(3)/32000 ~= 0.0000541; 0.00006 leaves only a small float/format
    # margin without hiding transform drift.
    parser.add_argument("--angle-tolerance-deg", type=float, default=0.1)
    parser.add_argument("--position-tolerance", type=float, default=0.00006)
    args = parser.parse_args()
    result = compare(load_report(args.left), load_report(args.right),
                     args.angle_tolerance_deg, args.position_tolerance)
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
