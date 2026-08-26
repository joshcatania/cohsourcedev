# Issue #36 — prototype web-swinging travel movement

Implemented the preflight design without using `MOVETYPE_WIRE`.

## Runtime behavior

- `/webswing 1` enables the mirrored web-swing control state; `/webswing 0` disables it.
- `/webswingbackend 0` selects the preserved `REAL_ANCHOR` backend (the default); `/webswingbackend 1` selects the v1-candidate `SKY_ASSISTED` backend. `-webswingphysics 0|1` requests the same authoritative selection after a client reaches a map. Changing it while attached performs a momentum-preserving release before reacquisition.
- `REAL_ANCHOR` retains the deterministic 21-ray fan around momentum/travel, forward fallback, side directions, and upper/lower elevations from chest height for a usable world collision anchor. Every probe uses the real world collision query; the search and rope limits share a 150 ft cap.
- `SKY_ASSISTED` performs no collision query for acquisition. Its smoothed point 45 ft above and 28 ft ahead of travel intent is tether presentation only; it never constrains position or velocity.
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

## Josh final V1 scale pass (2026-08-25)

Josh confirmed the crash and ground-strike blockers fixed and accepted the corrected Male animation as sufficient for V1. The remaining feel request was a much more legible Spider-Man-like web cycle and a larger powered pendulum. The final bounded tuning pass animates each line itself: four ticks retract it back into the raised hand, the web remains fully absent for at least 18 ticks and preferably until the assisted apex, then five ticks shoot the replacement line outward to a new hidden-moved sky endpoint. Physics remains continuous throughout. Ground launch now begins at 3.20 vertical and 2.20 forward speed, the visual endpoint is 45 feet up and 28 feet ahead, descent/bottom speeds and the vertical recovery target scale more strongly with energy, energy builds to a 2.00 cap, horizontal speed is capped at 6.50, and total assisted velocity at 8.00. Terrain anticipation and ordinary collision are unchanged from the stable no-ground-strike candidate.

The strengthened final gate passed with 50 assisted cycles, 50 release/retraction starts, 50 completed retractions, 42 completed post-gap reattachments, and 62 completed outward shots including each scenario's initial attach. Every replacement attach observed at least the required 18-tick fully absent interval. Bottom speed reached the 8.000 cap, bottom horizontal speed averaged 4.079 versus 2.354 at apex, energy grew from 0.000 to 2.000, ground-start travel advanced 513.938 feet without regressing terrain anticipation, and release preserved up to 6.531 speed. Evidence: `agent/logs/webswing-smoke-20260825-171935.json`. `Release|x86` passed with v145 fallback in `agent/logs/build-Release-x86-20260825-171349.log`, the warmed direct-DB MapServer smoke passed in `agent/logs/smoke-directdb-20260825-171829.json`, and a fresh mode-3 `Swingv3` GUI client reached Steel Canyon with Web Swing auto-enabled, one bounded overlay load, no parser failure, and no immediate crash.

## V2 authored web-shot checkpoint (2026-08-25)

V2 freezes the accepted SKY_ASSISTED movement constants and gives the fictional web cycle an authored character beat. A bounded audit of all four supplied Mixamo clips identified frames 1..26 of `Start Swinging.fbx` as the strongest sequence: crouched recovery, wrist aim, arm extension, and catch. Those frames were transferred through the already-accepted rest-basis pipeline into `MALE/COHSOURCEDEV_WEBSWING_SHOOT_V2`; the runtime proof passes for all 68 bones and 26 frames with zero bind-translation drift and 0.0719713 degrees maximum packed rotation error.

The assisted visual state now exposes a dedicated predictable `WEBSWING_SHOOT` bit. Mode 3 uses it to start the wrist-fire move once per new visual attachment, independently of physics-phase changes. The line remains invisible through 12 authored time units of wind-up, then travels outward over approximately four units. While it travels, a brighter leading segment makes the web read as a projectile; retraction uses the same moving-head cue back toward the wrist. The renderer selects the raised animated hand during the invisible wind-up and locks that hand as soon as the line becomes visible. None of these presentation states constrains velocity, position, collision, steering, or the synthetic anchor.

