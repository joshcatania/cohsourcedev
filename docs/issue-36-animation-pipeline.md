# Issue 36 — Animation authoring pipeline proof

Verified on 2026-08-21 from branch `agent/issue-36-web-swing` at
`8c7372d958dea0a457be050fe4495a7113d90b11`.

The requested `D:\github\coh-webswing` directory was not present.  The exact
branch and HEAD were checked out at `D:\github\cohsourcedev`, which was used
for every command below.  `D:\github\coh-graphics` was not modified and its
processes were not stopped.  Generated evidence is kept outside the source
tree under:

`agent/work/issue36-animation-proof-20260821/`

## Stock round trip

The test asset was `MALE/AIR_MA_IRONKICK` (`male/air_MA_ironkick`), length 120,
with base animation `male/skel_ready2`.  From `bin`, the verified sequence was:

```powershell
..\Utilities\GetAnimation2\bin\x86\Release\GetAnimation2.exe -runtime-rig MALE/AIR_MA_IRONKICK <stock>\original
..\Utilities\GetAnimation2\bin\x86\Release\GetAnimation2.exe -runtime-animx MALE/AIR_MA_IRONKICK <stock>\roundtrip.ANIMX
..\Utilities\GetAnimation2\bin\x86\Release\GetAnimation2.exe -compile-animx <stock>\roundtrip.ANIMX <stock>\original.SKELX MALE/COHSOURCEDEV_ROUNDTRIP_TEST MALE/SKEL_READY2 <stock>\COHSOURCEDEV_ROUNDTRIP_TEST.anim
..\Utilities\GetAnimation2\bin\x86\Release\GetAnimation2.exe -runtime-rig MALE/COHSOURCEDEV_ROUNDTRIP_TEST <stock>\roundtrip
python agent/compare-animation-roundtrip.py <stock>\original.json <stock>\roundtrip.json
```

The original report has a 68-bone hierarchy and 33 animation-authored tracks;
the base animation supplies the remaining 35 tracks.  The compiled report has
the same 68-bone hierarchy and resolves all samples.  The comparator result is:

| Measure | Result |
| --- | ---: |
| Maximum rotation error | 0.0831412692 degrees, `HANDL`, frame 22 |
| Maximum position error | 0.0000441939, `HANDL`, frame 2 |
| Failed samples | 0 |
| Hierarchy mismatches | none |
| Missing bones | none |
| Rotation tolerance | 0.1 degrees |
| Position tolerance | 0.00006 |

The tolerances are grounded in the runtime format rather than enlarged to hide
errors.  Runtime positions use a 1/32000 quantizer; the Euclidean worst case
for three components is `sqrt(3)/32000 = 0.0000541266`, so `0.00006` is a
small margin.  Runtime rotations use the shipped five-byte, 12-bit nonlinear
quaternion packing; the observed stock round trip peaks at 0.0832 degrees, so
0.1 degrees is a tight compression-level bound.  The round-trip images also
looked effectively identical when the source and rebuilt assets were
auditioned on the player.

## Developer canary audition

`agent/animation/canary-sequencer.inc` defines the private
`COHSOURCEDEV_CUSTOM_CANARY` move with normal sequencer relationships:

```text
Member "<COHSOURCEDEV_CUSTOM_CANARY>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
Interrupts "<COHSOURCEDEV_CUSTOM_CANARY>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
Requires COHSOURCEDEV_ANIMCANARY
Flags Cycle
```

The move is isolated in the private WebSwingDev overlay.  The new
`COHSOURCEDEV_ANIMCANARY` state bit is set only when `global_state.webswing_dev`
is active, and the developer-only `animcanary` command toggles that bit without
changing the legacy control packet layout.  This uses normal member/interrupt
selection; it does not call `seqSetMove`, bypass `seqAInterruptsB`, or repurpose
`TEST0`.

The tracked installer copies the overlay, normal include, canary include, and
state-bit file.  Final parity status was `installed=true` with these matching
hashes:

```text
overlay       4b21d8a1befeb3fecc8fa90d862d0431687b93a21b1237966f7c3c7e21975322
include       1c533024e0d15b2035b47e24b3d059b120c8f8303d46e3af6640d4ac3733143f
state bits    21bdddadcb6b0714eaa5017706a8946885739aa5b91f68e0d4ca4c69425821e06
canary include 0f30f8fa87b75ac685fd8823bf1e3d1a1a2082e8003b1535e6a19c21c1406b6e
```

The Python-authored canary was compiled from
`agent/animation/create_canary_animx.py`, loaded as
`MALE/COHSOURCEDEV_CUSTOM_CANARY`, and selected in the live WebSwingDev client.
The runtime log recorded `selectedMove=COHSOURCEDEV_CUSTOM_CANARY`, followed by
`selectedMove=READY` after `/animcanary 0`.  The visual contact sheet is at:

`agent/work/issue36-animation-proof-20260821/visual/custom/canary-contact-sheet.png`

## Blender authoring path

Blender was available locally:

```text
D:\Blender\blender.exe
Blender 5.2.0 LTS (fbe6228777e7)
```

`agent/animation/create_blender_canary.py` reconstructs the 68 Male export
bones from the stock runtime JSON, preserving exact names and parent IDs.  It
converts the runtime frame to the ANIMX/3ds-Max source frame, builds a Blender
armature with the reconstructed world rest matrices, then inserts actual
pose-bone quaternion/location keyframes.  The generated `.blend` contains 13
authored bones, four keyed frames (1, 12, 24, 36), 55 F-curves, and 220 keyframe
points.  The action was independently inspected through Blender 5.2's action
layer/strip/channel-bag API.

