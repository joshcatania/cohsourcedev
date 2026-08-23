# Issue 36 — Mixamo swing phase runtime audition + Male-only HOLD (FINAL GATE) — VALIDATION 2026-08-23

**Branch:** `agent/issue-36-web-swing`
**Implementation checkpoint:** `589a6b94b1c11fd9b42b370ae5da4b3dd7faaf26` (MAXSTATES 882 -> 884, STATE_ARRAY_SIZE 28 -> 28)
**Proven asset (byte-identical):** `MALE/COHSOURCEDEV_RETARGET_SWING_FULL` 60 f @30 fps SHA256 `2a674b086d7fa916530002ead55451fdf28fd9780a9efee23205eed4b66d388e` (`swinginganimations/Swinging.fbx` — untracked)
**Control fallback:** `COHSOURCEDEV_WEBSWING_STRETCH_V2` `35b6da70…` (ATTACHED/DESCEND), `TUCK`/`ASCEND` V1 for Fem/Huge

This document **supersedes** the prior `issue36-webswing-phase-audition.md` proposal and the `dfd17ce1` final-gate draft. The `webswing.inc` correctly cycles Mixamo `30–40` only via helper `START/HOLD` (generic ASCEND remains V1 fallback). Prior updates removed the dead `ascend_enter pulse` diagnostic in `pmotion.c` and corrected the `runtime_statebits` interpretation. This validation confirms the MAXSTATES capacity fix.

## Correction 1 — dead diagnostic removed

`Common/player/pmotion.c` previously had an `else` branch:

```c
WEB_SWING %s ascend_enter pulse male=%d enter=%d phase=%s
```

inside `phase == stored_phase`. That `enter` bit is set only when `phase == ASCEND && stored != ASCEND` (pulse on entry), so the diagnostic could never fire. **Removed.** The existing phase-transition log already prints `male=%d enter=%d` via `TSTB(...)` and is the correct proof source. No change to ENTER-bit generation logic.

## Correction 2 — runtime_statebits interpretation

`WEB_SWING_ANIM runtime_statebits ... male=1 enter=1` prints `male_state_bit >=0` / `enter_state_bit >=0` — i.e. **resolved**, not active. Do **not** use it to prove Male activation or to claim Fem/Huge `male=0`. Prove active state from the normal phase-transition line: `WEB_SWING CLIENT anim_phase=ASCEND ... male=1 enter=1 ...` (uses `TSTB`).

## Correction 3 — selectedMove is CLIENT-only

`WEB_SWING ANIM selectedMove=...` in `Common/seq/seqsequence.c` is `#ifdef CLIENT`. Do not expect it in `bin/logs/mapserver/webswing.log`; use **Ouroboros GUI client** `bin/logs/game/webswing.log` or `bin/logs/client/webswing.log` via `agent/play-local.ps1 -WebSwingDev` (`Dummy00009`/`SwingV2` Male in Atlas).

## Implementation checkpoint

* **Implementation checkpoint:** `589a6b94b1c11fd9b42b370ae5da4b3dd7faaf26`
* **MAXSTATES:** `882 -> 884` (`Common/seq/TriggeredMove.h:4` `// 876 stock state slots plus up to eight WebSwingDev overlay bits`)
* **STATE_ARRAY_SIZE:** `28 -> 28` (`(882+31)/32=28`, `(884+31)/32=28`)
* **Build:** `Release|x86` **BUILD PASS 9.5s** `agent/logs/build-Release-x86-20260823-055828.log` (v145 fallback, `git show HEAD:Common/seq/TriggeredMove.h` contains `884`)
* **Files changed at checkpoint:** `Common/seq/TriggeredMove.h` only for this capacity fix. Prior design changes: `Common/player/pmotion.c`, `agent/webswing-animation/*`. No physics/anchors/steering.

## 1. Ranges auditioned (actual Male skin, normal scale)

