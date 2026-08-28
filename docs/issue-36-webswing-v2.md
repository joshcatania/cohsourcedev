# Issue 36 Web Swing V2 proof

Date: 2026-08-22

This is the bounded Male Web Swing animation proof from the expected branch
`agent/issue-36-web-swing` at baseline `79f70b45205f1acfd212c364530370961f430856`.

The pre-fix V2 candidate was not rotation-only: the old frame-15 Blender
check measured a maximum local arm translation of `0.341128012` on `UARML`
(with a separate maximum scale error of approximately `5.96e-7`).

## Scope

Only the Male attached and descending Web Swing phases use
`MALE/COHSOURCEDEV_WEBSWING_STRETCH_V2`. Male airborne remains the stock
control. Male bottom and ascend remain the existing V1 assets. Fem and Huge
remain on V1 for every custom Web Swing phase. No rope, traversal, movement,
or physics source was changed.

## Authoring result

`agent/animation/create_webswing_animation.py` now authors the arm chain with
rotation-only local channels. The pose is solved in armature space and
converted back through Blender's local-pose basis; non-zero local translation
or non-unit scale fails the validation gate at `1e-5`. Elbow-plane roll is
derived from the solved upper/forearm plane with pole-derived sign continuity.
The hand orientation blends tether direction with forearm direction and uses
the solved roll normal as its up axis.

Candidate A was rejected as the deliberate negative (overhead/folded right
arm silhouette). Candidate B was selected over C because its elbow and hand
read most cleanly from the side and gameplay views. The candidate contact
sheets, with colored XYZ axes and asymmetric axial-twist fins, are in:

`agent/work/issue36-webswing-v2-rotation-safe/candidates/`

The visual review was explicit, not metric-only:

- Candidate A: reject. The right upper arm/forearm folded into an overhead,
  inverted-looking path; the elbow and hand became hard to read from the side
  and gameplay views, so it was retained only as the deliberate negative.
- Candidate B: select. Both shoulders stay attached, the right elbow sits in a
  readable bend below the tether line, the hand follows the tether, and the
  left arm counterbalances without crossing the torso. The same relationship
  remains legible from all five views.
- Candidate C: reject as weaker, not numerically invalid. It passed the same
  reach and rotation-only gates, but the more extended elbow/forearm silhouette
  was less distinct from the side and gameplay cameras and gave a less useful
  visual proof than B.
- Candidate D: not needed; B passed the bounded review.

The final Blender authoring log is
`agent/work/issue36-webswing-v2-rotation-safe/B-final-authoring.log`. Frames
1, 6, 15, 24, and 30 pass with zero local translation and zero scale error on
all eight arm-chain bones per side. The independent all-integer-frame check
over frames 1–30 also reports zero maximum translation and scale error. Reach
errors remain below `8.0e-7` in the keyed validation poses.

## Runtime asset and mapping

The selected V2 asset is:

`agent/animation/runtime/player_library/animations/male/COHSOURCEDEV_WEBSWING_STRETCH_V2.anim`

It is a 30-frame runtime animation compiled from the existing ANIMX/SKELX
pipeline. SHA-256:

`35b6da70ed03466828a38aa31270b22b139384a7a799f8bd8f2b9cbf05ece0e4`

The V1 Male stretch remains unchanged:

`6d4f0ab66e592b7e9e95cc78f088043a6adb2f68eb9100b729f1f25ddfad7ec8`

The manifest contains ten assets, and the installer reports both source and
runtime sets valid. The runtime include is synchronized from
`agent/webswing-animation/webswing.inc`.

The exact five-view candidate paths are:

`agent/work/issue36-webswing-v2-rotation-safe/candidates/{A,B,C}-frame15-axes/{front,front-3-4,side,rear-3-4,gameplay}.png`.

Each image includes colored XYZ axis rods at the collar, upper arm, forearm,
and hand, plus asymmetric gold/cyan roll fins and a WEP marker. The fins make
forearm corkscrews, inverted wrists, collar roll, and upper-arm axial twist
visible where round cylinders would hide them.

The V2 runtime inspection through GetAnimation2 loaded
`MALE/COHSOURCEDEV_WEBSWING_STRETCH_V2` with length `30.000` and `68` tracks;
the machine-readable inspection is
`agent/work/issue36-webswing-v2-rotation-safe/runtime-v2.json`. The compile
step exited 0 and wrote the runtime asset; the final output, tracked asset,
and manifest all agree on the SHA-256 above.

