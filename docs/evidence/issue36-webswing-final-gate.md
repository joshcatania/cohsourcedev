# Issue 36 — Mixamo swing phase runtime audition + Male-only HOLD (FINAL GATE) — CORRECTION APPLIED

**Branch:** `agent/issue-36-web-swing` @ `3f6d8fd6e06cc2e213b586712fa58f28b9697bfd` → `HEAD` after this diagnostic cleanup
**Proven asset (byte-identical):** `MALE/COHSOURCEDEV_RETARGET_SWING_FULL` 60 f @30 fps SHA256 `2a674b086d7fa916530002ead55451fdf28fd9780a9efee23205eed4b66d388e` (`swinginganimations/Swinging.fbx` — untracked)
**Control fallback:** `COHSOURCEDEV_WEBSWING_STRETCH_V2` `35b6da70…` (ATTACHED/DESCEND), `TUCK`/`ASCEND` V1 for Fem/Huge

This document **supersedes** the prior `issue36-webswing-phase-audition.md` proposal and the `dfd17ce1` final-gate draft. The `webswing.inc` now correctly cycles Mixamo `30–40` only via helper `START/HOLD` (generic ASCEND remains V1 fallback). This update also **removes the dead `ascend_enter pulse` diagnostic** in `pmotion.c` and **corrects the `runtime_statebits` interpretation** per the new pre-flight.

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

## Starting / final

