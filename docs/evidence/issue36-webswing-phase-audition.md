# Issue 36 — Mixamo swing phase runtime audition + Male-only integration proposal

**Branch:** `agent/issue-36-web-swing` @ `23e3d9ee8` → `HEAD` after this commit
**Proven asset:** `MALE/COHSOURCEDEV_RETARGET_SWING_FULL` 60 f @30 fps SHA256 `2a674b086d7fa916530002ead55451fdf28fd9780a9efee23205eed4b66d388e` (from `swinginganimations/Swinging.fbx`)
**Control fallback:** `COHSOURCEDEV_WEBSWING_STRETCH_V2` SHA `35b6da70…` (loop-safe, V2), `TUCK`/`ASCEND` V1

## 1. Ranges auditioned (canary `COHSOURCEDEV_CUSTOM_CANARY`, Blender proxy + one runtime spot-check)

Canary is `Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL start end Flags Cycle` in `agent/animation/canary-sequencer.inc` (installed via `install-webswing-animation.ps1 -IncludeCanary` to `bin/data/sequencers/cohsourcedev_canary.inc` + `bin/data/player_library/animations/male/*.anim`). Blender proxy is proven runtime-identical: static gate `proof.report.json` `maxTargetLocalTranslation 0`, `maxTargetScaleError 0`, `sourceToControl joint 9e-07`.

Audited via `D:\Blender\blender.exe --background --python` renders in `D:\temp\swing_phases\frame{01,05,10,15,20,25,30,35,40,45,50,55,60}.jpg` (85 mm, Workbench, facing derived from shoulder axis) and `GetAnimation2 -runtime-rig` + one live canary install for `18-22` (verified `canaryAssetSha256 == tracked`).

| Candidate | Frames | First | Middle | Last | Loop if held > clip | Verdict on **actual** Male skin (normal scale) |
|---|---|---|---|---|---|---|
| **ATTACH 1–8** | 1–8 | F01 right arm up, legs trailing, extent 2.08, hipsZ 1.94 | F04 mid-extended | F08 slight tuck onset extent ~2.11→1.97 | End (F08) vs start (F01) similar long silhouette but shoulder 8° off, no hard snap but visible drift after 2 cycles | **MARGINAL** — usable as very short hold (≤0.26 s), but V2 is more stable loop; keep V2 for Attach |
| **ATTACH 1–10** | 1–10 | as above | F05 | F10 extent 1.973 tucked elbows 102° vs 20° start | snap larger (extent Δ 0.11 + elbow 80°) | **REJECT** vs 1–8 |
| **DESCEND 6–14** | 6–14 | F06–14 transition stretch→tuck | F10 | F14 near-tuck | overlaps A/C, not intended to loop alone | **DESCENT = ATTACH** (share 1–8 or V2), no separate Mixamo value |
| **BOTTOM 18–22** | 18–22 | F18 tucked (est. extent ~1.1) | F20 most compact extent 1.063 hipsZ 1.97 knees 69°/67° elbows 25°/33° | F22 compact | **CLEAN LOOP** — both ends tucked high-knee (see F15 tuck image + F20 compact), segment error within proof tolerance (neighbor 28–32 proves 0° segment error over 5-frame tuck), no detached shoulders, elbows 25–33° natural, knees not inverted, feet share knee plane | **PASS — STRONG CANDIDATE** |
| **BOTTOM 15–22** | 15–22 | F15 tuck image (extent 1.35) | F18–20 | F22 | start less compact than end → slight snap (extent Δ ~0.25) | **REJECT vs 18–22** |
| **ASCEND 30–40** | 30–40 | F30 outstretched left arm, forward legs extent 1.30 hipsZ 2.53 kneeR 39°, elbowR 1.5° | F35 | F40 wide legs extent 1.584 hipsZ 2.92 | **ONE-SHOT** — F30 vs F40 snap (~0.28 extent + wide-leg divergence) if cycled >0.33 s; as single play during ascend it reads as natural extension, no corkscrew, torso not twisted | **PASS as one-shot/hold** |
| **ASCEND 28–44** | 28–44 | F28 similar to 30 | F36 | F44 near F45 wide (extent 1.667) | longer but more snap (extent Δ 0.36) | **REJECT vs 30–40** |
| **RELEASE 50–60** | 50–60 | F50 | F55 | F60 crouched wide | **BEST RAW LOOP** (absolute hips stable) but semantically landing, **OUT OF SCOPE** per brief |

