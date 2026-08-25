# Issue #36 — prototype web-swinging travel movement

Implemented the preflight design without using `MOVETYPE_WIRE`.

## Runtime behavior

- `/webswing 1` enables the mirrored web-swing control state; `/webswing 0` disables it.
- `/webswingbackend 0` selects the preserved `REAL_ANCHOR` backend (the default); `/webswingbackend 1` selects the v1-candidate `SKY_ASSISTED` backend. `-webswingphysics 0|1` requests the same authoritative selection after a client reaches a map. Changing it while attached performs a momentum-preserving release before reacquisition.
- `REAL_ANCHOR` retains the deterministic 21-ray fan around momentum/travel, forward fallback, side directions, and upper/lower elevations from chest height for a usable world collision anchor. Every probe uses the real world collision query; the search and rope limits share a 150 ft cap.
- `SKY_ASSISTED` performs no collision query for acquisition. Its smoothed point 35 ft above and 20 ft ahead of travel intent is tether presentation only; it never constrains position or velocity.
- `SKY_ASSISTED` runs an authored `LAUNCH -> ASCEND -> APEX -> DESCEND -> BOTTOM` controller. Ground starts drive up and forward immediately, ascent preserves forward travel, the apex turns over briefly, descent builds horizontal and downward speed, and a deterministic downward terrain probe begins the upward sweep before collision. A completed bottom sweep starts the next cycle automatically while Space remains held.
- Assisted steering blends horizontal velocity toward world-space travel intent. Authority is strongest near the apex and weakest at the fast bottom. Release applies no boost or reconstruction, and airborne reattach selects a phase from existing vertical velocity.
- `REAL_ANCHOR` alone applies the smooth rope constraint and its tangent/radial correction metrics.
- The tether is rendered client-side exactly once per displayed frame for each attached player and does not mutate physics state.
- Attachment happens before `checkJump()`. The selected backend runs after ordinary vertical physics and before candidate integration, so normal world/building collision remains active.

## SKY_ASSISTED A/B checkpoint (2026-08-25, historical)

The experiment starts from preserved real-anchor checkpoint `82f063dbb2ae4ec25609969f3779de6622b43683` plus its direct descendant `ad61982306e7140a3db18a0fac292bd96b7aa791`, which keeps the private animation overlay client-only. The SKY backend is additive: it does not replace the real collision search, shared pendulum solver, steering, release behavior, animation phase/segment IDs, or mode-3 Male animation work.

The deterministic matrix now accepts `-Backend RealAnchor|SkyAssisted`, an explicit `-CharacterName`, rotated Web Swing logs, and a 420-second bound matching the current roughly-five-minute pose matrix. Ground-origin evidence follows the intentional `ground_launch_begin ... catch_suppressed=1` path instead of requiring an airborne `attach_catch` record.

Both backends completed naturally on the warmed Release/x86 direct-DB shard with `Dummy00012 / TEST04223`:

- `REAL_ANCHOR`: 256 backend-selected anchors, 4 automatic handoffs, 12 nonzero-capable detach samples (maximum 4.500), all six steering buckets, all authored physics phases, 0 hard corrections, and 0.0698 maximum velocity-direction delta. The real search continued to report 21 probes. Evidence: `agent/logs/webswing-smoke-20260825-095251.json` and `.server-webswing.log`.
- `SKY_ASSISTED`: 380 backend-selected anchors, all 380 with `probes=0 ray_hits=0`, 148 automatic handoffs, 14 detach samples (maximum 3.712), all six steering buckets, all authored physics phases, 0 hard corrections, and 0.2523 maximum velocity-direction delta (below the 0.300 large-delta threshold). The ground scenario gained 85.469 ft altitude and 173.066 ft forward displacement in replicated TestClient state. Evidence: `agent/logs/webswing-smoke-20260825-095820.json` and `.server-webswing.log`.

These runs justified keeping the selectable backend and geometry-free acquisition. The initial SKY experiment still used the shared pendulum solver and is superseded by the authored controller below.

The final post-validation `Release|x86` rebuild passed with the v145 fallback in `agent/logs/build-Release-x86-20260825-100629.log`.

## SKY_ASSISTED authored-controller candidate (2026-08-25)

The v1 candidate branches before the real-anchor constraint. Its controller phase is stored in `MotionState` on both prediction paths and drives the existing animation phase builder directly; no Sky animation phase is inferred from tether geometry. A new bottom-to-ascent cycle advances the existing animation segment ID intentionally.