Canary `COHSOURCEDEV_CUSTOM_CANARY` = `Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL start end Flags Cycle` via `agent/animation/canary-sequencer.inc` → `bin/data/sequencers/cohsourcedev_canary.inc`. Blender proxy proven runtime-identical (`maxTargetLocalTranslation 0`, `sourceToControl joint 9e-07`, § static gate `proof.report.json`).

Rendered `D:\temp\swing_phases\frame{01,05,10,15,20,25,30,35,40,45,50,55,60}.jpg` (85 mm Workbench, facing from shoulder) + `GetAnimation2 -runtime-rig` + one live canary install for `18–22` (`canaryAssetSha256 == tracked`).

| Candidate | Frames | First | Middle | Last | Loop if held > clip | Verdict |
|---|---|---|---|---|---|---|
| **ATTACH 1–8** | 1–8 | F01 up, legs trailing, extent 2.08 hipsZ 1.94 | F04 | F08 extent 1.97 | drift after 2 cycles, shoulder 8° | **MARGINAL** — keep V2 |
| **ATTACH 1–10** | 1–10 | as above | F05 | F10 extent 1.973 tucked 102° vs 20° start | larger snap | **REJECT vs 1–8** |
| **DESCEND 6–14** | 6–14 | stretch→tuck | F10 | F14 | overlaps | **= ATTACH (V2)** |
| **BOTTOM 18–22** | 18–22 | F18 tucked ~1.1 | **F20 most compact 1.063 hipsZ 1.97 knees 69°** | F22 compact | **CLEAN LOOP** — both ends tucked high-knee, 0° segment error in 28–32 proof neighbourhood | **PASS strong** |
| **BOTTOM 15–22** | 15–22 | F15 extent 1.35 | F18–20 | F22 | start less compact → snap Δ0.25 | **REJECT vs 18–22** |
| **ASCEND 30–40** | 30–40 | F30 outstretched 1.30 hipsZ 2.53 | F35 | **F40 wide 1.584 hipsZ 2.92** | **ONE-SHOT** — snap if cycled >0.33 s; as single play natural | **PASS as one-shot + hold @40** |
| **ASCEND 28–44** | 28–44 | F28 | F36 | F44 1.667 | more snap Δ0.36 | **REJECT vs 30–40** |
| **RELEASE 50–60** | 50–60 | F50 | F55 | F60 crouched wide | best raw loop but landing — **OUT OF SCOPE** | deferred |

All: no detached shoulders, no inverted elbows, no corkscrew, no knee inversion (knee plane `cross(thigh,shin)` shared).

**Artifacts:** `D:\temp\swing_phases\*.jpg`, `D:\temp\prove30\visual\**`, `docs/evidence/issue36-mixamo-retarget/*`, `agent/work/issue36-mixamo-runtimefk-v3-20260822/proof.report.json`.

## 2. Loop / hold semantics — shipped precedent (corrected)

Shipped `bin/data/sequencers/player.txt`:
* `Flags Cycle` → looping (`READY 1 60` Cycle, all `WEBSWING_*` Cycle).
* **No `Cycle`** → one-shot; sequencer advances past last frame to `move->raw.nextMove[0]` (`Common/seq/seqsequence.c`). If `NextMove` not authored, `seqload.c` defaults to move 0 / READY.
* `Move JUMPPRE 3 6` (no Cycle) → `JUMPUP 6 65` → `JUMPAPEX 65 66` chained via `Requires`/`Interrupts` and `Scale` — precedent for non-cycling chain.
* **Move-level `Scale 0`** is the correct freeze (Type-level Scale is overridden by `seqStep`/`seqClientStep` which read `move->scale` and `TypeGfx scale`).

Therefore **removing `Cycle` alone is wrong** for ASCEND — it would fall to READY, not hold at 40. Correct is `NextMove HOLD` + `Scale 0` HOLD.

## 3. Final Male-only mappings (installed)

**File:** `agent/webswing-animation/webswing.inc` → `bin/data/sequencers/cohsourcedev_webswing.inc` (`3a2bafa…` after correction)

*No change to Fem/Huge, physics, anchors, rope, steering, momentum, renderer, fingers, IK, release, body pitch, exporter.*

