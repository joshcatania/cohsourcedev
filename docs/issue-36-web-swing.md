# Issue #36 — prototype web-swinging travel movement

Implemented the preflight design without using `MOVETYPE_WIRE`.

## Runtime behavior

- `/webswing 1` enables the mirrored web-swing control state; `/webswing 0` disables it.
- While enabled, a falling or jumping player probes five forward/upward directions from chest height for a usable world collision anchor.
- The selected anchor is constrained by a 12–70 ft rope, horizontal input pumps the tangent, and speed is capped at 3.25.
- The tether is rendered client-side while attached.
- Attachment happens before `checkJump()`, and the rope constraint runs after the vertical physics/jump override and before candidate integration.

## Required runtime evidence

Collected on the locally verified Release/x86 shard with the deterministic TestClient driver (`-webswing-smoke`). Durable server evidence is in:

`bin/logs/mapserver/webswing.log`

The observed sequence includes:

```text
WEB_SWING mode=1
WEB_SWING attach anchor=(100.00 157.90 -629.94) rope=42.88 speed=0.050
WEB_SWING swing speed=0.617 rope=42.88
WEB_SWING swing speed=0.825 rope=42.88
WEB_SWING mode=0
WEB_SWING detach speed=0.287 anchor=(100.00 157.90 -629.94)
WEB_SWING mode=1
WEB_SWING attach anchor=(100.00 148.06 -614.67) rope=37.13 speed=0.480
WEB_SWING swing speed=0.797 rope=37.13
WEB_SWING mode=0
WEB_SWING detach speed=0.149 anchor=(100.00 148.06 -614.67)
```

This proves anchor selection, rope attachment, nonzero swing motion, nonzero-speed detachment, and reattachment.

## Build

`Release|x86` passed with the repository’s v145 fallback. Full build output is recorded in:

`agent/logs/build-Release-x86-20260818-215716.log`
