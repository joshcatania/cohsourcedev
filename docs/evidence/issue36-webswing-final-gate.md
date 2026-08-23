# Issue 36 — Mixamo swing phase runtime audition + Male-only HOLD (FINAL GATE)

**Branch:** `agent/issue-36-web-swing` @ `0331ad7af7f572d890d3559635c238f0cc7be64b` → `HEAD` after this correction
**Proven asset (byte-identical):** `MALE/COHSOURCEDEV_RETARGET_SWING_FULL` 60 f @30 fps SHA256 `2a674b086d7fa916530002ead55451fdf28fd9780a9efee23205eed4b66d388e` (`swinginganimations/Swinging.fbx` — untracked)
**Control fallback:** `COHSOURCEDEV_WEBSWING_STRETCH_V2` `35b6da70…` (ATTACHED/DESCEND), `TUCK`/`ASCEND` V1 for Fem/Huge

This document **supersedes** the prior `issue36-webswing-phase-audition.md` proposal. The prior `webswing.inc` directly cycled Mixamo `30–40` under `Flags Cycle` for generic ASCEND — that would snap and also affect Fem/Huge (Cycle belongs to `SeqMove`, not `Type`). The corrected design implements the pre-flighted `Scale 0` hold + `NextMove` + `WEBSWING_MALE`/`ENTER` bits.

## Starting / final

* **Starting SHA (expected):** `0331ad7af7f572d890d3559635c238f0cc7be64b`
* **Final SHA (after this commit):** to be filled by `git rev-parse HEAD` after push
* **Files expected to change (per design):** `Common/player/pmotion.c`, `agent/webswing-animation/webswing.inc`, `agent/webswing-animation/webswing.statebits`, `agent/webswing-animation/webswing-canary.statebits`, final evidence doc. No `.anim` bytes, no `swinginganimations/*.fbx`, no `blender_export_animx.py`, no `prove_mixamo_anatomical_pose.py`, no `webswing-animations.json`, no physics/rope/steering code.

## 1. Ranges auditioned (actual Male skin, normal scale)

Canary `COHSOURCEDEV_CUSTOM_CANARY` = `Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL start end Flags Cycle` via `agent/animation/canary-sequencer.inc` → `bin/data/sequencers/cohsourcedev_canary.inc`. Blender proxy proven runtime-identical (`maxTargetLocalTranslation 0`, `sourceToControl joint 9e-07`, § static gate `proof.report.json`).

Rendered `D:\temp\swing_phases\frame{01,05,10,15,20,25,30,35,40,45,50,55,60}.jpg` (85 mm Workbench, facing from shoulder) + `GetAnimation2 -runtime-rig` + one live canary install for `18–22` (`canaryAssetSha256 == tracked`).

| Candidate | Frames | First | Middle | Last | Loop if held > clip | Verdict |
|---|---|---|---|---|---|---|
| **ATTACH 1–8** | 1–8 | F01 up, legs trailing, extent 2.08 hipsZ 1.94 | F04 | F08 extent 1.97 | drift after 2 cycles, shoulder 8° | **MARGINAL** — keep V2 |
| **ATTACH 1–10** | 1–10 | as above | F05 | F10 extent 1.973 tucked 102° vs 20° start | larger snap | **REJECT vs 1–8** |
| **DESCEND 6–14** | 6–14 | stretch→tuck | F10 | F14 | overlaps | **= ATTACH (V2)** |
| **BOTTOM 18–22** | 18–22 | F18 tucked ~1.1 | **F20 most compact 1.063 hipsZ 1.97 knees 69°** | F22 compact | **CLEAN LOOP** — both ends tucked high-knee, 0° segment error in 28–32 proof neighbourhood, no detached shoulders, elbows 25–33° natural | **PASS strong** |
| **BOTTOM 15–22** | 15–22 | F15 extent 1.35 | F18–20 | F22 | start less compact → snap Δ0.25 | **REJECT vs 18–22** |
| **ASCEND 30–40** | 30–40 | F30 outstretched 1.30 hipsZ 2.53 | F35 | **F40 wide 1.584 hipsZ 2.92** | **ONE-SHOT** — snap if cycled >0.33 s; as single play natural | **PASS as one-shot + hold @40** |
| **ASCEND 28–44** | 28–44 | F28 | F36 | F44 1.667 | more snap Δ0.36 | **REJECT vs 30–40** |
| **RELEASE 50–60** | 50–60 | F50 | F55 | F60 crouched wide | best raw loop but landing — **OUT OF SCOPE** | deferred |

All: no detached shoulders, no inverted elbows, no corkscrew, no knee inversion (knee plane `cross(thigh,shin)` shared).

**Artifacts:** `D:\temp\swing_phases\*.jpg`, `D:\temp\prove30\visual\**`, `docs/evidence/issue36-mixamo-retarget/*`, `agent/work/issue36-mixamo-runtimefk-v3-20260822/proof.report.json`.

## 2. Loop / hold semantics — shipped precedent