## GUI proof

The real `Ouroboros.exe` client was launched in WebSwingDev mode with account
`Dummy00009`, character `SwingV2`, and Male body data. The client log records
the actual sequence: `WEBSWING_AIRBORNE`, attach, `WEBSWING_DESCEND`,
`WEBSWING_BOTTOM`, `WEBSWING_ASCEND`, detach, and return to airborne. The
client also recorded one rendered tether line while attached.

Evidence:

- `agent/work/issue36-webswing-v2-rotation-safe/gui/v2-swing-zoom-1-airborne.png`
- `agent/work/issue36-webswing-v2-rotation-safe/gui/v2-swing-zoom-2-attach.png`
- `agent/work/issue36-webswing-v2-rotation-safe/gui/v2-swing-zoom-3-bottom.png`
- `agent/work/issue36-webswing-v2-rotation-safe/gui/webswing-v2-gui-proof.mp4`
- `agent/work/issue36-webswing-v2-rotation-safe/gui/webswing-off-negative-control.png`
- `bin/logs/game/webswing.log`

The MP4 is an 18-second, 30-fps H.264 desktop capture at 1272x734. The
negative-control image was taken after the in-game `/webswing 0` command and
shows the normal idle state with the client confirmation message.

The same GUI log shows `compiledFound=1`, `overlay_load result=selected`,
`selected_source=COMPILED_OVERLAY`, `include_consumed=1`, and all five Web
Swing moves resolved. The attached/descending Male entries in that consumed
overlay are the V2 mapping above; no missing-animation, thumbs-up, or runtime
load-error line was produced. The client then selected attached, descend,
bottom, and ascend transitions and rendered a tether line. `/webswing 0`
returned the character to stock locomotion state; the normal movement source
and all traversal/physics sources remained outside the change set.

## Regression checks

The clean-account server smoke passed on `Dummy00010`:

- 15/15 selected anchors and attach/detach pairs
- 577 swing samples
- all six steering buckets
- zero hard corrections
- smoothness, constraint, retained-momentum, and divergent-anchor gates pass

Machine-readable result:
`agent/logs/webswing-smoke-20260822-070917.json`.

The client-only Release/x86 rebuild passed with the verified v145 fallback:
`agent/logs/build-client-Release-x86-20260822-071159.log`.

A full-solution build was also attempted while the warm shard was running;
its compiler phase did not report a source error, but legacy post-build copies
were blocked by the live server process locks. That log is
`agent/logs/build-Release-x86-20260822-064639.log`; the client-only build is
the successful binary-build gate for this data-only change.

## Final review gate

- Starting SHA: `79f70b45205f1acfd212c364530370961f430856`.
- Implementation SHA: `3145d076b`; the final branch tip is verified and
  reported on PR #37 after this report update.
- Exact mapping: only Male `WEBSWING_ATTACHED` and `WEBSWING_DESCEND` use
  `COHSOURCEDEV_WEBSWING_STRETCH_V2`; Male airborne/bottom/ascend and all
  Fem/Huge phases remain V1.
- Rotation-only integrity: location tolerance `1e-5`, scale-identity
  tolerance `1e-5`; all eight arm bones per side report `0` location and `0`
  scale error at every keyed frame and across the independent integer-frame
  1–30 check.
- Pose conversion: armature-space targets are converted through the parent
  pose and rest matrices with Blender `Bone.convert_local_to_pose(...,
  invert=True)`, then applied as zero local location, quaternion rotation, and
  unit local scale.
- Roll: the normalized solved upper-direction cross lower-direction defines
  the shared elbow-plane normal; the pole supplies the near-straight fallback
  and neighboring-frame sign continuity. The hand forward axis blends tether
  direction `0.78` with forearm direction `0.22`, and its up axis is projected
  from that same roll normal.
- Stock/physics controls: `/webswing 0` produced the normal idle control;
  no physics, traversal, rope, or movement source file changed. The clean
  server smoke also recorded 15/15 anchor selections, 15 attach/detach pairs,
  577 swing samples, and zero hard corrections.

This proof stops here for Josh/Sol visual review. No Tuck V2, Ascend V2, Fem,
Huge, release flip, runtime body pitch, IK, or physics work was started.