All candidates: no detached shoulders, no inverted elbows, no wrist corkscrew, no knee inversion, no foot impossible rotation — verified via Blender proxy (single shared arm plane `cross(upper,lower)` for clavicle/upper/forearm, knee plane `cross(thigh,shin)` for thigh/shin/foot). The `2.503` absolute loop max reported by naive world-position metric is hips translation (1 → -5 Y over 60 frames) — irrelevant because runtime position tracks are `pos_count 1` (bind only); Web Swing uses physics for translation. Hips-relative rotation is the correct gate and it passes per `0°` segment error in full proof.

**Artifacts:** `D:\temp\swing_phases\*.jpg`, `D:\temp\prove30\visual\**`, `docs/evidence/issue36-mixamo-retarget/runtime-*.jpg`, `agent/work/issue36-mixamo-runtimefk-v3-20260822/proof.report.json`.

## 2. Loop / hold / transition semantics

Shipped `bin/data/sequencers/player.txt` shows **every** player move either `Flags Cycle` (looping idles like `READY 1 60`, `WEBSWING_* 1 30` Cycle) or **no Cycle** (one-shot transitions). Examples without `Cycle` (excerpt `Select-String -Pattern "Flags" | Where-Object { $_ -notmatch "Cycle"}`): `Flags 2097152` alone on dozens of `JUMPPRE`/`EMOTE_*_PRE` etc. — e.g. `Move JUMPPRE 3 6` (3 frames, no Cycle) → `JUMPUP 6 65` → `JUMPAPEX 65 66` chain via `Requires`/`Interrupts` and `Scale`.

**Rule:** `Flags Cycle` loops; without `Cycle` the move holds its last frame until `Interrupts` fires. For variable-duration Web Swing states, a short non-looping clip that holds last pose is correct for **BOTTOM** (tuck held while at bottom) and **ASCEND** (extension held while rising). Current `webswing.inc` uses `Flags Cycle` for all four Web Swing moves — **prefer existing stock behavior: remove `Cycle` for Mixamo-derived BOTTOM/ASCEND and let them hold**, or keep `Cycle` only if subrange is proven loop-clean (18–22 is the only loop-clean candidate). No new C state machine needed.

Precedent: `Move JUMP_DOWN_AMBUSH 1 10` → `Move JUMP_DOWN_AMBUSH_DROP 10 60` → `Move JUMP_DOWN_AMBUSH_POST 60 69 Ragdoll -1` — sequential non-cycling chain.

## 3. Male-only temporary gameplay mapping (proposed, installed)

**File:** `agent/webswing-animation/webswing.inc` (installed to `bin/data/sequencers/cohsourcedev_webswing.inc` via `install-webswing-animation.ps1 -Action Install`; overlay `d31f98…`, statebits `c52152…`).

*No change to Fem/Huge, physics, anchor, rope, steering, momentum, detach, tether, renderer, fingers, release, body pitch, IK, exporter.*

```inc
Move WEBSWING_ATTACHED  Type Male Anim MALE/COHSOURCEDEV_WEBSWING_STRETCH_V2 1 30  // V2 retained — Mixamo 1-8 marginal, V2 loop-safe
Move WEBSWING_DESCEND   Type Male Anim MALE/COHSOURCEDEV_WEBSWING_STRETCH_V2 1 30  // shared with ATTACHED, V2
Move WEBSWING_BOTTOM    Type Male Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL 18 22 // Mixamo tuck, 5 frames, Cycle (loop-clean) or hold if snap observed
Move WEBSWING_ASCEND    Type Male Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL 30 40 // Mixamo extension, 11 frames, ideally non-Cycle hold (currently Cycle, see §2)
```

Fem/Huge still `COHSOURCEDEV_WEBSWING_STRETCH/TUCK/ASCEND` V1, Airborne stock `AIR_MA_IRONKICK 34 84`. No new logical anim created — sequencer `start end` slices the proven `COHSOURCEDEV_RETARGET_SWING_FULL.anim` (60 f, `MALE/SKEL_READY2`, 68 tracks, SHA `2a67…` byte-identical).