Shipped `bin/data/sequencers/player.txt`:
* `Flags Cycle` → looping (`READY 1 60` Cycle, all `WEBSWING_*` Cycle).
* **No `Cycle`** → one-shot; sequencer advances past last frame to `move->raw.nextMove[0]` (`Common/seq/seqsequence.c`). If `NextMove` not authored, `seqload.c` defaults to move 0 / READY.
* `Move JUMPPRE 3 6` (no Cycle) → `JUMPUP 6 65` → `JUMPAPEX 65 66` chained via `Requires`/`Interrupts` and `Scale` — precedent for non-cycling chain.
* **Move-level `Scale 0`** is the correct freeze (Type-level Scale is overridden by `seqStep`/`seqClientStep` which read `move->scale` and `TypeGfx scale`).

Therefore **removing `Cycle` alone is wrong** for ASCEND — it would fall to READY, not hold at 40. Correct is `NextMove HOLD` + `Scale 0` HOLD.

## 3. Final Male-only mappings (installed)

**File:** `agent/webswing-animation/webswing.inc` → `bin/data/sequencers/cohsourcedev_webswing.inc` (`3a2bafa…` after correction, vs `ca6fdc…` before)

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

Isolation: START/HOLD use `WEBSWING_MALE_ASCEND` not `WEBSWING_ANIM`; generic ASCEND interrupts `WEBSWING_ANIM` → cannot interrupt helper. `AIRBORNE/ATTACHED/DESCEND/BOTTOM` add `Interrupts "<WEBSWING_MALE_ASCEND>"` so phase change interrupts HOLD cleanly. HOLD’s `Scale 0` is move-level (not Type).

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

**Generation:** `Common/player/pmotion.c:pmotionSetWebSwingAnimState()` — `is_male = e && e->seq && e->seq->type && e->seq->type->seqTypeName && !stricmp(..., "Male")` (not `calculatedSeqTypeName`). After `phase = pmotionGetWebSwingAnimPhase(...)`, before `e->motion->web_swing_anim_phase = phase`: `WEBSWING_MALE` set when `is_male && phase != NONE`; `WEBSWING_ASCEND_MALE_ENTER` set when `is_male && phase == ASCEND && e->motion->web_swing_anim_phase != ASCEND` (one-update pulse). Both bits cleared each call via `seqSetState(...,0,...)` alongside the five phase bits. Diagnostics extended: `WEBSWING_ANIM runtime_statebits ... male=%d enter=%d` and `WEB_SWING ... anim_phase=%s ... male=%d enter=%d`.

## 5. Runtime sequencer proof (12 steps) — status

*Build:* `Release|x86` via `agent/build.ps1` with v145 fallback — **BUILD PASS 73.4s** (`agent/logs/build-Release-x86-20260822-195840.log`), shard restarted `ServerMonitor 8952`, `normalModeInstalled true` (`includeSha256 3a2bafa…`, `stateBitsSha256 840bc09…` — matches tracked), animation assets `sourceValid/runtimeValid true`.

*Expected log sequence on Male ASCEND entry (to be captured in `bin/logs/mapserver/webswing.log` / `agent/logs/webswing-smoke-*.server-webswing.log`):*
1. `WEBSWING_ANIM runtime_statebits ... male=1 enter=1` resolved
2. `WEB_SWING SERVER anim_phase=ASCEND male=1 enter=1`
3. `selectedMove=WEBSWING_ASCEND_MALE_START` (Priority 23 beats generic 22 during ENTER pulse)
4. START `Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL 30 40` progresses 30→40 once
5. Natural non-cycle completion → `NextMove WEBSWING_ASCEND_MALE_HOLD`
6. `selectedMove=WEBSWING_ASCEND_MALE_HOLD` starting at frame 40
7. `Scale 0` holds frame 40 while `WEBSWING_ASCEND` + `WEBSWING_MALE` remain
8. No `40→30` wrap
9. Generic `WEBSWING_ASCEND` (Flags Cycle, `WEBSWING_ANIM` member) does **not** interrupt START/HOLD (different member)
10. On phase exit (`AIRBORNE`/`ATTACHED`/`DESCEND`/`BOTTOM`), those moves’ `Interrupts "<WEBSWING_MALE_ASCEND>"` interrupt HOLD cleanly
11. Re-enter `ASCEND` → new `ENTER=1` pulse → replay START
12. No `Scale 0` drift client/server (both use `move->scale`)

*Current status:* Code and sequencer installed; shard restarted. Latest `webswing-smoke` runs after the rebuild: `Dummy00010 -Timeout 180` timed out (no exit), `Dummy00010 -Timeout 240` previously `FAIL missing yaw90/forward` with `serverSelectedAnchors 16` (proof that physics/anchors still work). The `runtime_statebits male=1 enter=1` log has not yet appeared in `bin/logs/mapserver/webswing.log` tail because no successful ASCEND swing has been logged since the 20:08 restart (last tail is `player_seq` logs at 20:11:50). The next successful `webswing-smoke` that reaches `ASCEND` will emit the 12-step sequence; **that log capture is the gating proof for this correction** — to be attached as `agent/logs/webswing-smoke-*.server-webswing.log` with `male=1 enter=1` and `selectedMove=WEBSWING_ASCEND_MALE_START/HOLD`.

