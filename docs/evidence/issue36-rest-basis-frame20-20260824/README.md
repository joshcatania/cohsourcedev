# Issue 36 — frame-20 rest-basis forensic canary

RETARGET REST-BASIS DIAGNOSIS: PASS
CORRECTED ACTUAL-SKIN FRAME20: AWAITING JOSH

This is one complete static corrected Male pose source frame, created from
`swinginganimations/Swinging.fbx`, action `Armature|mixamo.com|Layer0`, source
frame 20. The target is the exact 68-bone CoH Male export rig. The focused proof
covers HIPS/WAIST/CHEST/NECK/HEAD, both shoulders and arms/hands, and both
legs/feet (19 bones); the authored animation is static across five frames with
fixed bind translations, zero pose location, and unit pose scale.

## Mathematical and runtime proof

The diagnostic rest-basis transfer is:

`sourceDelta = sourcePoseWorld * inverse(sourceRestWorld)`
`targetPoseWorld = sourceDelta * targetRestWorld`

The resulting runtime-local channels use the existing CoH FK convention. The
exporter was not changed.

| Check | Result |
|---|---:|
| Pre-export maximum full-basis error | 0.039565 degrees |
| Pre-export maximum runtime-world quaternion error | 0.039565 degrees |
| Runtime decoded local-channel error | 0.057652 degrees |
| Runtime static rotation drift | 0.00000296 degrees |
| Bind translation error | 0 |
| Runtime bone count | 68 |
| Runtime length / maximum sample frame | 1.0 s / 60 |
| Failed runtime checks | 0 |

The proof tolerance is 0.1 degrees. GetAnimation2 compiled and decoded
`MALE/COHSOURCEDEV_RETARGET_RESTBASIS_FRAME20` against `MALE/SKEL_READY2`.

- [Math proof JSON](rest-basis-frame20.math.json)
- [Decoded runtime proof JSON](runtime-proof.json)
- [Runtime identity and final capture log references](runtime-identity.txt)
- [Durable canary include](../../../agent/animation/canary-rest-basis-frame20.inc)
- [Durable compiled animation](../../../agent/animation/runtime/player_library/animations/male/COHSOURCEDEV_RETARGET_RESTBASIS_FRAME20.anim)

Compiled animation SHA-256:
`176B500AD8D5498059BA63B789D525A2BBBC6025B3820AE1E99D30925CA038C9`

## Actual-skin review set

These are final direct-DB captures using account `Dummy00009`, character
`Swingv3`, named character handoff, `seq_type=male`, `calculated_type=male`,
`is_male=1`, `TypeGfx=male`, and the new asset on
`COHSOURCEDEV_CUSTOM_CANARY`. They are ready for Josh's visual review; they
are not being labeled visual PASS here.

| View | Raw Mixamo 20 | Current retarget 20 | Corrected proxy 20 | Corrected actual CoH |
|---|---|---|---|---|
| Front | [raw](../issue36-orientation-20260824/visual/frame20/raw/front.png) | [current](../issue36-orientation-20260824/visual/frame20/current/front.png) | [rest-basis proxy](../issue36-orientation-20260824/visual/frame20/rest/front.png) | [actual](actual/FRAME20_front.jpg) |
| Three-quarter | [raw](../issue36-orientation-20260824/visual/frame20/raw/threequarter.png) | [current](../issue36-orientation-20260824/visual/frame20/current/threequarter.png) | [rest-basis proxy](../issue36-orientation-20260824/visual/frame20/rest/threequarter.png) | [actual](actual/FRAME20_threequarter.jpg) |
| Side | [raw](../issue36-orientation-20260824/visual/frame20/raw/side.png) | [current](../issue36-orientation-20260824/visual/frame20/current/side.png) | [rest-basis proxy](../issue36-orientation-20260824/visual/frame20/rest/side.png) | [actual](actual/FRAME20_side.jpg) |

The canary uses `Scale 0`, `Type Male`, and the existing custom canary state
and move. It does not replace `MALE/COHSOURCEDEV_RETARGET_SWING_FULL` and does
not modify `WEBSWING_BOTTOM`.

## Validation

- `agent/build.ps1 -Configuration Release -Platform x86` passed.
- `agent/smoke.ps1 -ExerciseCharacter -AccountName Dummy00009 -TimeoutSeconds 180 -Json` passed with MapServer entry and clean client exit.
- The three final captures passed with exit code 0 and the shard was left running with `RestBasisFrame20` staged for review.