```inc
Move WEBSWING_ATTACHED  Type Male Anim MALE/COHSOURCEDEV_WEBSWING_STRETCH_V2 1 30  // V2 retained
Move WEBSWING_DESCEND   Type Male Anim MALE/COHSOURCEDEV_WEBSWING_STRETCH_V2 1 30  // V2 retained
Move WEBSWING_BOTTOM    Type Male Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL 18 22 // Cycle (loop-clean)
Move WEBSWING_ASCEND    Type Male Anim MALE/COHSOURCEDEV_WEBSWING_ASCEND 1 28      // generic fallback, Cycle, Fem/Huge unchanged

Move WEBSWING_ASCEND_MALE_START
	Interpolate 5  Priority 23  NextMove WEBSWING_ASCEND_MALE_HOLD
	Type Male Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL 30 40
	Member "<WEBSWING_MALE_ASCEND>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Interrupts "<WEBSWING_ANIM>", "<JUMPS>", "<FALL>", "<GROUNDMOVEALL>"
	Requires "WEBSWING_ATTACHED", "WEBSWING_ASCEND", "WEBSWING_MALE", "WEBSWING_ASCEND_MALE_ENTER"
	// NO Flags Cycle, NO ReqInputs, explicit NextMove, Priority 23 > generic 22

Move WEBSWING_ASCEND_MALE_HOLD
	Interpolate 5  Priority 23  Scale 0
	Type Male Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL 40 60
	Member "<WEBSWING_MALE_ASCEND>", "<DEATHIRQ>", "<HITIRQ>", "<REACTIRQ>", "<BLOCKIRQ>", "<BLOCK>", "<STUNMOVE>", "<ATTACKIRQ>", "<MOVEIRQ>"
	Requires "WEBSWING_ATTACHED", "WEBSWING_ASCEND", "WEBSWING_MALE"
	Flags Cycle
```

Isolation: START/HOLD use `WEBSWING_MALE_ASCEND` not `WEBSWING_ANIM` → generic ASCEND cannot interrupt them; `AIRBORNE/ATTACHED/DESCEND/BOTTOM` add `Interrupts "<WEBSWING_MALE_ASCEND>"` to interrupt HOLD cleanly.

## 4. State bits

**Files:** `agent/webswing-animation/webswing.statebits` and `webswing-canary.statebits` (canary includes normal include)

```
StateBit WEBSWING_AIRBORNE Predictable
StateBit WEBSWING_ATTACHED Predictable
StateBit WEBSWING_DESCEND Predictable
StateBit WEBSWING_BOTTOM Predictable
StateBit WEBSWING_ASCEND Predictable
StateBit WEBSWING_MALE Predictable
StateBit WEBSWING_ASCEND_MALE_ENTER Predictable
StateBit COHSOURCEDEV_ANIMCANARY Predictable  (canary only)
```

**Generation:** `Common/player/pmotion.c:pmotionSetWebSwingAnimState()` — `is_male = e && e->seq && e->seq->type && e->seq->type->seqTypeName && !stricmp(..., "Male")` (not `calculatedSeqTypeName`). After `phase = pmotionGetWebSwingAnimPhase(...)`, before `e->motion->web_swing_anim_phase = phase`: `WEBSWING_MALE` set when `is_male && phase != NONE`; `WEBSWING_ASCEND_MALE_ENTER` set when `is_male && phase == ASCEND && e->motion->web_swing_anim_phase != ASCEND` (one-update pulse). Both bits cleared each call via `seqSetState(...,0,...)` alongside the five phase bits. Diagnostics: `WEBSWING_ANIM runtime_statebits ... male=%d enter=%d` now correctly means *resolved* ( `>=0` ), while `WEB_SWING ... anim_phase=ASCEND ... male=1 enter=1` via `TSTB` proves *active*.

## 5. Runtime sequencer proof — VALIDATION 2026-08-23