The final `Release|x86` build passed in `agent/logs/build-Release-x86-20260825-180822.log`; warmed direct-DB character/MapServer entry passed in `agent/logs/smoke-directdb-20260825-181239.json`. The full Sky gate passed in `agent/logs/webswing-smoke-20260825-181326.json` with 46 assisted cycles, every controller phase, energy growth to 2.000, 49 completed visual shots, and all 49 animation-clock shot durations between 15.85 and 16.50. The remaining checkpoint is deliberately human: whether the combined arm recovery, wrist fire, traveling web head, retract, and phase handoff feel exceptional at normal gameplay speed.

The follow-up combo pass makes retraction authored rather than line-only. `COHSOURCEDEV_WEBSWING_RETRACT_V2` is the accepted 1..26 preparation motion in exact reverse order: the raised arm recoils as the old line collapses, reaches the compact ready pose during the invisible gap, and hands off seamlessly to the forward `SHOOT_V2` clip when the replacement line fires. Initial street activation now selects `COHSOURCEDEV_WEBSWING_GROUND_LAUNCH_V2`, the complete 62-frame `Start Swinging.fbx` performance, so the one-hand wrist shot continues into a two-hand catch and full-body pull instead of being replaced after the reach. Dedicated predictable `RETRACT` and `GROUND_LAUNCH` bits make those choices directly from assisted visual/controller state; ordinary physics phases cannot cut the choreography off, while release, fall, hit, death, block, stun, and attack transitions remain authoritative.

Both new assets passed pre-export and decoded-runtime proofs for all 68 bones with zero bind/translation drift. Maximum packed local rotation error was 0.0719713 degrees for retract and 0.0892473 degrees for ground launch. The transition checker passes 312 assertions, including the protected ground-launch window, retract-to-shoot handoff, and stock interruption paths. The final `Release|x86` build passed in `agent/logs/build-Release-x86-20260825-183328.log`; warmed direct-DB entry passed in `agent/logs/smoke-directdb-20260825-183702.json`. The named-character Sky gate passed on `Dummy00009 / Swingv3` in `agent/logs/webswing-smoke-20260825-184931.json`: 71 automatic cycles, all five phases, 71 release/retract starts, 70 completed retractions, 52 completed outward shots with valid animation-clock timing, energy growth to 2.000, 483.656 feet of ground-start travel, 967.234 feet in the forward scenario, 6.113 release speed, no hard correction, and no rope-constraint samples.

## V2 fluid-cycle and altitude-band correction (2026-08-25)

Josh's recorded Steel Canyon pass isolated the next objective failure. The complete ground launch visibly advanced and read well, but subsequent phase moves spent most of their controller time in `Scale 0` terminal HOLDs. The trace selected `WEBSWING_FULL_BOTTOM_HOLD` and `WEBSWING_FULL_ASCEND_HOLD` while position continued advancing; the video showed the same midair pose for several seconds. The corrected mode-3 phase windows remain the accepted ATTACH `1..8`, DESCEND `9..17`, BOTTOM `18..22`, and ASCEND `23..40`, but both START and HOLD now play each complete window at a phase-matched rate. Choreography fallback holds retain short authored motion tails. No Web Swing gameplay move freezes on a terminal frame, and the checker now rejects a zero-rate or single-frame HOLD.

The same pass exposed a movement issue from elevated starts: terrain avoidance was the only low-point trigger, so every assisted arc eventually normalized toward the surface below it. SKY_ASSISTED now snapshots an authored world-space low point from the fictional pivot height minus its displayed line length when attaching. That altitude band is not a rope constraint; it is an additional deterministic signal for beginning the bottom sweep, while current/ahead terrain remains the collision-safety floor. Each successful replacement web may raise the low point by a bounded energy-scaled amount but never lowers it; total gain is capped at 48 feet over the initial band so the mechanic cannot turn into unrestricted Flight. A rooftop or high-air attachment therefore keeps a high arc, while a street launch continues to be governed by normal terrain anticipation.

The supplied animation set is now treated by scenario: `Start Swinging.fbx` owns initial ground launch and wrist-fire/recovery material, `Swinging.fbx` owns the corrected repeated arc, `Swinging(1).fbx` is reserved as the strongest alternate compact arc candidate, and `Swing To Land.fbx` is reserved for an intentional landing transition. The latter two are not selected blindly or randomly before their own corrected-runtime assets and controller conditions are proven.

