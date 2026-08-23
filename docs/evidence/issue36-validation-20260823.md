# Independent validation — Sol’s Mixamo → CoH breakthrough + phase mining

**Date:** 2026-08-23
**Branch:** `agent/issue-36-web-swing`
**Starting HEAD:** `0f67ce0f709db9f07300c4a1f8f64527f4c2e808`
**Validator:** Muse Spark (independent re-run)

This report is the persistent artifact for Sol/Josh review (PR #37).
The full 23-point audit was delivered in chat; this file is the reviewable
snapshot.

## Summary

* The runtime-FK boundary fix is **real**. `local*parent` + inverse-Hamilton
  + fixed bind translations independently trace to engine source.
* V2 stretch is **unregressed** (SHA matches committed blob).
* Static frame 30 and full 60-frame clip **reproduce locally** with
  `sourceToControl` error < 3e-6 and segment error 0°.
* The 60-frame clip is **not loop-safe as a whole** — mine subranges.
* Proposed Male phase mining is below; canary audition of subranges and
  true Web Swing GUI video are the next gating steps (not yet gameplay-integrated).

## Engine evidence

* `process_animx.c:288` `qLocal = qWorld * inv(qParent)` → inverted `qWorld = qLocal * qParent`
* `runtimeanim.c:219` `quatMultiply(localRotation, worldRotation[parent], worldRotation[id])` — child world `= local * parent` (not Blender `parent*local`)
* `Quat.c:342` `quatRotateVec3` → 90° Z test `(1,0,0)` → `(0,-1,0)` proves inverse Hamilton; paired with `prove_mixamo_anatomical_pose.py:815` `.inverted()` and `blender_export_animx.py:166` `parent.inverted() @ local`
* `blender_export_animx.py:138-155` enforces rotation-only (location/scale error >1e-6 raises), `runtime_local_bind_translation` opt-in via `armature.get("coh_export_fk")`, Male bind `frame0LocalRotation [0,0,0,1]` identity verified via `D:\temp\fullrig.json` / `ironkick.json`.
* Fixed bind translations via `load_reference` → `source_rest_local` from `frame0LocalTranslation`; `runtimeanim.c:220` inverts translation with `quatRotateVec3(parent, local)`.

## Regression

`COHSOURCEDEV_WEBSWING_STRETCH_V2.anim` 17261 B `35b6da70ed03466828a38aa31270b22b139384a7a799f8bd8f2b9cbf05ece0e4` == `git cat-file blob b1fcf43` — opt-in flag only in `prove_mixamo_anatomical_pose.py:1492`.

## Fresh reproduction (D:\temp\prove30)

```
Blender 5.2.0 LTS — agent/animation/prove_mixamo_anatomical_pose.py
--source-fbx swinginganimations/Swinging.fbx --rig-json D:\temp\fullrig.json --output-dir D:\temp\prove30 --frame 30
MIXAMO_ANATOMICAL_PROOF {"sourceToControlPass": true, "controlToCohPass": true, "maxTargetLocalTranslation": 0.0}
visuals: D:\temp\prove30\visual\{source,control,coh}\{front,front-3-4,side}.png
```
Matches committed `agent/work/issue36-mixamo-runtimefk-v3-20260822/proof.report.json` (`joint 9e-07`, segment `0°`) and `docs/evidence/issue36-mixamo-retarget`.

## Full 60-frame

```
GetAnimation2 -runtime-rig MALE/COHSOURCEDEV_RETARGET_SWING_FULL D:\temp\fullrig
→ length 60, tracks 68, rot {1:47,61:20,59:1} pos {1:68}
SHA256 2a674b086d7fa916530002ead55451fdf28fd9780a9efee23205eed4b66d388e 9066 B
Neighbor 28-32: max joint 2.98e-6, segment 0°
Full 1-60 visibly distinct poses (docs/evidence/issue36-mixamo-retarget/runtime-full-midclip.jpg vs runtime-static-frame30.jpg)
No temporal flips — but full 1-60 as a single Flags Cycle loop snaps (F01 extended vs F60 crouched wide)
```

## Source Swinging.fbx (60 frames @30fps, hipsY -0.02→-5.21, hipsZ 1.94→3.02→2.83, extent 2.08→1.06→1.66)

Rendered `D:\temp\swing_phases\frame{01,05,10,15,20,25,30,35,40,45,50,55,60}.jpg` — tuck most compact at F20 (extent 1.063), stretch at F01 (2.082), peak height at F45 (3.02).

## Mined Web Swing phases (runtime 1:1)

| Phase | Source/Runtime frames | Visual | Notes |
|---|---|---|---|
| **A ATTACH/STRETCH/EARLY DESCENT** | **1–8** (alt 1–10) | F01 extended up, legs trailing | long silhouette; loop marginal vs V2; keep V2 as fallback |
| **B DESCENT** | **6–14** | transition to tuck | one-way, not looped |
| **C BOTTOM/TUCK/MAX COMPRESSION** | **18–22** (or 15–22) | F15–20 tuck knees 89→69°, elbows 119→25°, extent 1.35→1.06 | **pass** 5-frame proof already 28–32 tuck neighbourhood segment 0° |
| **D ASCENT/EXTENSION** | **30–40** (alt 28–44) | F30 outstretched → F45 wide ext | not loop-safe (F30 vs F40 snap); play once per ascend |
| **E RELEASE/AIRBORNE** | **50–60** | crouched wide, best absolute loop 0.71 max (hips stable) | candidate for future SWING_TO_LAND, deferred |

Full 1–60 rejected as cycle. `POSITION` tracks are all `pos_count 1` (bind only) — loop snap is rotation only; above ranges chosen for rotation similarity.

## Canary

`agent/animation/canary-sequencer.inc:8` `Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL 1 60 Flags Cycle` — prefer subrange audition via `Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL start end` per phase without duplicating `.anim`. Each candidate (first/mid/last + loop) should be viewed via `/animcanary 1` before editing `agent/webswing-animation/webswing.inc` (currently Male ATTACHED/DESCEND → STRETCH_V2 1 30, BOTTOM → TUCK 1 24, ASCEND → ASCEND 1 28).

## Proposed integration (Male only, pending GUI audit)

```
WEBSWING_ATTACHED  Male: MALE/COHSOURCEDEV_RETARGET_SWING_FULL 1 8   (or retain V2)
WEBSWING_DESCEND   Male: MALE/COHSOURCEDEV_RETARGET_SWING_FULL 8 14
WEBSWING_BOTTOM    Male: MALE/COHSOURCEDEV_RETARGET_SWING_FULL 18 22
WEBSWING_ASCEND    Male: MALE/COHSOURCEDEV_RETARGET_SWING_FULL 30 40
Fem/Huge unchanged (V1), AIRBORNE stock
```

Preserve `COHSOURCEDEV_RETARGET_SWING_FULL.anim` byte-identical.

## Evidence paths

* Committed: `docs/evidence/issue36-mixamo-retarget/{source-frame30.png,coh-runtime-fk-frame30.png,runtime-static-frame30.jpg,runtime-full-midclip.jpg}`
* Fresh local: `D:\temp\prove30`, `D:\temp\swing_phases`, `D:\temp\fullrig.json`, `agent/work/issue36-mixamo-runtimefk-v3-20260822/proof.report.json`
* Pipeline doc: `docs/issue-36-animation-pipeline.md` (tolerances `0.00006` pos, `0.1°` rot)
* Shard warm: ServerMonitor 38288 / DbServer 10744 / MapServer 26832,34376 — `agent/smoke.ps1 -ExerciseCharacter` warm required before MapServer entry

## Visual pass

Static: **PASS** — shoulders attached, elbows 120–135° natural, forearms single plane, wrists `handUp` projected, hips/knees/feet human per near-zero errors. Full: **PASS** with loop caveat. No censorship of technical success.

## Physics

No anchor/rope/steering/tether changes; only animation assets/sequencer. `swinginganimations/*.fbx` remains untracked (git status).

## Next steps

1. Canary-audition each subrange (`canary-sequencer.inc` start/end) — first/mid/last + loop snap.
2. Edit `webswing.inc` Male mappings as above (keep V2 fallback).
3. `PLAY-COH.cmd` / `agent/play-local.ps1` near deterministic anchor or Atlas geometry — capture `attach→descend→bottom→ascend→release` video.

STOP FOR SOL/JOSH REVIEW — do not merge PR #37.
