# Issue #36 — prototype web-swinging travel movement

Implemented the preflight design without using `MOVETYPE_WIRE`.

## Runtime behavior

- `/webswing 1` enables the mirrored web-swing control state; `/webswing 0` disables it.
- While enabled, a falling or jumping player probes a deterministic 21-ray fan around momentum/travel, forward fallback, side directions, and upper/lower elevations from chest height for a usable world collision anchor. Every probe uses the real world collision query; the search and rope limits share a 150 ft cap.
- The selected anchor uses momentum/travel and forward scoring with a forward fallback, then applies a smooth rope constraint that preserves tangent motion, W/A/D steering, forward tangent, and release momentum without hard projection or velocity reconstruction. Bounded correction metrics are logged at attach detach.
- The tether is rendered client-side exactly once per displayed frame for each attached player and does not mutate physics state.
- Attachment happens before `checkJump()`, and the rope constraint runs after the vertical physics/jump override and before candidate integration.

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

The review follow-up was validated on 2026-08-19 with the final `Release|x86` build on a warmed direct-DB shard. A fresh no-flight character (`Dummy00010`) exercised the deterministic Web Swing matrix:

- 13 anchor selections from the 21-probe fan, 13 attachments, 13 detachments, and 551 swing samples.
- Maximum observed detach speed was 4.500, demonstrating retained release momentum.
- 13 constraint summaries recorded 1,117 soft corrections, 6 radial-bias corrections, 0 hard corrections, maximum radial correction 0.0302, and maximum velocity-direction delta 0.5646.
- The headless TestClient has no renderer, so tether draw evidence remains a manual GUI checkpoint; the source path is once-per-displayed-frame and physics-free.

Machine-readable results are in `agent/logs/webswing-smoke-20260819-140254.json`; the corresponding server evidence is in `agent/logs/webswing-smoke-20260819-140254.server-webswing.log`.