*Build:* `Release|x86` **BUILD PASS 9.5s** `agent/logs/build-Release-x86-20260823-055828.log` (v145 fallback, checkpoint `589a6b94b`, `MAXSTATES 884`, `STATE_ARRAY_SIZE 28`), shard `ServerMonitor 20508` warm, `normalModeInstalled true` (WebSwingDev 7-bit overlay).

*GUI path (required for `selectedMove`):* `Dummy00009` / `SwingV2` Male in Atlas via `agent/play-local.ps1 -WebSwingDev` (normal integrated Web Swing, NOT `animcanary`). Client log `bin/logs/game/webswing.log` (CLIENT, `#ifdef CLIENT` in `seqsequence.c`) shows `selectedMove`; MapServer logs do **not** contain it.

**Validation 2026-08-23 results:**

* **Normal 7-bit WebSwingDev startup:** `agent/play-local.ps1 -WebSwingDev -AccountName Dummy00009` **PASS** — `Ouroboros PID 7244` (then `25796`, `34984` for Josh) survived 5s probe, no `play-client-launch` failure JSON created (last failure `20260823-051744.json` pre-fix), startup proceeded beyond `command-line.parse.complete`, `statebits_load` 7-bit overlay succeeded (`selected_source=COMPILED_OVERLAY`, moves airborne/attached/descend/bottom/ascend=1), GUI reached game.
* **8-bit canary capacity:** `-WebSwingDev -WebSwingCanary` **PASS** — `PID 33748` survived probe, `statebit index=882 COHSOURCEDEV_ANIMCANARY` loaded within `884` capacity, `move_compare COHSOURCEDEV_CUSTOM_CANARY requires 882` observed, no crash.
* **Regression of prior failure:** prior `2026-08-22T21:02:40Z` `exit -1 marker=command-line.parse.complete` no longer reproduces; root cause was `MAXSTATES 882` overflow for 8-bit overlay (now `884`).

**Expected excerpts after a swing reaching ASCEND (to be filled from `bin/logs/game/webswing.log` CLIENT after a successful GUI swing):**

```
WEB_SWING CLIENT anim_phase=ASCEND ... male=1 enter=1 ...  // A. phase transition into ASCEND
WEB_SWING ANIM selectedMove=WEBSWING_ASCEND_MALE_START     // B. START selected (Priority 23)
WEB_SWING ANIM selectedMove=WEBSWING_ASCEND_MALE_HOLD      // C. after 30-40 completes, NextMove
... HOLD remains frame 40 while ASCEND persists ...        // D/E hold, no 40→30 wrap, generic WEBSWING_ASCEND does not appear
... phase exit → AIRBORNE/ATTACHED/DESCEND/BOTTOM interrupts HOLD ...
... re-enter ASCEND → new enter=1 → START again            // F
```

*Current START/HOLD status:* **NOT YET OBSERVED** in client `webswing.log` — GUI now launches, but actual `anim_phase=ASCEND male=1 enter=1` / `selectedMove=WEBSWING_ASCEND_MALE_START/HOLD` requires in-game swing input (`Dummy00009`/`SwingV2` Male in Atlas). Code + sequencer proof remains Priority 23, NextMove, Scale 0. Direct-DB smoke `agent/smoke.ps1 -ExerciseCharacter Dummy00009` **PASS 8.6s** `TestClient exit 0` (`agent/logs/smoke-directdb-20260823-060553.json`).

## 6. Frame-40 HOLD proof

`HOLD Anim = 40 60, move-level Scale 0` → sequencer initializes at 40 and never advances (`seqStep` uses `move->scale`, `seqClientStep` uses `move->scale`). Practical gate: `selectedMove` proves `HOLD` entered, GUI video shows no `40→30` snap, held pose remains stable. No new instrumentation needed.

## 7. GUI visual gate — READY FOR JOSH

Capture normal-size Male `airborne→attach→descend→bottom (18–22)→ascend START (30–40)→hold @40→detach→airborne` via `play-local -WebSwingDev` + `Dummy00009`/`SwingV2`. Preserve MP4 + stills (Bottom athletic tuck, Ascend START/mid, HOLD). `/webswing 0` negative control must return to stock `AIR_MA_IRONKICK`.