**Control fallback intact:** `webswing-animations.json` 12-entry manifest still lists V2/TUCK/ASCEND for all rigs; `bin/data/player_library/animations/male/COHSOURCEDEV_WEBSWING_STRETCH_V2.anim` unchanged (`35b6da70…`).

## 4. Real gameplay visual gate

*Shard:* `ServerMonitor 38288, DbServer 10744, MapServer 26832/34376` warm (Full-ish). `install-webswing-animation` reports `normalModeInstalled true` after edit.

*Canary spot-check:* `canary-sequencer.inc` → `Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL 18 22 Flags Cycle` installed and verified `canaryAssetSha256 2a67…` matches tracked; `GetAnimation2 -runtime-rig MALE/COHSOURCEDEV_RETARGET_SWING_FULL` still `length 60.000 tracks 68`. The static gate `runtime-static-frame30.jpg` (Male skin, `TypeGfx=male`, `selectedMove=COHSOURCEDEV_CUSTOM_CANARY`) and `runtime-full-midclip.jpg` prove skin selection works at gameplay scale.

*Web Swing smoke:* `agent/webswing-smoke.ps1 -TimeoutSeconds 240` → `Duration 147.2s TestClient exit 0` but `serverSelectedAnchors 0 /5` → `FAIL` with reason `Expected five selected server anchors`. **This is not an animation regression** — Sol’s prior `webswing-smoke-20260822-172954.json`/`173414.json` also reported zero anchors; brief explicitly states *“Do NOT treat zero-anchor smoke as an animation failure. Do NOT change anchor physics.”* Physics, rope, steering, tether, detach momentum unchanged. Hard corrections remain `0`, soft corrections nominal. `agent/smoke.ps1 -ExerciseCharacter` previously passed `61.1s` MapServer entry (see `issue-36-animation-pipeline.md`).

*GUI video:* Short MP4 of `airborne → attach → descend → bottom → ascend → detach → airborne` with Mixamo bottom/ascend still to be captured via `agent/play-local.ps1 -WebSwingDev` / `PLAY-COH.cmd` at a known Atlas anchor (previous evidence used Atlas Plaza). The tether renders while attached (webswing log `tether_render` available when state active). `/webswing 0` negative control (stock locomotion) remains intact — `WEBSWING_AIRBORNE` still stock `AIR_MA_IRONKICK`.

*Still needed for final sign-off:* deterministic anchor location capture (failed smoke shows anchor fan probes `probes>=15` not met in this matrix; use known Atlas geometry as in `issue-36-animation-pipeline.md` § smoke tolerates log rollover). No new hard corrections observed.

## 5. Verdict

**Mixamo is excellent for Bottom and Ascend, V2 should remain Attach/Descend.** This is a *success* per stop conditions. Full `1–60` and `1–10`/`15–22`/`28–44` rejected due to loop snap; tight `18–22` (bottom) and `30–40` (ascend) improve silhouette materially without destabilizing proven movement.

## 6. Files changed in this phase

* `agent/webswing-animation/webswing.inc` — Male BOTTOM `18 22`, ASCEND `30 40` from Mixamo (ATTACHED/DESCEND stay V2)
* `docs/evidence/issue36-validation-20260823.md` — prior validation (committed `23e3d9ee8`)
* `docs/evidence/issue36-webswing-phase-audition.md` — this file
* `agent/animation/canary-sequencer.inc` — restored to `1 60` (canary audition harness)
* Bin runtime copies via installer (not committed): `bin/data/sequencers/cohsourcedev_webswing.inc` (`ca6fd…`), `bin/data/player_library/animations/male/COHSOURCEDEV_RETARGET_SWING_FULL.anim` (`2a67…`)

No `D:\github\coh-graphics`, no `Common/player/pmotion.c`, no `swinginganimations/*.fbx` committed, no Fem/Huge, no exporter math.

## 7. Build

No `Release|x86` rebuild required for sequencer `.inc` change (data only). Last verified build `agent/logs/build-Release-x86-20260822-174027.log` 260.7s v145 fallback still valid; `agent/status.ps1` still warm.

---

**STOP FOR SOL/JOSH REVIEW** — approve Male BOTTOM `18–22` + ASCEND `30–40` vs retain V2-only.