The first warmed `Dummy00009 / Swingv3` objective run completed naturally with 98 automatic assisted cycles, all five controller phases, seven meaningfully redirected input headings, release samples up to the 4.500 movement cap, 574.094 ft of ground-start forward displacement, and 126.953 ft in the separate forward scenario. Bottom samples peaked at 4.428 versus a 2.968 average across ascent/apex samples. The terrain anticipation samples normally began the upward conversion with several feet of clearance and produced no rope-constraint samples. Evidence: `agent/logs/webswing-smoke-20260825-112525.json` and `.server-webswing.log`.

That JSON reports failure only because it predates the checker update and still expected pendulum-specific `swing`, `chain_handoff`, and tangent-steering lines. The updated Sky gate consumes `assisted_tick`, `assisted_phase`, and `assisted_cycle`, requires all phases and multiple cycles, checks bottom-speed emphasis, geometry-free acquisition, multiple steering headings, forward displacement, and preserved release momentum. Human gameplay remains the authority for whether the resulting cadence is fun.

The rebuilt updated gate passed on `Dummy00009 / Swingv3`: 1,508 assisted tick samples, 107 automatic cycles, all five phases, nine redirected input headings, bottom peak speed 4.393 versus 3.076 upper-phase average, 574.422 ft ground-start forward displacement, 721.453 ft forward-scenario displacement, release speed up to 4.500, zero rope-constraint samples, and continued animation-state production. Evidence: `agent/logs/webswing-smoke-20260825-113911.json` and `.server-webswing.log`. The corresponding final `Release|x86` build passed with the v145 fallback in `agent/logs/build-Release-x86-20260825-113442.log`.

## Josh feel pass: pendulum rhythm, energy growth, and hand tether (2026-08-25)

Josh's first normal-gameplay pass confirmed useful forward traversal but described the cadence as a constant-speed up/down motion rather than a swing. He also requested a readable web from the raised hand and a stronger initial ground jump. The bounded V1 feel pass keeps the authored `SKY_ASSISTED` architecture and changes only its shaping: one ground impulse, slower horizontal travel through ascent/apex, stronger acceleration through descent/bottom, and a per-cycle energy value that raises both bottom speed and the following upward sweep until a finite cap. Release still applies no force and preserves the exact velocity at that instant; airborne reattachment derives a bounded starting energy from existing horizontal momentum.

The client tether now selects the higher animated `HANDR`/`HANDL` bone each displayed frame, with the previous root-plus-height origin retained only as a missing-bone fallback. A blue-white outer line plus opaque white core keeps it readable without coupling the visual endpoint to physics.

The strengthened objective gate requires the ground impulse, energy growth, and bottom-versus-apex horizontal-speed contrast in addition to the existing cadence, phase, steering, collision, release, and geometry-independence checks. It passed on `Dummy00009 / Swingv3` with 72 automatic cycles, two ground-boost observations, energy growth from 0.000 to the 1.500 cap, 3.739 average bottom horizontal speed versus 2.289 at apex, 5.750 maximum release speed, all five phases, and zero rope-constraint samples. Evidence: `agent/logs/webswing-smoke-20260825-125800.json` and `.server-webswing.log`. `Release|x86` passed with the v145 fallback in `agent/logs/build-Release-x86-20260825-125637.log`. The rendered hand-origin tether remains a manual GUI checkpoint because TestClient is headless.

## Josh feel pass: web cadence, terrain anticipation, and crash hardening (2026-08-25)

Josh's next normal-gameplay pass exposed one P0 and two P1 issues: the client died after a long mode-3 session, the visual web remained continuously attached, and a descent could meet rising terrain before the bottom sweep recovered. The crash window ended during an authored move transition after `player.txt` overlay selection had run 24,472 times and semantic comparison had run 20,117 times, producing a 110 MB log. The overlay builder used a single-entry cache keyed by a compiled pointer; normal switching among player variants repeatedly replaced that slot and leaked a deep copy of the 8,000-plus-move sequencer. V1 now keeps a bounded per-source overlay cache, blocks recursive construction, and logs overlay eligibility/selection only once. The tether renderer also validates the current pose-tree root and hand-node identities before reading their matrices.

Continuous SKY_ASSISTED physics now has a deliberately separate visual web cadence. At every completed bottom sweep the old line disappears for seven simulation ticks, its fictional endpoint is repositioned while invisible, and a fresh raised-hand line appears for the next arc. Release/re-fire changes no velocity, phase, or collision state. Terrain anticipation samples current, half-ahead, and full-ahead clearance over a 32-foot vertical probe, using the lowest result and a conservative stopping distance to begin the upward sweep before the character reaches pavement or rising ground.

An explicit `-webswingphysics 0|1` client handoff now also enables Web Swing through the existing predicted server-command channel after map entry. This is scoped to explicit development/test launches and removes the remaining manual `/webswing 1` setup step.

