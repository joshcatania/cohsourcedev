# Issue 36 — full corrected Male rest-basis swing

Date: 2026-08-25

Branch: `agent/issue-36-web-swing`

Starting HEAD: `a30939fcac3673ea2af9aefcc9358b8de0d9b6be`

PR #37 remains a stacked draft.

## Result

The accepted rest-basis transfer used for source frames 18..22 now covers the
complete authored action without changing the exporter convention. The result
is a distinct private runtime asset:

```text
MALE/COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL
```

| Property | Result |
|---|---:|
| Source file | `swinginganimations/Swinging.fbx` (local, untracked) |
| Source action | `Armature|mixamo.com|Layer0` |
| Source range / rate | `1..60` / `30 fps` |
| Authored runtime samples | `60` |
| Runtime bones | `68` |
| Base animation | `MALE/SKEL_READY2` |
| Runtime asset SHA-256 | `acc0fc2d0ce4f382ec1cbf6675dfb3f6f96747080ffe24f6cc1c53866dfec840` |
| Maximum decoded runtime-local error | `0.0768279269°` |
| Maximum bind translation error | `0.0` |
| Maximum authored translation drift | `0.0` |
| Full 18..22 versus accepted BOTTOM | `0.0000029576°` maximum |
| Animated bones | `19` |
| Full proof | **PASS** (`0` failed checks) |

The checker also requires exact 60-frame/68-bone coverage, exact stock bone
names and topology, all expected Blender proof samples, fixed stock bind
translations, and a genuinely changing clip. See [runtime-proof.json](runtime-proof.json).

## Rebuild

From the repository root:

```powershell
.\agent\animation\build_issue36_rest_basis_full.ps1 -Promote
```

The wrapper uses Blender 5.2.0 LTS at `D:\Blender\blender.exe`, exports with
`agent/animation/blender_export_animx.py`, compiles with
`Utilities/GetAnimation2/bin/x86/Release/GetAnimation2.exe` 2.07, decodes the
temporarily decodes the logical runtime asset with `-runtime-rig`, restores its
prior loose-data state even on failure, runs the preliminary full proof, and
renders the 13-frame [source/target contact sheet](source-target-contact-sheet.jpg).
Intermediates remain inspectable under
`agent/work/issue36-rest-basis-full-20260825/`. Only after that proof passes,
`-Promote` updates the tracked private library, invokes the existing installer,
decodes the installer-installed asset, and requires a second full proof. It
does not replace stock or older WebSwing assets.

## Technical correspondence audit

The contact sheet samples frames `1, 6, 12, 17, 18, 20, 22, 27, 33, 39, 45,
52, 60`. Source and corrected Male target retain the same reach direction,
torso twist, hip orientation, knee bend, and head direction. No left/right
mirror, inverted limb, detached chain, abrupt rest-pose contamination, or
full-clip static-pose failure was observed. This is a technical correspondence
pass, not a claim of final subjective gameplay quality.

The selected phase windows follow the actual motion:

| Phase | Source frames | Sequencer behavior | Rationale |
|---|---:|---|---|
| ATTACH | `1..8` | one shot, hold 8 | initial overhead reach and extension |
| DESCEND | `9..17` | one shot, hold 17 | body closes and drops toward the compressed arc |
| BOTTOM | `18..22` | one shot, hold 22 | previously accepted compact BOTTOM neighborhood |
| ASCEND | `23..40` | one shot, hold 40 | sustained opening/rise to the widest useful ascent pose |
| RELEASE/AIRBORNE | stock fallback | existing stock move | `41..49` is a possible continuation, but AIRBORNE also covers pre-attach; `50..60` reads as landing recovery |

Holds use `Scale 0`; only the deliberate hold moves carry `Flags Cycle`.
None of the dramatic authored windows loops.

## Experimental mode 3

The existing modes retain their meanings:

| `webswinganim` | Name | Behavior |
|---:|---|---|
| 0 | `SAFE_NONE` | unchanged; no WebSwing visual bits |
| 1 | `ALL_EXPERIMENTAL` | unchanged existing broad experiment |
| 2 | `MALE_BOTTOM_ONLY` | unchanged accepted Male BOTTOM audition |
| 3 | `MALE_FULL_CORRECTED` | new Male-only corrected ATTACH/DESCEND/BOTTOM/ASCEND |

Mode 3 is opt-in through the private WebSwingDev overlay. It uses the existing
seven overlay state bits; the normal overlay totals 882 states. The optional
canary is the eighth overlay slot and totals 883, below `MAXSTATES=884`.
`STATE_ARRAY_SIZE` remains 28. Fem and Huge continue through the existing
generic phase moves.

## Controlled runtime audit

The normal installer validated the new source/runtime SHA. A canary install and
four deterministic `Dummy00009` / `Swingv3` Male captures then ran with
`-webswingdev -webswinganim 3 -animcanary 1 -nosharedmemory`; every capture
completed and the client exited cleanly. The client parser selected the
compiled private overlay, resolved all eight mode-3 moves to the new asset, and
reported `MALE_FULL_CORRECTED`. The representative selection excerpt is in
[runtime-identity.txt](runtime-identity.txt).

[Attach frame 6](actual/attach-frame06.jpg) visibly shows the corrected custom
one-arm reach on the actual skin. The frame 12, 20, and 33 captures landed in
the short login fade/READY transition before the canary was visible; their
logs still show the canary selecting the new asset immediately afterward.
Those three images are retained as timing evidence, not claimed as pose proof:
[descend](actual/descend-frame12.jpg), [bottom](actual/bottom-frame20.jpg), and
[ascend](actual/ascend-frame33.jpg). This bounded audit proves parser, asset,
mode, and actual-skin selection without tuning traversal physics.

The Release/x86 v145-fallback build passed in 8.2 seconds. The direct-DB
TestClient character smoke timed out without login markers and was not used as
an animation gate, per the task scope. The shard was stopped, the temporary
canary was removed by restoring the normal private install, the pre-existing
capture target was restored byte for byte, and both local mode configuration
files were returned to mode 2.

## Scope and safety

`Common/entity/entworldcoll.c`, `Common/entity/motion.h`, the animation exporter,
root-translation policy, old corrected BOTTOM asset, old uncorrected full asset,
Fem/Huge assets, and stock pigg animation content are unchanged. The local
TestClient fixture and untracked source FBX remain outside these commits.