`agent/animation/blender_export_animx.py` samples the evaluated Blender pose.
For each frame it computes:

```text
delta = BlenderRest^-1 * EvaluatedPose
sourceWorld = ReconstructedANIMXRest * delta
```

It then emits `sourceWorld.translation`, unit scale, and the normalized
`sourceWorld` quaternion as ANIMX axis/angle data.  It does not emit local
pose-bone transforms or apply an unexplained corrective rotation.

The Blender evidence sequence was:

```powershell
D:\Blender\blender.exe --background --python agent\animation\create_blender_canary.py -- --rig-json <stock>\original.json --output <blender>\COHSOURCEDEV_CUSTOM_CANARY.blend --frames 36
D:\Blender\blender.exe --background --python agent\animation\blender_export_animx.py -- --blend <blender>\COHSOURCEDEV_CUSTOM_CANARY.blend --rig-json <stock>\original.json --output <blender>\COHSOURCEDEV_CUSTOM_CANARY.ANIMX --start-frame 1 --end-frame 36
..\Utilities\GetAnimation2\bin\x86\Release\GetAnimation2.exe -compile-animx <blender>\COHSOURCEDEV_CUSTOM_CANARY.ANIMX <stock>\original.SKELX MALE/COHSOURCEDEV_CUSTOM_CANARY MALE/SKEL_READY2 <blender>\COHSOURCEDEV_CUSTOM_CANARY_FROM_BLENDER.anim
..\Utilities\GetAnimation2\bin\x86\Release\GetAnimation2.exe -compile-animx <blender>\COHSOURCEDEV_CUSTOM_CANARY.ANIMX <stock>\original.SKELX MALE/COHSOURCEDEV_BLENDER_CANARY MALE/SKEL_READY2 <blender>\COHSOURCEDEV_BLENDER_CANARY.anim
```

Both Blender-generated runtime assets compiled and loaded.  The canonical
developer audition asset was replaced with the Blender-generated bytes so the
existing safe move selected `COHSOURCEDEV_CUSTOM_CANARY` without adding a
second gameplay path.  Runtime inspection reported length 36, 68 tracks, and
base `MALE/SKEL_READY2`.  Frame 0 matched the stock reference exactly in the
independent report comparison (maximum position error 0; maximum rotation
error 0 degrees).

The live client selected the same canary move from the Blender-generated asset;
the focused close captures show the authored arm/chest pose, and the clear
capture shows return to the stock pose:

```text
agent/work/issue36-animation-proof-20260821/visual/blender-close2/blender-close-00.png
agent/work/issue36-animation-proof-20260821/visual/blender-close2/blender-close2-contact-sheet.png
agent/work/issue36-animation-proof-20260821/visual/blender-close2/stock-after-clear.png
```

The source coordinate conversion is the exact inverse of the existing
`ConvertCoordsFrom3DSMAX` path.  Runtime game positions `(x, y, z)` map to
source positions `(-x, -z, y)`; equivalently source `(x, y, z)` becomes game
`(-x, z, -y)`.  A runtime quaternion axis maps to source axis `(x, z, -y)`.
This is the only handedness/up-axis correction used by the Blender scripts.

## Male/Fem/Huge audit

`AIR_MA_IRONKICK` was inspected on all three runtime rigs:

| Rig | Bones | Animation-authored tracks | Base animation |
| --- | ---: | ---: | --- |
| Male | 68 | 33 | `male/skel_ready2` |
| Fem | 63 | 36 | `fem/skel_ready2` |
| Huge | 60 | 34 | `huge/Skel_ready` |

The canonical animated names used by the canary—hips, waist, chest, collar,
upper/lower arms, upper legs, neck, and head—are present on all three rigs.
There are no parent-name mismatches among bones common to all three; the
hierarchy difference is eight optional names: Male-only wings and sleeves plus
`BOSOML`, `BOSOMR`, and `FACE` absent from Huge.  Rest local rotations are
identical in this runtime extraction.  Rest translations differ by body scale
and proportions; among the common canonical names, Male-to-Fem reaches 0.27
and Male-to-Huge reaches 0.22, while the largest common-bone difference is the
Fem `MYSTIC` bone at 0.6526.  One logical action can therefore reasonably drive
the shared named semantic bones, but separate Male/Fem/Huge export files are
required; a Male `.anim` must not be reused directly for all three.

## Build and ownership

GetAnimation2 Release/x86 built successfully with the repository's v145
fallback.  The current Game Release/x86 build also passed after the canary
source change; the final successful client log is
`agent/logs/build-client-Release-x86-20260821-194938.log`.

The warm ServerMonitor/DbServer/Launcher/MapServer shard was preserved.  Only
the cohsourcedev Ouroboros GUI client was restarted for the two audition
passes; no ServerMonitor, DbServer, or MapServer restart was required.  No Web
Swing physics, rope, traversal, tether, or existing move files were changed.

The `.blend`, ANIMX, extracted JSON/SKELX, runtime `.anim`, screenshots, and
contact sheets remain local evidence under `agent/work` and are intentionally
not source artifacts in the commit.  The two small Blender scripts, the
comparator tightening, the developer audition wiring/parity fixes, and this
document are the reproducible source changes.
