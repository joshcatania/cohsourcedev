# Issue 36 — Web Swing animation set V1

Verified on 2026-08-21 from branch `agent/issue-36-web-swing` in
`D:\github\cohsourcedev`. The requested `D:\github\coh-webswing` checkout was
not present, so that exact branch checkout was used. `D:\github\coh-graphics`
was not modified and its processes were not stopped.

## Authoring and export

The authored set uses Blender 5.2 and the repository's `GetAnimation2`
Release/x86 tool. `agent/animation/create_webswing_animation.py` reconstructs
the Male, Fem, and Huge runtime rigs from extracted JSON, creates a real
armature/action, and keys 19 semantic bones shared by all three rigs:

```text
HIPS WAIST CHEST COL_R UARMR LARMR HANDR COL_L UARML LARML HANDL
ULEGR LLEGR FOOTR ULEGL LLEGL FOOTL NECK HEAD
```

Each logical action is loop-safe, uses no root translation, and has a clear
whole-body silhouette:

| Action | Frames | Keyed source frames | Read |
| --- | ---: | --- | --- |
| `COHSOURCEDEV_WEBSWING_STRETCH` | 30 | 1, 6, 15, 24, 30 | long reaching/hanging body |
| `COHSOURCEDEV_WEBSWING_TUCK` | 24 | 1, 5, 12, 19, 24 | compact folded bottom pose |
| `COHSOURCEDEV_WEBSWING_ASCEND` | 28 | 1, 6, 14, 22, 28 | open outbound reach |

The Blender action is exported to ANIMX through
`agent/animation/blender_export_animx.py`, then compiled to runtime `.anim`
with `GetAnimation2 -compile-animx`. The nine final files are tracked under
`agent/animation/runtime/player_library/animations/` and their byte counts and
SHA-256 values are authoritative in
`agent/animation/runtime/webswing-animations.json`.

The extracted source rigs used for the export were:

| Runtime type | Bones | Base animation |
| --- | ---: | --- |
| Male | 68 | `MALE/SKEL_READY2` |
| Fem | 63 | `FEM/SKEL_READY2` |
| Huge | 60 | `HUGE/Skel_ready` |

## Runtime mapping

`agent/webswing-animation/webswing.inc` keeps the existing state-bit and move
relationship structure. It changes only the animation ranges for the five
existing Web Swing states:

| Web Swing state | Runtime animation |
| --- | --- |
| `WEBSWING_AIRBORNE` | existing stock airborne mapping |
| `WEBSWING_ATTACHED` | `MALE/FEM/HUGE/COHSOURCEDEV_WEBSWING_STRETCH`, 1–30 |
| `WEBSWING_DESCEND` | `MALE/FEM/HUGE/COHSOURCEDEV_WEBSWING_STRETCH`, 1–30 |
| `WEBSWING_BOTTOM` | `MALE/FEM/HUGE/COHSOURCEDEV_WEBSWING_TUCK`, 1–24 |
| `WEBSWING_ASCEND` | `MALE/FEM/HUGE/COHSOURCEDEV_WEBSWING_ASCEND`, 1–28 |

`WEBSWING_RELEASE` remains intentionally unmapped. No physics, rope,
traversal, tether, or movement source was changed for this asset pass.

## Canary isolation

Normal `agent/webswing-animation/webswing.txt` contains only the five-state
overlay, and normal `webswing.statebits` contains only the five Web Swing bits.
The canary is opt-in through `webswing-canary.txt`,
`webswing-canary.statebits`, and `-IncludeCanary` on
`agent/install-webswing-animation.ps1` or `agent/play-local.ps1 -WebSwingCanary`.

Normal installation removes the tracked runtime canary include and does not
require the legacy `COHSOURCEDEV_CUSTOM_CANARY.anim`. Explicit canary mode
requires that audition asset and installs the private canary include/state bit.
This keeps the normal WebSwingDev path free of the canary dependency while
retaining a reversible developer audition path.

## Validation evidence

- Blender script syntax passed with Python compilation; all nine Blender
  authoring runs produced real armature/action keyframes.
- All nine ANIMX files compiled with `GetAnimation2`; direct runtime-byte
  inspection passed for Male/Fem/Huge bases and 30/24/28 frame lengths.
- GetAnimation2 Release/x86 build passed with the repository's v145 fallback.
- Game Release/x86 build passed with the repository's v145 fallback; log:
  `agent/logs/build-client-Release-x86-20260821-214803.log`.
- Normal installer parity passed with all nine source/runtime hashes equal,
  `normalModeInstalled=true`, `canaryModeInstalled=false`, and no runtime
  canary include.
- Explicit canary installation passed with the canary overlay/state bit and
  all nine normal assets hash-valid. The opt-in client loaded
  `COHSOURCEDEV_ANIMCANARY` as state-bit index 880 and resolved all five normal
  Web Swing overlay moves; the canary remains a developer-only audition path.
- The human normal-mode gate is the supplied 17.17-second Steel Canyon capture:
  `C:\Users\relat\Videos\Captures\Steel Canyon 2026-08-21 21-33-36.mp4`.
  The normal client log for PID 37292 recorded `selectedMove` transitions
  through `WEBSWING_DESCEND`, `WEBSWING_BOTTOM`, `WEBSWING_ASCEND`,
  `WEBSWING_ATTACHED`, and `WEBSWING_AIRBORNE` with the compiled overlay
  selected and no missing-animation diagnostics.
- The primary direct-DB character/map smoke passed on `Dummy00018`:
  `agent/logs/smoke-directdb-20260821-214852.json`.
- The autonomous server-side Web Swing smoke passed on the repository-standard
  `Dummy00009` account with 15/15 selected anchors, all six yaw/steering
  buckets, 15 attach/detach sequences, retained release momentum, no hard
  corrections, and smoothness gates passing:
  `agent/logs/webswing-smoke-20260821-215221.json`. TestClient has no renderer,
  so the supplied GUI clip remains the visual animation checkpoint.

Generated `.blend`, ANIMX, extracted rig reports, runtime inspection files,
screenshots, and contact sheets remain local under the ignored
`agent/work/issue36-webswing-v1/` evidence directory and are not release
artifacts.
