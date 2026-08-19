# Issue #36 — prototype web-swinging travel movement

Implemented the preflight design without using `MOVETYPE_WIRE`.

## Runtime behavior

- `/webswing 1` enables the mirrored web-swing control state; `/webswing 0` disables it.
- While enabled, a falling or jumping player probes five forward/upward directions from chest height for a usable world collision anchor.
- The selected anchor is constrained by a 12–70 ft rope, horizontal input pumps the tangent, and speed is capped at 3.25.
- The tether is rendered client-side while attached.
- Attachment happens before `checkJump()`, and the rope constraint runs after the vertical physics/jump override and before candidate integration.

## Required runtime evidence

Collected on 2026-08-19 on the locally verified Release/x86 shard in Atlas Park with the deterministic TestClient driver (`-webswing-smoke`). The driver exercised a normal ground jump, airborne attach, forward/left/right tangent input, release, and reattachment without OS keyboard automation. Durable server evidence is in:

`bin/logs/mapserver/webswing.log`

The observed sequence for TestClient PID 29880 includes:

```text
WEB_SWING mode=1
WEB_SWING attach anchor=(100.00 154.76 -623.84) rope=54.84 speed=1.000
WEB_SWING swing speed=1.078 rope=54.84 input=(-1.00 0.00 0.00)
WEB_SWING swing speed=1.243 rope=54.84 input=(0.76 0.00 0.00)
WEB_SWING mode=0
WEB_SWING detach speed=0.003 anchor=(100.00 154.76 -623.84)
WEB_SWING mode=1
WEB_SWING attach anchor=(121.78 142.33 -601.55) rope=30.54 speed=0.480
WEB_SWING swing speed=1.100 rope=30.54 input=(1.00 0.00 0.00)
WEB_SWING swing speed=1.193 rope=30.54 input=(1.00 0.00 0.00)
WEB_SWING mode=0
WEB_SWING detach speed=0.271 anchor=(121.78 142.33 -601.55)
```

The sampled input records show the tangent steering/pumping phases: `(-1, 0, 0)` and `(1, 0, 0)` for left/right steering, with forward held in the intervening samples. The second release retained nonzero momentum (`0.271`), and the first attachment was followed by a new anchor attachment. The debug tether remains rendered from the attached entity to the selected anchor.

## Build

`Release|x86` passed with the repository’s v145 fallback on 2026-08-19. Full build output is recorded in:

`agent/logs/build-Release-x86-20260819-054001.log`
