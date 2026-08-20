# Issue #36 — prototype web-swinging travel movement

Implemented the preflight design without using `MOVETYPE_WIRE`.

## Runtime behavior

- `/webswing 1` enables the mirrored web-swing control state; `/webswing 0` disables it.
- While enabled, a falling or jumping player probes a deterministic 21-ray fan around momentum/travel, forward fallback, side directions, and upper/lower elevations from chest height for a usable world collision anchor. Every probe uses the real world collision query; the search and rope limits share a 150 ft cap.
- The selected anchor uses momentum/travel and forward scoring with a forward fallback, then applies a smooth rope constraint that preserves tangent motion, W/A/D steering, forward tangent, and release momentum without hard projection or velocity reconstruction. Bounded correction metrics are logged at attach detach.
- The tether is rendered client-side exactly once per displayed frame for each attached player and does not mutate physics state.
- Attachment happens before `checkJump()`, and the rope constraint runs after the vertical physics/jump override and before candidate integration.

## Animation slice

- `pmotionSetState()` now classifies the existing Web Swing state from attachment, rope geometry, tangential speed, and vertical velocity. It resolves `WEBSWING_AIRBORNE`, `WEBSWING_ATTACHED`, `WEBSWING_DESCEND`, `WEBSWING_BOTTOM`, and `WEBSWING_ASCEND` by name, so missing animation data leaves the ordinary sequencer state unchanged. The phase latch is animation-only; no rope solver or movement constant changed.
- The tracked loose data is in `agent/webswing-animation/`: all five state bits are `Predictable`, and the moves use verified stock player mappings for `Male`, `Huge`, and `Fem`: `SUPERJUMP`, `FLYDOWN`/`FLY_DOWN`, `FLY`, `HOVER`, `FLOAT2`, `FLOAT`, and `FLY_READY`. Critical/death/hit/react/block/stun/attack groups remain able to interrupt the Web Swing moves.
- Install or remove the local override with `agent/install-webswing-animation.ps1`. It is sentinel- and hash-guarded, preserves a protected copy of the resolved player source, and never edits a pigg. The checked-in local piggs expose compiled `sequencers.bin` rather than raw `sequencers/player.txt`; therefore the installer refuses to synthesize a player file unless an exact loose/pigg source or explicit runtime dump is available. The current workspace was installed from an exact runtime sequencer dump for the pending GUI gate, without committing that large dump.
- `agent/webswing-smoke.ps1 -RequireAnimationPhases` is the strict phase-evidence gate. The normal direct-DB shard intentionally starts MapServer with `-productionmode`, so its headless smoke uses the safe fallback and reports no phase lines; silhouette/transition judgment remains the requested GUI checkpoint in a loose-data development client.
- Post-change evidence: `agent/logs/build-Release-x86-20260819-194252.log` passed Release/x86; `agent/logs/jump-height-smoke-20260819-194832.json` passed with OFF 11.234 ft and ON 17.188 ft; and the final production-mode matrix `agent/logs/webswing-smoke-20260819-200225.json` passed with 15 selected anchors, 15 attach/detach pairs, retained release momentum, all six steering buckets, no hard corrections, and smoothness/divergence gates passing. The strict phase run `agent/logs/webswing-smoke-20260819-194935.json` records the expected separate limitation: `animationPhases=[]` under production mode.

## Required runtime evidence

Collected on 2026-08-19 on the locally verified Release/x86 shard in Atlas Park with the deterministic TestClient driver (`-webswing-smoke`). The driver exercised a normal ground jump, held UP/Space for airborne attach and tethering, released UP/Space, then held it again for reattachment, with forward/left/right tangent input and no OS keyboard automation. Durable server evidence is in:

`bin/logs/mapserver/webswing.log`

The observed sequence for TestClient PID 19932 includes:

```text
WEB_SWING mode=1
WEB_SWING attach anchor=(100.00 153.61 -621.97) rope=46.73 speed=0.352
WEB_SWING swing speed=0.840 rope=46.73 input=(-1.00 0.00 0.00)
WEB_SWING swing speed=1.337 rope=46.73 input=(1.00 0.00 0.00)
WEB_SWING detach speed=0.028 anchor=(100.00 153.61 -621.97) input=(0.00 0.00 0.20)
WEB_SWING attach anchor=(110.84 142.94 -604.34) rope=34.30 speed=0.345
WEB_SWING swing speed=1.123 rope=34.30 input=(1.00 0.00 0.00)
WEB_SWING swing speed=1.273 rope=34.30 input=(1.00 0.00 0.00)
WEB_SWING mode=0
WEB_SWING detach speed=0.119 anchor=(110.84 142.94 -604.34) input=(0.00 0.00 0.20)
```

The sampled input records show the tangent steering/pumping phases: `(-1, 0, 0)` and `(1, 0, 0)` for left/right steering, with forward held in the intervening samples. UP/Space release detached with nonzero retained speed (`0.028`, followed by `0.119` on the second swing), and the first attachment was followed by a new anchor attachment. The debug tether remains rendered from the attached entity to the selected anchor.

## Build

`Release|x86` passed with the repository’s v145 fallback on 2026-08-19. Full build output is recorded in:

`agent/logs/build-Release-x86-20260819-135745.log`

## Review follow-up validation

Sol review `4975850659` was addressed narrowly on 2026-08-19 from the existing Issue #36 head. The final locally verified `Release|x86` build passed with the v145 fallback:

`agent/logs/build-Release-x86-20260819-155450.log`

The fresh direct-DB application smoke passed after the normal shard warm-up and reached MapServer:

`agent/logs/smoke-directdb-20260819-155526.json`

The deterministic Web Swing matrix used `Dummy00012` / `TEST04223` (level 50, `Pool.Flight.Fly`, access level 9) and passed:

- 15 selected real collision anchors, 15 attachments, 15 detachments, and 601 swing samples.
- 9 selected anchors used measured horizontal momentum; divergent facing/travel acquisition passed with 4 approximately-45-degree attempts and 2 approximately-90-degree attempts. The final server log records `travel_right` as the normalized world-up cross-product of travel, sign-aligned to entity-right.
- Retained release momentum passed with maximum detach speed 3.128; yaw 0/90 forward, left, and right steering evidence all passed.
- No hard corrections; 9,139 attached constraint samples; average velocity-direction delta 0.001199; maximum 0.6982; 6 samples (0.0657%) at or above the 0.300 direction-delta threshold; maximum consecutive large-delta run 1.
- Outward radial velocity removal occurred on 666 samples (7.2874%), average removed magnitude 0.181244, maximum 3.2224; 67 samples (0.7331%) were at or above the 0.250 radial-velocity threshold.
- The smoke gate allows an isolated entry-transition discontinuity but fails repeated held-swing evidence at more than 3 consecutive large direction deltas or more than 12.5% large-delta samples. This run passed, so the existing solver was preserved.

Machine-readable results are in `agent/logs/webswing-smoke-20260819-160030.json`; the corresponding server evidence is in `agent/logs/webswing-smoke-20260819-160030.server-webswing.log`.

The jump-height smoke also passed at the preserved `WEB_LAUNCH_JUMP_HEIGHT_SCALE=2.50`: OFF 11.203 ft, ON 17.188 ft, ratio 1.534x, matching the measured approximately-1.536x launch baseline. Evidence is in `agent/logs/jump-height-smoke-20260819-155931.json`. The launch diagnostic uses the hidden TestClient-only `webswing_test_no_attach` control bit, synchronized through ServerControlState on both client and server; normal `/webswing 0` and `/webswing 1` behavior is unchanged.

Final constants: 150 ft anchor/search and rope cap; 6 ft minimum anchor height; 8 ft minimum rope; 2 ft probe start height; 0.75 ft rope slop; 0.35 bias gain; 0.75 ft/s bias cap; 4.50 ft/s maximum swing speed; 21 deterministic probes; 0.300 direction-delta threshold; 0.250 radial-velocity threshold.

The client tether path remains once per displayed frame and physics-free. Headless TestClient has no renderer, so tether pixels remain a manual GUI checkpoint for Josh.