The final objective gate passed after a `Release|x86` build (`agent/logs/build-Release-x86-20260825-194433.log`) and warmed direct-DB MapServer entry (`agent/logs/smoke-directdb-20260825-194923.json`). The named-character Sky matrix (`agent/logs/webswing-smoke-20260825-195036.json`) completed with exit code 0: one verified ground boost, all five assisted phases, 53 automatic cycles, 52 completed retracts, 53 completed outward shots, 45 altitude-band bottom guards, 48 bounded band raises, bottom speed 8.000 versus upper average 3.719, preserved release speed up to 4.876, and no rope-constraint corrections. The transition checker passes 342 assertions. These are objective stability gates only; animation fluidity and elevated-swing feel remain a real-client human gate.

## V2 authored-cycle ownership and gravity-band follow-up (2026-08-25)

The next real-client pass showed that moving phase HOLD windows alone did not solve the visible statue. The client-selected-move trace in `bin/logs/game/webswing.log` showed stock sequencer moves entering between authored Web Swing windows: `COSTUME_SWORD_ZOOMED` replaced a descend START, `PROFILEREADY` entered between the authored shoot and phase motion, and `READY_XARMS_POST` appeared after ground launch. The server controller continued advancing phases and segment IDs throughout, so server-only phase evidence could not expose this presentation failure.

Each corrected phase START now owns and cycles its complete accepted authored window for as long as the controller reports that phase. It no longer falls through a `NextMove` boundary or depends on a one-tick entry pulse. Retract and shoot likewise cycle their complete authored clips while their choreography state remains active, and phase STARTs can resume directly when either requirement clears. The accepted 62-frame ground launch remains a protected one-shot: repeated phases cannot cut it off. The transition checker now parses `Requires`, `Flags`, and `NextMove` and passes 373 assertions, including persistent phase ownership, direct post-choreography recovery, stock interrupt safety, and ground-launch protection.

The high-air bottom was also lowered without weakening terrain safety. The authored altitude low point is now 36 feet below the visual pivot-derived line bottom, altitude braking uses its own stronger `0.90` stopping acceleration, per-cycle low-point gain is `1.5 + energy` feet, and total gain is capped at 24 feet. Current and ahead terrain probes remain the independent hard floor. In the two post-change named-character matrices, controller traversal completed with exit code 0, 50 automatic cycles, all altitude-band and energy checks, bottom speed at the 8.000 cap, bottom horizontal speed greater than apex speed, preserved release momentum, no rope constraints, and no crash. Across sampled elevated attachments, the character descended 12.53--35.63 feet before the altitude-band bottom turn (25.76 feet average), rather than turning almost immediately at attachment height. Those matrices reported a harness-only ground-scenario failure because the old Steel Canyon coordinate was not a street surface; TestClient now uses the verified real-client street coordinate `(-4668.25, 0.50, 1585.13)`. The last pre-change named-character matrix already proved the unchanged ground boost and launch path in `agent/logs/webswing-smoke-20260825-195036.json`.

The final full `Release|x86` build passed through the v145 fallback in `agent/logs/build-Release-x86-20260825-204653.log`. After the expected cold first-start timeout, the single warmed direct-DB character/MapServer smoke passed in `agent/logs/smoke-directdb-20260825-205604.json`. The remaining gates are deliberately human: confirm that no stock ready/statue pose enters a held swing and that the deeper elevated arc still feels gravitational at gameplay speed.

## Animation slice

- `pmotionSetState()` resolves the Web Swing phase and choreography bits by name, so missing animation data leaves the ordinary sequencer state unchanged. `REAL_ANCHOR` retains rope/velocity classification; `SKY_ASSISTED` maps the authored controller phase directly (`LAUNCH` attach, `ASCEND` ascend, `APEX` stable attached/dead-band pose, `DESCEND` descend, `BOTTOM` bottom) and maps its visual tether state directly to ground launch, retract/recovery, and wrist fire.
- The tracked loose data is in `agent/webswing-animation/`: its private state bits are `Predictable`, and fallback moves use verified stock player mappings for `Male`, `Huge`, and `Fem`: `SUPERJUMP`, `FLYDOWN`/`FLY_DOWN`, `FLY`, `HOVER`, `FLOAT2`, `FLOAT`, and `FLY_READY`. Critical/death/hit/react/block/stun/attack groups remain able to interrupt the Web Swing moves.
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