*Honest note:* Automated `webswing-smoke` currently hits the known zero-anchor / steering harness variability (prior Sol logs `172954.json`/`173414.json` also 0 anchors) — brief says *do not treat as animation failure* and do not change anchor physics. Use the proven Atlas `Dummy00009`/`SwingV2` GUI path for deterministic swing.

## 6. Fem/Huge unchanged

* `GetAnimation2 -runtime-rig MALE/COHSOURCEDEV_WEBSWING_ASCEND` etc. still `Scale 0` only on Male HOLD, not under `Type`. Fem/Huge `WEBSWING_ASCEND` still `MALE/COHSOURCEDEV_WEBSWING_ASCEND 1 28 Flags Cycle` in generic move, `WEBSWING_MALE` never set for them (`!stricmp` fails), helper moves require `WEBSWING_MALE` → cannot become eligible. Parsed `player.txt` after install still shows `COMPILED` vs `LLOOSE` selection via `player_seq` but `webswing.log` `runtime_statebits … male=0 enter=0` for Huge/Fem.

## 7. GUI visual gate

*Prior proven path:* `Dummy00009` / `SwingV2` Male, Atlas Plaza, `airborne→attach→descend→bottom→ascend→detach→airborne` already produced `airborne→attach` etc. in `issue-36-animation-pipeline.md`. Reuse that workflow with `agent/play-local.ps1 -WebSwingDev` (client `-animcanary 1` for canary, or normal Web Swing for integrated moves). Capture MP4 via `PLAY-COH.cmd` + OBS / `bin/screenshots` plus stills at `F18/F20/F22` (bottom) and `F30/F35/F40 hold` (ascend). `/webswing 0` negative control must return to stock locomotion (`WEBSWING_AIRBORNE` → `AIR_MA_IRONKICK`).

*This correction’s GUI video still pending* — bottom `18–22` already judged loop-clean in Blender proxy; ascend `30–40` START + `40 hold Scale 0` is designed to eliminate the `40→30` snap; the video must prove no snap, held F40 reasonable, clean detach.

## 8. Smoke / regression

* `agent/smoke.ps1 -ExerciseCharacter Dummy00009` → **PASS** `8.3s` `TestClient exit 0` `character creation and MapServer entry` (`agent/logs/smoke-directdb-20260822-202006.json`)
* `agent/webswing-smoke.ps1` — latest `Dummy00010` `300s` timed out, prior `Dummy00010 240s` had `serverSelectedAnchors 16` but `missing yaw90/forward` — steering harness variability, **zero hard corrections**, `softCorrections` nominal. The 0-anchor `Dummy00009` run (147.2s `serverSelectedAnchors 0`) is the known harness/environment issue, not a new regression from the Male helper moves (hard corrections `0` still). Retained momentum `detach speed>0.25` previously passed; to be re-proven on the next successful anchor run.

## 9. Build / installer parity

* Build: `Release|x86` `v145 fallback` PASS (73.4s)
* Installer: `install-webswing-animation.ps1 -Action Install` → `normalModeInstalled true`, `includeSha256 3a2bafa… == tracked`, `stateBitsSha256 840bc0… == tracked`, `animationAssetsSourceValid/runtimeValid true`, `legacyPlayerOverride false`.

## 10. Files changed in this correction

* `Common/player/pmotion.c` — Male/Enter bits generation + diagnostics
* `agent/webswing-animation/webswing.inc` — generic ASCEND restored to V1 Male `1 28`, added `WEBSWING_ASCEND_MALE_START` (30 40, NextMove HOLD, Priority 23) and `WEBSWING_ASCEND_MALE_HOLD` (40 60, Scale 0, Priority 23), added `Interrupts "<WEBSWING_MALE_ASCEND>"` to AIRBORNE/ATTACHED/DESCEND/BOTTOM
* `agent/webswing-animation/webswing.statebits` — added `WEBSWING_MALE`, `WEBSWING_ASCEND_MALE_ENTER`
* `agent/webswing-animation/webswing-canary.statebits` — same
* `docs/evidence/issue36-webswing-phase-audition.md` — superseded by this file (or `docs/evidence/issue36-webswing-final-gate.md` if split)
* No `swinginganimations/*.fbx`, no `blender_export_animx.py`, no `prove_mixamo_anatomical_pose.py`, no `.anim` bytes, no `webswing-animations.json`, no physics/rope/steering.

## 11. Confirmation

Physics, anchor selection, rope, steering, momentum, Fem/Huge assets, release animation, body pitch, fingers, IK, exporter math, renderer, `D:\github\coh-graphics` **untouched**.

---

**Visual verdict (pending final MP4):** Bottom `18–22` remains **PASS loop-clean**; Ascend `30–40` START + `40 hold` designed to be **PASS one-shot + hold**; V2 retained for ATTACHED/DESCEND. If actual GUI proves `18–22` snaps or `40 hold` looks wrong, revert that Male Bottom/Ascend to V1 and document — visual quality is the gate.

**STOP FOR SOL/JOSH REVIEW** — do not merge PR #37.
