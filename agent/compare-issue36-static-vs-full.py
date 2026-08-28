"""Issue 36 forensic comparison: static proof runtime asset vs full-clip asset.

Both inputs are GetAnimation2 -runtime-rig JSON reports of the compiled
runtime assets.  The comparison uses local transforms only (the reports store
per-bone local rotation/translation samples), the same convention the engine's
FK evaluates.

Tolerances are the already-proven stock round-trip bounds:
  rotation <= 0.1 degree, position <= 0.00006.
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from pathlib import Path

FOCUS_BONES = {
    "HIPS", "WAIST", "CHEST", "NECK", "HEAD", "CRANIUM",
    "COL_R", "UARMR", "LARMR", "HANDR",
    "COL_L", "UARML", "LARML", "HANDL",
}

# Runtime .anim tracks store the bind/reference pose in sample/frame 0; authored
# animation data starts at frame 1.  The static proof asset therefore holds its
# Mixamo source-frame-30 pose constantly on samples 1..N (verified: every
# sample 1..60 is identical), and sample 0 is the identity bind pose.  All
# proof comparisons below intentionally use PROOF_POSE_SAMPLE, never sample 0.
PROOF_POSE_SAMPLE = 1


def quaternion_error_degrees(left, right):
    def norm(values):
        magnitude = math.sqrt(sum(v * v for v in values))
        if magnitude <= 1.0e-12:
            return [0.0, 0.0, 0.0, 1.0]
        return [v / magnitude for v in values]

    left = norm(left)
    right = norm(right)
    dot = abs(sum(a * b for a, b in zip(left, right)))
    dot = max(-1.0, min(1.0, dot))
    return math.degrees(2.0 * math.acos(dot))


def position_error(left, right):
    return math.sqrt(sum((a - b) ** 2 for a, b in zip(left, right)))


def load_report(path):
    with Path(path).open("r", encoding="utf-8") as stream:
        return json.load(stream)


def bones_by_name(report):
    return {bone["name"]: bone for bone in report["bones"]}


def sample_at(bone, frame):
    for sample in bone["samples"]:
        if sample["frame"] == frame:
            return sample
    return None


def fmt_vec(values):
    return "(" + ", ".join(f"{v:.6g}" for v in values) + ")"


def parent_name(report, bone):
    parent = bone["parent"]
    if 0 <= parent < len(report["bones"]):
        return report["bones"][parent]["name"]
    return "NONE"


def asset_summary(label, report):
    lines = [
        f"### {label}",
        "",
        f"- base animation name: `{report.get('baseAnimName')}`",
        f"- bone count: {report.get('boneCount')}",
        f"- maxSampleFrame: {report.get('maxSampleFrame')} (length {report.get('length')})",
        "",
        "| bone | parent | rot keys | pos keys | trackSource |",
        "| --- | --- | ---: | ---: | --- |",
    ]
    for bone in report["bones"]:
        lines.append(
            f"| {bone['name']} | {parent_name(report, bone)} | "
            f"{bone['rotationTrackCount']} | {bone['positionTrackCount']} | "
            f"{bone['trackSource']} |"
        )
    lines.append("")
    return "\n".join(lines)


def frame_errors(proof_bone, full_bone, frame):
    proof_sample = sample_at(proof_bone, PROOF_POSE_SAMPLE)
    full_sample = sample_at(full_bone, frame)
    if not proof_sample or not full_sample:
        return None
    return (
        quaternion_error_degrees(full_sample["rotation"], proof_sample["rotation"]),
        position_error(full_sample["translation"], proof_sample["translation"]),
    )


def find_matching_frame(proof, full, candidate_frames):
    proof_bones = bones_by_name(proof)
    full_bones = bones_by_name(full)
    common = sorted(set(proof_bones) & set(full_bones))
    rows = []
    best = None
    for frame in candidate_frames:
        worst_angle = -1.0
        worst_position = -1.0
        worst_bone = None
        missing = 0
        for name in common:
            errors = frame_errors(proof_bones[name], full_bones[name], frame)
            if errors is None:
                missing += 1
                continue
            angle, position = errors
            if angle > worst_angle:
                worst_angle, worst_position, worst_bone = angle, position, name
            elif angle == worst_angle and position > worst_position:
                worst_position, worst_bone = position, name
        rows.append((frame, max(worst_angle, 0.0), max(worst_position, 0.0),
                     worst_bone or "-", missing))
        if best is None or worst_angle < rows[best][1]:
            best = len(rows) - 1
    return rows, (rows[best][0] if best is not None else None)


def compare_frame(proof, full, frame, angle_tolerance, position_tolerance):
    proof_bones = bones_by_name(proof)
    full_bones = bones_by_name(full)
    hierarchy_order = [bone["name"] for bone in proof["bones"]]
    md = []
    first_divergence = None
    max_angle = 0.0
    max_position = 0.0
    max_angle_bone = None
    max_position_bone = None
    failures = []

    md.append("| bone | parent match | full local translation | full local quat | pos error | rot error (deg) | verdict |")
    md.append("| --- | --- | --- | --- | ---: | ---: | --- |")
    for name in hierarchy_order:
        proof_bone = proof_bones.get(name)
        full_bone = full_bones.get(name)
        focus = "*" if name in FOCUS_BONES else ""
        if not proof_bone or not full_bone:
            md.append(f"| {name}{focus} | MISSING | - | - | - | - | FAIL |")
            if first_divergence is None:
                first_divergence = f"{name}: missing bone"
            failures.append(name)
            continue

        parent_ok = proof_bone["parent"] == full_bone["parent"]
        errors = frame_errors(proof_bone, full_bone, frame)
        if errors is None:
            md.append(f"| {name}{focus} | {'yes' if parent_ok else 'NO'} | NO_SAMPLE | - | - | - | FAIL |")
            if first_divergence is None:
                first_divergence = f"{name}: missing sample at frame {frame}"
            failures.append(name)
            continue

        angle, position = errors
        translation_ok = position <= position_tolerance
        rotation_ok = angle <= angle_tolerance
        ok = parent_ok and translation_ok and rotation_ok
        if first_divergence is None and not ok:
            if not parent_ok:
                reason = (f"parent differs: proof={proof_bone['parent']} "
                          f"({parent_name(proof, proof_bone)}) vs full={full_bone['parent']} "
                          f"({parent_name(full, full_bone)})")
            elif not translation_ok:
                reason = f"local translation differs by {position:.9g}"
            else:
                reason = f"local rotation differs by {angle:.6f} deg"
            first_divergence = f"{name}: {reason}"

        if angle > max_angle:
            max_angle, max_angle_bone = angle, name
        if position > max_position:
            max_position, max_position_bone = position, name
        if not ok:
            failures.append(name)

        full_sample = sample_at(full_bone, frame)
        md.append(
            f"| {name}{focus} | {'yes' if parent_ok else '**NO**'} | "
            f"{fmt_vec(full_sample['translation'])} | {fmt_vec(full_sample['rotation'])} | "
            f"{position:.9g} | {angle:.6f} | {'OK' if ok else '**FAIL**'} |"
        )
    md.append("")

    return {
        "markdown": "\n".join(md),
        "first_divergence": first_divergence,
        "max_rotation_error_degrees": max_angle,
        "max_rotation_bone": max_angle_bone,
        "max_position_error": max_position,
        "max_position_bone": max_position_bone,
        "failures": failures,
    }


def verify_constant_translations(label, report, stock_report, position_tolerance):
    """Every bone's position track must be constant and equal the stock bind
    translation.  Authored samples start at frame 1; frame 0 is the bind
    reference, so the authored range 1..N is what must be constant."""
    stock_bones = bones_by_name(stock_report) if stock_report else {}
    problems = []
    worst_error = 0.0
    worst_bone = None
    for bone in report["bones"]:
        translations = [tuple(sample["translation"])
                        for sample in bone["samples"]
                        if sample["frame"] >= PROOF_POSE_SAMPLE]
        if not translations:
            continue
        constant = all(t == translations[0] for t in translations)
        stock_bone = stock_bones.get(bone["name"])
        error = float("nan")
        known = False
        if stock_bone and stock_bone["samples"]:
            stock_translation = tuple(stock_bone["samples"][0]["translation"])
            error = position_error(translations[0], stock_translation)
            known = True
        ok = constant and known and error <= position_tolerance
        if known and error > worst_error:
            worst_error, worst_bone = error, bone["name"]
        if not ok:
            problems.append({
                "asset": label,
                "bone": bone["name"],
                "constantAcrossFrames": constant,
                "stockTranslationKnown": known,
                "positionErrorVsStock": None if not known else error,
                "translation": list(translations[0]),
            })
    return problems, worst_error, worst_bone


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--proof", required=True)
    parser.add_argument("--full", required=True)
    parser.add_argument("--stock", required=True,
                        help="Stock reference report, e.g. AIR_MA_IRONKICK")
    parser.add_argument("--base",
                        help="Optional base-animation report, e.g. SKEL_READY2")
    parser.add_argument("--candidates", default="28,29,30,31",
                        help="Full-clip frames tested against the proof pose")
    parser.add_argument("--out-md")
    parser.add_argument("--angle-tolerance-deg", type=float, default=0.1)
    parser.add_argument("--position-tolerance", type=float, default=0.00006)
    args = parser.parse_args()

    proof = load_report(args.proof)
    full = load_report(args.full)
    stock = load_report(args.stock)
    base = load_report(args.base) if args.base else None
    candidates = [int(v) for v in args.candidates.split(",") if v.strip()]

    sections = []
    sections.append(asset_summary("Static proof runtime asset (`MALE/COHSOURCEDEV_RETARGET_POSE_PROOF`)", proof))
    sections.append(asset_summary("Full clip runtime asset (`MALE/COHSOURCEDEV_RETARGET_SWING_FULL`)", full))

    match_rows, matched_frame = find_matching_frame(proof, full, candidates)
    lines = [
        "## Proof-frame match search (full clip vs static proof pose, proof sample 1)",
        "",
        "Runtime frame 0 is the bind reference; authored frames start at 1. The",
        "static proof asset holds one constant pose on samples 1..60 (verified),",
        "so every candidate below compares full@frame against that pose.",
        "",
        "| full frame | worst rotation error (deg) | worst bone | worst position error | missing samples |",
        "| ---: | ---: | --- | ---: | ---: |",
    ]
    for frame, angle, position, bone, missing in match_rows:
        lines.append(f"| {frame} | {angle:.6f} | {bone} | {position:.9g} | {missing} |")
    lines += ["", f"Matched full-clip runtime frame: **{matched_frame}**", ""]
    sections.append("\n".join(lines))

    comparison = compare_frame(proof, full, matched_frame, args.angle_tolerance_deg,
                               args.position_tolerance)
    header = (
        f"## Per-bone comparison: full@{matched_frame} vs static proof pose (sample {PROOF_POSE_SAMPLE})\n\n"
        f"`*` marks the hard-focus bones. Tolerances: rotation <= "
        f"{args.angle_tolerance_deg} deg, position <= {args.position_tolerance}."
    )
    sections.append(header + "\n\n" + comparison["markdown"])

    all_problems = []
    for label, report in (("proof", proof), ("full", full)):
        problems, worst_error, worst_bone = verify_constant_translations(
            label, report, stock, args.position_tolerance)
        all_problems.extend(problems)
        print(f"[bind-check] {label}: worst constant-translation error vs stock = "
              f"{worst_error:.9g} on {worst_bone}; problems={len(problems)}")

    result = {
        "matchedFullFrame": matched_frame,
        "firstDivergence": comparison["first_divergence"],
        "maxRotationErrorDegrees": comparison["max_rotation_error_degrees"],
        "maxRotationErrorBone": comparison["max_rotation_bone"],
        "maxPositionError": comparison["max_position_error"],
        "maxPositionErrorBone": comparison["max_position_bone"],
        "failures": comparison["failures"],
        "bindTranslationProblems": all_problems,
        "passed": (comparison["first_divergence"] is None and not all_problems),
    }

    print(json.dumps({k: v for k, v in result.items() if k != "bindTranslationProblems"},
                     indent=2))
    if all_problems:
        print(json.dumps(all_problems, indent=2)[:4000])

    if args.out_md:
        Path(args.out_md).write_text(
            "\n".join(sections) + "\n\n## Machine-readable summary\n\n```json\n" +
            json.dumps(result, indent=2) + "\n```\n", encoding="utf-8")

    return 0 if result["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