*Current status 2026-08-23:* **GUI VISUAL GATE READY FOR JOSH** — client now survives startup (`PID 34984` left running for Josh, Atlas, `Dummy00009`), 7-bit and 8-bit overlays load, prior `exit -1 command-line.parse.complete` fixed by `MAXSTATES 884`. START/HOLD `selectedMove` and visual tuck/hold confirmation still require human in-game swing.

Bottom `18–22` is **candidate / proxy-pass** (Blender proxy `0°` segment error, no buzzing in 5-frame window, but GUI confirmation pending); if worse than V1 `TUCK`, **revert Male BOTTOM to `MALE/COHSOURCEDEV_WEBSWING_TUCK 1 24`**. Ascend `30–40→hold` must show smooth `F30` entry, readable extension, stable `F40`, clean detach.

## 8. Regression

* `agent/smoke.ps1 -ExerciseCharacter Dummy00009` **PASS 8.6s** `TestClient exit 0` (`20260823-060553`)
* `agent/smoke.ps1 -ExerciseCharacter Dummy00009` pre-validation also **PASS 1.7s** (`20260823-054057`)
* Normal WebSwingDev startup **PASS** (7-bit) and canary capacity **PASS** (8-bit, statebit 882) — previously failing path now passes.
* `agent/status.ps1` shard warm (`ServerMonitor 20508`, `MapServer 22400,34512`), `install-webswing-animation` parity: `normalModeInstalled true`, `animationAssets Valid`, client `PID 34984` playable.

## 9. Fem/Huge proof (static parsed)

```
generic WEBSWING_ASCEND: Male V1 1–28, Fem V1 1–28, Huge V1 1–28, Flags Cycle
helper moves require WEBSWING_MALE → cannot be eligible for Fem/Huge
WEBSWING_MALE not set for Fem/Huge (is_male false)
```

Do not claim `runtime_statebits male=0` for Fem/Huge (that diagnostic is resolved, not active).

## 10. Files changed

* **This validation:** `Common/seq/TriggeredMove.h` — `MAXSTATES 882 -> 884` (capacity fix only)
* Prior correction: `Common/player/pmotion.c` — dead `else` diagnostic removed, male/enter generation intact
* Prior correction: `agent/webswing-animation/webswing.inc` — generic ASCEND restored to V1 Male `1 28`, added `WEBSWING_ASCEND_MALE_START`/`HOLD` + `Interrupts "<WEBSWING_MALE_ASCEND>"` on AIRBORNE/ATTACHED/DESCEND/BOTTOM
* Prior correction: `agent/webswing-animation/webswing.statebits` — added `WEBSWING_MALE`, `WEBSWING_ASCEND_MALE_ENTER`
* Prior correction: `agent/webswing-animation/webswing-canary.statebits` — same
* `docs/evidence/issue36-webswing-final-gate.md` — this validation (checkpoint 589a6b94b)

No `swinginganimations/*.fbx`, no `blender_export_animx.py`, no `prove_mixamo_anatomical_pose.py`, no `.anim` bytes, no `webswing-animations.json`, no `entworldcoll.c` physics.

---

**Validation summary 2026-08-23 (checkpoint 589a6b94b):**
* MAXSTATES `882 -> 884` (STATE_ARRAY_SIZE `28 -> 28`) — **FIXED**
* Build `Release|x86` **PASS 9.5s** `build-Release-x86-20260823-055828.log`
* Normal 7-bit WebSwingDev startup **PASS** (`PID 34984` alive, no new failure JSON)
* 8-bit canary capacity **PASS** (`PID 33748`, statebit 882)
* Direct-DB smoke **PASS** (`060553`)
* START/HOLD `selectedMove` **NOT YET OBSERVED** — requires in-game swing input
* Visual gate **GUI VISUAL GATE READY FOR JOSH** — client left playable (`Dummy00009`/`SwingV2` Male Atlas, `PID 34984`)

**STOP FOR SOL/JOSH REVIEW** — do not merge PR #37.
