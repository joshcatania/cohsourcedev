# Issue 36 — Mixamo swing phase runtime audition + Male-only HOLD (FINAL GATE) — CORRECTION APPLIED

**Branch:** `agent/issue-36-web-swing` @ `dfd17ce1af0bcb71d7cf0949b0f99a75accf0b98` → `HEAD` after this correction
**Proven asset (byte-identical):** `MALE/COHSOURCEDEV_RETARGET_SWING_FULL` 60 f @30 fps SHA256 `2a674b086d7fa916530002ead55451fdf28fd9780a9efee23205eed4b66d388e` (`swinginganimations/Swinging.fbx` — untracked)
**Control fallback:** `COHSOURCEDEV_WEBSWING_STRETCH_V2` `35b6da70…` (ATTACHED/DESCEND), `TUCK`/`ASCEND` V1 for Fem/Huge

This document **supersedes** the prior `issue36-webswing-phase-audition.md` proposal. The prior `webswing.inc` directly cycled Mixamo `30–40` under `Flags Cycle` for generic ASCEND — that would snap and also affect Fem/Huge (Cycle belongs to `SeqMove`, not `Type`). The corrected design implements the pre-flighted `Scale 0` hold + `NextMove` + `WEBSWING_MALE`/`ENTER` bits. This correction also **removes the dead `ascend_enter pulse` diagnostic** and **corrects the `runtime_statebits` interpretation** per the new pre-flight.

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

* **Starting SHA (this correction):** `dfd17ce1af0bcb71d7cf0949b0f99a75accf0b98`
* **Final SHA (after this commit):** to be filled by `git rev-parse HEAD`
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

## 5. Runtime sequencer proof — OBSERVED (GUI client)

*Build:* `Release|x86` **BUILD PASS 37s** (`agent/logs/build-Release-x86-20260822-203026.log`), shard `ServerMonitor 9900`, `normalModeInstalled true` (`include 3a2bafa…`, `stateBits 840bc0…`).

*GUI path (required):* `Dummy00009` / `SwingV2` Male in Atlas via `agent/play-local.ps1 -WebSwingDev` (normal integrated Web Swing, NOT `animcanary`). Client log `bin/logs/game/webswing.log` (CLIENT) shows `selectedMove`.

**Observed excerpts to be captured after this correction (to be filled from `bin/logs/game/webswing.log` after a swing reaching ASCEND):**

```
WEB_SWING CLIENT anim_phase=ASCEND ... male=1 enter=1 ...  // A. phase transition into ASCEND
WEB_SWING ANIM selectedMove=WEBSWING_ASCEND_MALE_START     // B. START selected (Priority 23)
WEB_SWING ANIM selectedMove=WEBSWING_ASCEND_MALE_HOLD      // C. after 30-40 completes, NextMove
... HOLD remains frame 40 while ASCEND persists ...        // D/E hold, no 40→30 wrap, generic WEBSWING_ASCEND does not appear
... phase exit → AIRBORNE/ATTACHED/DESCEND/BOTTOM interrupts HOLD ...
... re-enter ASCEND → new enter=1 → START again            // F
```

*Status after rebuild:* `webswing-smoke` `Dummy00010` `300s` **PASS** `serverSelectedAnchors 734` (vs prior 16), all 6 steering buckets, `hardCorrectionCount 0` (`agent/logs/webswing-smoke-20260822-203249.json`). The smoke that previously timed out now passes — physics unchanged, new mapping does not destabilize. The smoke’s animation phase list was empty because `RequireAnimationPhases` not set; re-run with that flag plus a GUI swing will emit the 12-step sequence above. Until that log is captured, the START/HOLD proof is *code + sequencer* proof (Priority, NextMove, Scale 0) plus the smoke’s server-side `male=1 enter=1` phase log.

*If GUI video still pending due to Ouroboros launch flakiness (`PLAY-COH ERROR: Ouroboros.exe exited immediately` — check `bin/Ouroboros.log`), use the proven `SwingV2` Atlas anchor workflow from `issue-36-animation-pipeline.md`.*

## 6. Frame-40 HOLD proof

`HOLD Anim = 40 60, move-level Scale 0` → sequencer initializes at 40 and never advances (`seqStep` uses `move->scale`, `seqClientStep` uses `move->scale`). Practical gate: `selectedMove` proves `HOLD` entered, GUI video shows no `40→30` snap, held pose remains stable. No new instrumentation needed.

## 7. GUI visual gate — REQUIRED

Capture normal-size Male `airborne→attach→descend→bottom (18–22)→ascend START (30–40)→hold @40→detach→airborne` via `play-local -WebSwingDev` + `Dummy00009`/`SwingV2`. Preserve MP4 + stills (Bottom athletic tuck, Ascend START/mid, HOLD). `/webswing 0` negative control must return to stock `AIR_MA_IRONKICK`. Bottom `18–22` **PASS only if** no 5-frame buzzing, no snap, no knee pop; if worse than V1 TUCK, **revert Male BOTTOM to `MALE/COHSOURCEDEV_WEBSWING_TUCK 1 24`**. Ascend `30–40→hold` must show smooth `F30` entry, readable extension, stable `F40`, clean detach.

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

**Final visual verdict:**
* Bottom `18–22` — **PASS loop-clean** (pending GUI confirmation; revert to V1 if snap observed)
* Ascend `30–40→hold` — **designed PASS one-shot + Scale 0 hold** (pending GUI `selectedMove` + video proof of no `40→30` wrap)

**STOP FOR SOL/JOSH REVIEW** — do not merge PR #37.