* **Starting SHA (this diagnostic cleanup):** `3f6d8fd6e06cc2e213b586712fa58f28b9697bfd`
* **Final SHA (after this commit):** `1a59c2d80bc5f8579be83df061adc6aac4337f9c` (pushed to `origin/agent/issue-36-web-swing`, PR #37 draft)
* **Files changed (per design):** `Common/player/pmotion.c` (dead diagnostic removed, male/enter generation intact), `agent/webswing-animation/webswing.inc`, `agent/webswing-animation/webswing.statebits`, `agent/webswing-animation/webswing-canary.statebits`, this evidence doc. No `.anim` bytes, no `swinginganimations/*.fbx`, no `blender_export_animx.py`, no `prove_mixamo_anatomical_pose.py`, no `webswing-animations.json`, no physics/rope/steering.

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

## 5. Runtime sequencer proof — PENDING GUI CLIENT

*Build:* `Release|x86` **BUILD PASS 37s** (`agent/logs/build-Release-x86-20260822-203026.log`) and **BUILD PASS 37s** after diagnostic cleanup (`agent/logs/build-Release-x86-20260822-203026.log` second build), shard `ServerMonitor 9900` warm, `normalModeInstalled true` (`include 3a2bafa…`, `stateBits 840bc0…`).

*GUI path (required for `selectedMove`):* `Dummy00009` / `SwingV2` Male in Atlas via `agent/play-local.ps1 -WebSwingDev` (normal integrated Web Swing, NOT `animcanary`). Client log `bin/logs/game/webswing.log` (CLIENT, `#ifdef CLIENT` in `seqsequence.c`) shows `selectedMove`; MapServer logs do **not** contain it.

**Expected excerpts after a swing reaching ASCEND (to be filled from `bin/logs/game/webswing.log` CLIENT after a successful GUI swing):**

```
WEB_SWING CLIENT anim_phase=ASCEND ... male=1 enter=1 ...  // A. phase transition into ASCEND
WEB_SWING ANIM selectedMove=WEBSWING_ASCEND_MALE_START     // B. START selected (Priority 23)
WEB_SWING ANIM selectedMove=WEBSWING_ASCEND_MALE_HOLD      // C. after 30-40 completes, NextMove
... HOLD remains frame 40 while ASCEND persists ...        // D/E hold, no 40→30 wrap, generic WEBSWING_ASCEND does not appear
... phase exit → AIRBORNE/ATTACHED/DESCEND/BOTTOM interrupts HOLD ...
... re-enter ASCEND → new enter=1 → START again            // F
```

*Status after rebuild:* `webswing-smoke` `Dummy00010` `300s` **PASS** `serverSelectedAnchors 734` (vs prior 16), all 6 steering buckets, `hardCorrectionCount 0` (`agent/logs/webswing-smoke-20260822-203249.json`). The smoke that previously timed out now passes — physics unchanged, new mapping does not destabilize. The smoke’s `animationPhases` was empty because `RequireAnimationPhases` not set; re-run with that flag plus a GUI swing will emit the 12-step sequence above. Until GUI `selectedMove` log is captured, the START/HOLD proof remains *code + sequencer* proof (Priority, NextMove, Scale 0) plus the smoke’s server-side phase log — **not yet OBSERVED**.

*Client launch diagnostics (new in `play-local.ps1`):* `WebSwingDev` GUI launch `Dummy00009` on `2026-08-22T21:02:40Z` exited during startup probe `exit code -1` at `marker=command-line.parse.complete`, `Web Swing runtime parity: PASS`, diagnostic `agent/logs/play-client-launch-20260822-205920.json` / `210240.json`. **Classification:** client-wide WebSwingDev startup failure, not Web Swing sequencer parse (parity PASS, `statebits_load overlay_complete data_count=7`). Bounded control without `-WebSwingDev` **succeeded** (`Ouroboros PID 5376` launched, `PLAY-COH CLIENT STARTUP` held). Both old (`0331ad7af` direct `30–40`) and new (`helper moves`) `webswing.inc` fail identically for WebSwingDev, so failure is not caused by `START/HOLD` syntax. Normal smoke `agent/smoke.ps1 -ExerciseCharacter Dummy00009` **PASS 8.3s** — shard healthy.

## 6. Frame-40 HOLD proof

`HOLD Anim = 40 60, move-level Scale 0` → sequencer initializes at 40 and never advances (`seqStep` uses `move->scale`, `seqClientStep` uses `move->scale`). Practical gate: `selectedMove` proves `HOLD` entered, GUI video shows no `40→30` snap, held pose remains stable. No new instrumentation needed.

## 7. GUI visual gate — REQUIRED (PENDING)

Capture normal-size Male `airborne→attach→descend→bottom (18–22)→ascend START (30–40)→hold @40→detach→airborne` via `play-local -WebSwingDev` + `Dummy00009`/`SwingV2`. Preserve MP4 + stills (Bottom athletic tuck, Ascend START/mid, HOLD). `/webswing 0` negative control must return to stock `AIR_MA_IRONKICK`.

*Current status:* GUI video **pending** due to `PLAY-COH CLIENT STARTUP FAILURE exit -1` at `command-line.parse.complete` (see `agent/logs/play-client-launch-20260822-210240.json`). The new diagnostics prove it is a **client-wide WebSwingDev startup failure** (normal client without `-WebSwingDev` launches, parity `PASS`, `statebits_load data_count=7`). Not caused by `START/HOLD` — old `webswing.inc` (`30–40` direct) fails identically.

Bottom `18–22` is **candidate / proxy-pass** (Blender proxy `0°` segment error, no buzzing in 5-frame window, but GUI confirmation pending); if worse than V1 `TUCK`, **revert Male BOTTOM to `MALE/COHSOURCEDEV_WEBSWING_TUCK 1 24`**. Ascend `30–40→hold` must show smooth `F30` entry, readable extension, stable `F40`, clean detach.

## 8. Regression

* `agent/smoke.ps1 -ExerciseCharacter Dummy00009` **PASS 8.3s** `TestClient exit 0`
* `webswing-smoke` `Dummy00010` **PASS** `734` selected anchors (post-correction), all steering buckets, `hardCorrection 0`, `maxRadial 0.2625` — constraint/smoothness and retained-momentum gates still pass. Prior `0-anchor` runs (e.g. `147.2s 0/5`) documented as harness variability, not animation — brief says do not change anchor physics if zero-anchor recurs.
* `agent/status.ps1` shard warm, `install-webswing-animation` parity: `normalModeInstalled true`, `animationAssets Valid`.

## 9. Fem/Huge proof (static parsed)

```
generic WEBSWING_ASCEND: Male V1 1–28, Fem V1 1–28, Huge V1 1–28, Flags Cycle
helper moves require WEBSWING_MALE → cannot be eligible for Fem/Huge
WEBSWING_MALE not set for Fem/Huge (is_male false)
```

Do not claim `runtime_statebits male=0` for Fem/Huge (that diagnostic is resolved, not active).

## 10. Files changed in this correction

* `Common/player/pmotion.c` — dead `else` diagnostic removed, male/enter generation intact
* `agent/webswing-animation/webswing.inc` — generic ASCEND restored to V1 Male `1 28`, added `WEBSWING_ASCEND_MALE_START`/`HOLD` + `Interrupts "<WEBSWING_MALE_ASCEND>"` on AIRBORNE/ATTACHED/DESCEND/BOTTOM
* `agent/webswing-animation/webswing.statebits` — added `WEBSWING_MALE`, `WEBSWING_ASCEND_MALE_ENTER`
* `agent/webswing-animation/webswing-canary.statebits` — same
* `docs/evidence/issue36-webswing-final-gate.md` — this correction (supersedes phase-audition proposal)

No `swinginganimations/*.fbx`, no `blender_export_animx.py`, no `prove_mixamo_anatomical_pose.py`, no `.anim` bytes, no `webswing-animations.json`, no `entworldcoll.c` physics.

---

**Final visual verdict (pending GUI):**
* Bottom `18–22` — **candidate / proxy-pass loop-clean** (Blender `0°` segment error, 5-frame window clean; GUI confirmation pending — revert to V1 `TUCK` if snap observed)
* Ascend `30–40→hold` — **designed one-shot + Scale 0 hold** (code + sequencer proof: Priority 23, NextMove, Scale 0; GUI `selectedMove` + video proof of no `40→30` wrap pending)

**Client launch diagnostics:** `WebSwingDev` `Dummy00009` → `PLAY-COH CLIENT STARTUP FAILURE exit -1 marker=command-line.parse.complete parity=PASS diagnostic=agent/logs/play-client-launch-20260822-210240.json`; normal launch without `WebSwingDev` **succeeded** (`PID 5376`). Classification: **B. WebSwingDev/runtime-data-specific startup failure, not animation** — both old and new `webswing.inc` fail identically. Normal smoke `8.3s PASS`, webswing smoke `734 anchors PASS` (physics intact).

**STOP FOR SOL/JOSH REVIEW** — do not merge PR #37.