The strengthened Sky gate passed with 65 completed cycles, 65 visual releases, 62 completed visual reattachments (the remaining releases ended with the test scenario), all forward-clearance probe fields present, 3.977 average bottom horizontal speed versus 2.342 at apex, energy growth from 0.000 to 1.500, 509.562 feet of ground-scenario forward travel, and a 5.750 maximum momentum-preserving detach. Evidence: `agent/logs/webswing-smoke-20260825-144351.json`. The final `Release|x86` build passed via the v145 fallback in `agent/logs/build-Release-x86-20260825-145851.log`; its warmed direct-DB client/map smoke passed in `agent/logs/smoke-directdb-20260825-150248.json`. A fresh mode-3 GUI client then reached Steel Canyon, auto-enabled Web Swing, remained responsive, selected the overlay once, emitted one bounded semantics dump, and showed no parser failure or immediate crash.

## Animation slice

- `pmotionSetState()` resolves `WEBSWING_AIRBORNE`, `WEBSWING_ATTACHED`, `WEBSWING_DESCEND`, `WEBSWING_BOTTOM`, and `WEBSWING_ASCEND` by name, so missing animation data leaves the ordinary sequencer state unchanged. `REAL_ANCHOR` retains rope/velocity classification; `SKY_ASSISTED` maps the authored controller phase directly (`LAUNCH` attach, `ASCEND` ascend, `APEX` stable attached/dead-band pose, `DESCEND` descend, `BOTTOM` bottom).
- The tracked loose data is in `agent/webswing-animation/`: all five state bits are `Predictable`, and the moves use verified stock player mappings for `Male`, `Huge`, and `Fem`: `SUPERJUMP`, `FLYDOWN`/`FLY_DOWN`, `FLY`, `HOVER`, `FLOAT2`, `FLOAT`, and `FLY_READY`. Critical/death/hit/react/block/stun/attack groups remain able to interrupt the Web Swing moves.
- Install or remove the local override with `agent/install-webswing-animation.ps1`. It is sentinel- and hash-guarded, preserves a protected copy of the resolved player source, and never edits a pigg. The checked-in local piggs expose compiled `sequencers.bin` rather than raw `sequencers/player.txt`; therefore the installer refuses to synthesize a player file unless an exact loose/pigg source or explicit runtime dump is available. The current workspace was installed from an exact runtime sequencer dump for the pending GUI gate, without committing that large dump.
- The runtime `player.txt` used for the GUI gate is ParserWriteText output rather than canonical authored source: each `Move` contains unnamed `SeqMoveRaw` bin-time records after its last authored field (often `Flags`, but `Flags` is omitted when defaulted) and before `MEnd` (index/counts, next/cycle arrays, required bits, and member/interrupt bit arrays). `agent/install-webswing-animation.ps1` recognizes only that exact post-authored-field/pre-`MEnd` numeric-record region, removes those derived records, preserves every authored field, and fails on any unexpected content. The resulting loose file is parsed by the unmodified normal tokenizer callback; no parser errors are suppressed.
- `agent/webswing-smoke.ps1 -RequireAnimationPhases` is the strict phase-evidence gate. The normal direct-DB shard intentionally starts MapServer with `-productionmode`, so its headless smoke uses the safe fallback and reports no phase lines; silhouette/transition judgment remains the requested GUI checkpoint in a loose-data development client.
- Post-change evidence: `agent/logs/build-Release-x86-20260819-194252.log` passed Release/x86; `agent/logs/jump-height-smoke-20260819-194832.json` passed with OFF 11.234 ft and ON 17.188 ft; and the final production-mode matrix `agent/logs/webswing-smoke-20260819-200225.json` passed with 15 selected anchors, 15 attach/detach pairs, retained release momentum, all six steering buckets, no hard corrections, and smoothness/divergence gates passing. The strict phase run `agent/logs/webswing-smoke-20260819-194935.json` records the expected separate limitation: `animationPhases=[]` under production mode.
- Review follow-up evidence (2026-08-21): Release/x86 rebuilt successfully (`agent/logs/build-client-Release-x86-20260821-115015.log`). A fresh WebSwingDev GUI client logged `normal_dev_eligible=0 webswing_player_eligible=1 dev_eligible=1`, strict `dev_parse=PASS suppressed_parser_errors=0`, the resolved loose path, consumed include, all five statebits, and all five moves; the normal negative client logged `dev_eligible=0` and `selected_source=COMPILED`. Launcher mode switching restarted only Ouroboros (PIDs 28764, 28448, 32516, 18324); ServerMonitor/DbServer/Launcher and the existing MapServer PIDs/start times remained unchanged. Runtime include/state-bit hashes matched tracked hashes. Actual GUI swing input and resulting `selectedMove=WEBSWING_*` remain a manual gameplay gate; no desktop input automation was used.

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
