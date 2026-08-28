# Issue 36 — corrected rest-basis BOTTOM 18..22 proof

Date: 2026-08-24
Branch: `agent/issue-36-web-swing`
Starting HEAD: `74169d3ce6021e04d13640b2b45fdba61a25d373`
PR #37: stacked draft; not merged and not rebased

## Status

```yaml
"source->Male retarget orientation fix direction": confirmed
"corrected frame20 actual-skin": PASS
"corrected bottom 18..22 actual-skin": PASS
"full 60-frame animation": NOT PROVEN
"ASCEND 30..40": NOT IMPLEMENTED
```

The corrected BOTTOM range is a PASS on the actual `Dummy00009` / `Swingv3`
Male skin. Enlarged front, three-quarter, and side review across all five
source frames shows a coherent compressed swing tuck: torso and hips remain
related, both arm chains and both leg chains remain attached, and no visible
corkscrew, detached segment, or extreme stretch appears.

Review sheet: [actual-bottom-contact-sheet.jpg](actual-bottom-contact-sheet.jpg)

## Bounded asset

The proof asset is separate from production:

```text
MALE/COHSOURCEDEV_RETARGET_RESTBASIS_BOTTOM
```

Authored runtime samples map exactly as follows:

| Authored runtime sample | Source frame |
|---:|---:|
| 1 | 18 |
| 2 | 19 |
| 3 | 20 |
| 4 | 21 |
| 5 | 22 |

The generator applies the accepted frame-20 transfer to every required chain:
torso, both arms, both legs, and head/neck.  The transfer is the explicit
source-pose delta multiplied by the target rest basis.  Bind translations are
fixed, pose-bone translations are zero, and scale remains at one.

The production asset `MALE/COHSOURCEDEV_RETARGET_SWING_FULL` was not opened for
writing, and the production WebSwing sequencer was not changed.

## Blender and runtime proof

The target is the exact `MALE/SKEL_READY2` 68-bone runtime skeleton.  Blender
pre-export proof passed for all five samples and 19 focused bones:

| Check | Result |
|---|---:|
| Direction error | `0.0°` |
| Maximum roll error | `0.002097°` |
| Maximum full-basis error | `0.039565°` |
| Maximum runtime-world quaternion error | `0.039565°` |
| Maximum local translation magnitude | `0.0` |
| Maximum local scale error | `3.5763e-7` |

The established runtime pipeline then exported ANIMX, compiled with
`GetAnimation2`, and decoded the installed `.anim`:

| Check | Result |
|---|---:|
| Runtime animation | `MALE/COHSOURCEDEV_RETARGET_RESTBASIS_BOTTOM` |
| Base animation | `MALE/SKEL_READY2` |
| Runtime length / authored samples | `5 / 5` |
| Runtime tracks | `68` |
| Bind translation error | `0.0` |
| Hierarchy mismatch | `0` |
| Authored translation drift | `0.0` |
| Maximum decoded runtime-local rotation error | `0.072206°` (`ULEGL`, authored sample 2) |
| Runtime proof checks failed | `0` |

The maximum authored adjacent rotation delta is `30.069°` on `HANDR` between
authored samples 4 and 5; it is retained as a measured motion transition, not
a transport failure.

Machine-readable reports:

- [rest-basis-bottom.math.json](rest-basis-bottom.math.json)
- [runtime-proof.json](runtime-proof.json)

## Actual-skin canary

The canary was installed through the existing `COHSOURCEDEV_CUSTOM_CANARY`
path, not through the production move.  The full bounded include maps runtime
samples `1..5` to `MALE/COHSOURCEDEV_RETARGET_RESTBASIS_BOTTOM` and is left
staged for Sol/Josh review.

Test identity was `Dummy00009` / `Swingv3`, with `seq_type=male`,
`calculated_type=male`, `is_male=1`, and `TypeGfx=male`.  The selection log also
records `selectedMove=COHSOURCEDEV_CUSTOM_CANARY` and
`AnimP=MALE/COHSOURCEDEV_RETARGET_RESTBASIS_BOTTOM`.

Identity and capture details are recorded in
[runtime-identity.txt](runtime-identity.txt).  The 15 committed actual-skin
images are under [actual](actual/):

| Source frame | Views |
|---:|---|
| 18 | [front](actual/SWINGV3_BOTTOM_FRAME18_front.jpg), [three-quarter](actual/SWINGV3_BOTTOM_FRAME18_threequarter.jpg), [side](actual/SWINGV3_BOTTOM_FRAME18_side.jpg) |
| 19 | [front](actual/SWINGV3_BOTTOM_FRAME19_front.jpg), [three-quarter](actual/SWINGV3_BOTTOM_FRAME19_threequarter.jpg), [side](actual/SWINGV3_BOTTOM_FRAME19_side.jpg) |
| 20 | [front](actual/SWINGV3_BOTTOM_FRAME20_front.jpg), [three-quarter](actual/SWINGV3_BOTTOM_FRAME20_threequarter.jpg), [side](actual/SWINGV3_BOTTOM_FRAME20_side.jpg) |
| 21 | [front](actual/SWINGV3_BOTTOM_FRAME21_front.jpg), [three-quarter](actual/SWINGV3_BOTTOM_FRAME21_threequarter.jpg), [side](actual/SWINGV3_BOTTOM_FRAME21_side.jpg) |
| 22 | [front](actual/SWINGV3_BOTTOM_FRAME22_front.jpg), [three-quarter](actual/SWINGV3_BOTTOM_FRAME22_threequarter.jpg), [side](actual/SWINGV3_BOTTOM_FRAME22_side.jpg) |

## Scope boundary

This task proves only the corrected BOTTOM range `18..22`.  It does not prove
the full 60-frame animation and does not implement ASCEND.  The next task
recommendation is:

```text
ASCEND 30..40 with HOLD at 40
```

No exporter/runtime-FK, system/shard, WebSwing mode, physics, anchors, rope,
steering, jump, or tether work was reopened.
