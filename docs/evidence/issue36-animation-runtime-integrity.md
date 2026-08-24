# Issue 36 — Web Swing animation runtime-integrity forensic pass — 2026-08-23

**Branch:** `agent/issue-36-web-swing`
**Expected starting HEAD:** `19eb739a28ac8b3b0d09733edfd476279027957a`
**Implementation at this checkpoint:** the committed source and evidence changes at the checkpoint SHA below
**PR #37:** remains stacked draft — not merged, not rebased

This document is the **focused evidence file** required by the forensic
animation-integrity pass. It records Josh's real-GUI failure, the old
three-generation runtime contamination, the temporary safe-baseline,
the deterministic Mixamo/runtime-FK A/B on the actual `TypeGfx=male` skin,
the numeric static-vs-full comparison, the stock/bind cross-check,
the exact-loaded-asset proof, the decision-tree result, and the
single-phase `WEBSWING_BOTTOM` reintegration gate.

---

## Josh GUI failure summary

Josh's real GUI footage (post-MAXSTATES-884 fix) showed:

* the character **severely mangled during the current swing**;
* the head/neck visibly separate/stretch — a classic
  parent→child translation or FK-order failure;
* when `/webswing 1` was enabled but the character was **unattached**
  (airborne), the character repeatedly played the **old weird kicking
  animation** — a stock `AIR_MA_IRONKICK` cycle unrelated to web swinging.

The current swing under test was therefore **not a clean test** of the new
Mixamo/Blender/runtime-FK asset.

## SOL preflight — three animation generations co-resident in the runtime

Inspection of the installed overlay `agent/webswing-animation/webswing.inc`
at the expected HEAD confirms exactly three generations were selectable at
once:

| Phase (classifier `pmotion.c:308`) | Male anim referenced | Generation |
|---|---|---|
| `WEBSWING_AIRBORNE` | `MALE/AIR_MA_IRONKICK 34 84` `Flags Cycle` | Stock slice — explains the weird kicking when unattached |
| `WEBSWING_ATTACHED` / `WEBSWING_DESCEND` Male | `MALE/COHSOURCEDEV_WEBSWING_STRETCH_V2 1 30` | **Old V2** — already had skeletal-quality failures; see `docs/issue-36-webswing-v2.md`, SHA `35b6da70…` |
| `WEBSWING_BOTTOM` and `WEBSWING_ASCEND_MALE_START/HOLD` Male | `MALE/COHSOURCEDEV_RETARGET_SWING_FULL` (bottom `18 22`, ascend `30 40` + hold `40 60 Scale 0`) | **New Mixamo runtime-FK** — the asset this pass must judge |

Therefore the horrific footage is **contaminated**. AIR_MA_IRONKICK and old V2
STRETCH are confirmed contaminants of Josh's failed footage. Static frame 30
clears the new runtime-FK representation at that pose, but BOTTOM 18..22 and
the full temporal clip remain visually unproven; ALL observed mangling cannot
yet be attributed conclusively to the old assets.

---

## Phase 1 — Safe gameplay baseline (animation-neutral, traversal preserved)

**Goal:** ordinary `/webswing 1` gameplay must *never* select the five
experimental visual moves, while every other subsystem stays intact.

### What changed

| File | Change | Purpose |
|---|:---|:---|
| `Common/cmdparse/cmdcommon.h:324` / `Common/cmdparse/cmdcommon.c:31,153` | new global `S32 g_cohsourcedev_webswing_anim_selection` (default `0`) and console command `{9,"webswinganim",…}` (access 9, hidden) | mirrors the proven `-animcanary` plumbing so the forensic baseline is reversible for gated canary work without touching data files |
| `Game/src/game.c:268` | argv `-webswinganim 0|1|2` bypass (`game_startupTracef("webswinganim.argument=%d")`) | same reason — the normal parser rejects access-9 commands before login |
| `Common/player/pmotion.c:380` | `pmotionSetWebSwingAnimState()` now **always** resolves the five `WEBSWING_*` state bits, computes `phase = pmotionGetWebSwingAnimPhase()` and emits every diagnostic (`runtime_statebits`, `CLIENT_STATE_BUILD`, `WEB_SWING … anim_phase=…`), but **suppresses all** `seqSetState(…,1,…)` calls unless selection is enabled; transition `e->motion->web_swing_anim_phase` still advances so the classifier's behaviour is observable without rendering | preserves the classifier/overlay/diagnostics while letting the stock player sequencer own the pose |
| `Common/seq/seqload.c:1226` | `seqLogWebSwingPlayerData()` now reports `overlay_moves_present=N/5` per move instead of `include_consumed = airborne && attached && descend && bottom && ascend` | during the forensic pass the runtime overlay intentionally carries an arbitrary subset; requiring all five would misreport a healthy reduced overlay as broken |

Preserved unchanged: all `entworldcoll.c` physics, anchors, rope solver,
steering, launch jump, tether, state classifier, `Predictable` state-bit
declarations, overlay architecture (`sequencers/cohsourcedev_webswing.txt`
`+ cohsourcedev_webswing.inc` deep-copied via `StructCopy(ParseSeqInfo,…)` in
`seqBuildWebSwingPlayerInfo()`), `START/HOLD` source, custom runtime
`.anim` assets, installer manifest.

The explicit developer canary path
`MALE/COHSOURCEDEV_RETARGET_*` via `COHSOURCEDEV_CUSTOM_CANARY`
(`Requires COHSOURCEDEV_ANIMCANARY`, injected in
`Game/src/entity/entclient.c:2436` under `-animcanary 1` / `-webswingdev`)
is **unaffected** by the baseline gate.

### Validation

```
WEB_SWING CLIENT anim_selection_mode=0 custom_move_selection=SAFE_NONE
WEB_SWING CLIENT anim_phase=NONE … statebits airborne=0 attached=0 descend=0 bottom=0 ascend=0 …
```

appears once per client invocation after the state-bit resolution header:

```
WEBSWING_ANIM statebits mode=WEBSWINGDEV airborne=1 attached=1 descend=1 bottom=1 ascend=1 total=883
```

The phase-transition log still fires (so the harness can require
`AIRBORNE`, `DESCEND`, `BOTTOM`/`ASCEND` if desired) but `TSTB(state, bit)`
for every `WEBSWING_*` bit is `0`.  `WEB_SWING ANIM selectedMove=WEBSWING_*`
**never appears** for ordinary gameplay (it does appear for the canary —
see Phase 5).  `/webswing 0` idle/run/jump remain stock-normal; `/webswing 1`
unattached no longer repeats `AIR_MA_IRONKICK`; rope/steering/launch/tether
are unchanged (soft/hard constraint corrections stay within the established
`agent/webswing-smoke.ps1` limits — smoke **PASS**, see below).

`seqLogWebSwingPlayerData` now logs honestly:

```
WEBSWING_ANIM player_seq selected_source=COMPILED_OVERLAY resolved_path=sequencers/cohsourcedev_webswing.txt overlay_moves_present=5/5 airborne=1 …
```

---

## Phase 2 — Static runtime-FK A/B (deterministic canary auditions)

No real swinging is required. The existing `COHSOURCEDEV_CUSTOM_CANARY`
canary overlay is used; both auditions are frozen with the **proven**
`Scale 0` sequencer freeze semantics (see `Common/seq/seqsequence.c:1417`
`anim->frame += move->scale * timestep` and `seqSetMove()` `anim->frame =
first_frame` — with `Scale 0` the frame never advances past the subrange's
`firstFrame`):

| Label | Canary include (`agent/animation/*.inc` → `bin/data/sequencers/cohsourcedev_canary.inc`) | `Anim` line | Freeze | Hash at capture |
|---|---|---|---|---|
| **A — STATIC_PROOF** | `canary-static-proof.inc` | `MALE/COHSOURCEDEV_RETARGET_POSE_PROOF 1 60` `Scale 0` `Flags Cycle` | proof pose (constant on authored samples `1…60`) | `bc806eeeb66d46c80adaed7e2f3052c07c7ff64c3a7c648ac6902c6c2a61df36` |
| **B — FULL_FRAME30** | `canary-full-frame30.inc` | `MALE/COHSOURCEDEV_RETARGET_SWING_FULL 30 31` `Scale 0` `Flags Cycle` | runtime frame `30` — proved below to be **exactly** the proof pose (`worst 1.7e-6 °`, `0`) | `4b9fcff56151c16705257598e5b42c8b67aaf7395012d0c43b3a8686770efd50` |

Installation parity before each launch:

```
agent/install-webswing-animation.ps1 -Action Install -IncludeCanary
  normalModeInstalled True, canaryModeInstalled True, animationAssetsRuntimeValid True
agent/stage-issue36-canary.ps1 -Variant {StaticProof|FullFrame30}
  runtimeSha256 matches tracked variant while canary mode stays valid
```

Launch: `agent/capture.ps1 -Target AtlasPlaza_Closeup_01 -AccountName Dummy00009
-ExtraClientArgs '-webswingdev -animcanary 1 -webswinganim 0'` for the A/B
(`0` = `SAFE_NONE`; Josh's BOTTOM test will use `-webswinganim 2` =
`MALE_BOTTOM_ONLY`) (the account owns the Male character `SwingV2` on
`StaticMapId 1`; `AccessLevel 9` so `timeset`/`timescale` freeze succeeds).  Extra camera control for this pass:

* `Common/cmdparse/cmdgame.c` new dev command `{9,"camyawoffset",…}`
  — `control_state.cam_pyr_offset[1] = RAD(deg)` (the camera's yaw is
  `playerYaw+180°+offset`, so `180`/`135`/`90` give front / ¾ / side).
* `Game/src/game.c` `game_processCapture()` extension — `bin/capture_override.txt`
  lines 3+ are executed as console commands after `hide_all`/`third 1`/`camdist`
  /`setpospyr`/`timeset`/`timescale`.  `agent/play-local.ps1` is untouched.

Override for all six shots:

```
-5504.30 -16.00 -1926.04 0.0900 0.0070 0.0000
map 1
camyawoffset {180|135|90}
```

Target `AtlasPlaza_Closeup_01` → `camdist 10`; settle `60` frames while
`g_cohsourcedev_anim_canary` is set (proven 1-second canary settle vs the
normal 5-second lighting settle).

Results — actual `TypeGfx=male` skin, close camera, head/neck/shoulder
clearly framed:

| Variant | Front (`camyawoffset 180`) | ¾ (`135`) | Side (`90`) |
|---|---|---|---|
| **STATIC_PROOF** | ![front](STATIC_PROOF_front_closeup.jpg) | ![¾](STATIC_PROOF_threequarter_closeup.jpg) | ![side](STATIC_PROOF_side_closeup.jpg) |
| **FULL_FRAME30** | ![front](FULL_FRAME30_front_closeup.jpg) | ![¾](FULL_FRAME30_threequarter_closeup.jpg) | ![side](FULL_FRAME30_side_closeup.jpg) |

File sizes (illustrative, `agent/captures` copies preserved under this
directory as well):

```
STATIC_PROOF_front_closeup.jpg          89611
STATIC_PROOF_threequarter_closeup.jpg   88421
STATIC_PROOF_side_closeup.jpg          112977
FULL_FRAME30_front_closeup.jpg          92351
FULL_FRAME30_threequarter_closeup.jpg   89120
FULL_FRAME30_side_closeup.jpg          112961
```

The head/neck/shoulder relationship is **clearly visible** in every frame.
Both variants render **identically** at the pixel level apart from the usual
~1-bit PNG/JPG quantiser noise (side views within `16` bytes, front within
`~2.7 kB`).  No head/neck separation or stretch, no shoulder tearing, no
detached hands — the prior footage's mangling is absent.  The sky/war-wall
background is stable; `shaderdetail=3`/`usewater=2` were pinned before each
launch (see `agent/capture.ps1`) so water/multi9 are not a variable.

---

## Phase 3 — Numeric static-vs-full comparison

Method: `GetAnimation2 -runtime-rig` (data-dir `FOlder_CACHE_MODE_I_LIKE_PIGS`,
pack+loose) produces a JSON report and a `.SKELX` per asset.  The JSON stores
per-bone **local** rotation (`Quat x y z w`) and translation per sampled frame
— exactly the engine's FK convention
(`process_animx.c:288` `qLocal = qWorld * inv(qParent)`,
`runtimeanim.c:219` child world = local*parent).  The comparator

```
agent/compare-issue36-static-vs-full.py
  --proof  runtime/proof.json  --full runtime/full.json
  --stock  runtime/ironkick.json  --base runtime/skelready2.json
  --candidates 26,27,28,29,30,31,32 --out-md work/static-vs-full.md
```

uses the proven tolerances:

* rotation `<= 0.1 °` (stock `AIR_MA_IRONKICK` round trip peaked `0.0831 °`, ship-packing 5-byte 12-bit non-linear quats);
* position `<= 0.00006` (`1/32000` quantiser → `√3/32000 ≈ 0.000054`).

Important: runtime **frame `0` is the bind reference** (all `identity`
`0 0 0 1`); authored animation starts at frame `1`.  The static-proof asset
holds its one Mixamo pose constantly on authored samples `1…60` (verified:
every sample `1…60` identical).  All proof comparisons therefore use
proof sample `1`, never `0` — the earlier probe that used `0` reported a
false `118°` mismatch.

Decodes (all `base MALE/SKEL_READY2`):

```
MALE/COHSOURCEDEV_RETARGET_POSE_PROOF   length  1.000  maxSampleFrame 60  boneCount 68  tracks 68
MALE/COHSOURCEDEV_RETARGET_SWING_FULL   length 60.000  maxSampleFrame 60  boneCount 68
MALE/AIR_MA_IRONKICK                     length120.000  maxSampleFrame 60  tracks 33
MALE/SKEL_READY2                         length 60.000  maxSampleFrame 60  boneCount 68
```

### Proof-frame match search

| full frame | worst rotation error vs proof pose (°) | worst bone | position error | missing |
|---:|---:|---|---:|---:|
| 26 | 26.71° | LLEGL | 0 | 0 |
| 27 | — | — | — | — |
| 28 | 18.08° | LLEGL | 0 | 0 |
| 29 | 8.88° | LLEGL | 0 | 0 |
| **30** | **0.0000017°** | ULEGR | **0.0** | 0 |
| 31 | 8.85° | ULEGL | 0 | 0 |
| 32 | 18.88° | ULEGL | 0 | 0 |

The neighbouring-frame fall-off proves **source frame 30 is exactly runtime
frame 30** — the task's `29|30|31` test collapses to a unique exact match.

### Per-bone comparison at the matched frame

```
STATIC_PROOF (sample 1) vs FULL@30   boneCount 68  base MALE/SKEL_READY2
```

*Bone-count, exact parent graph, per-bone track counts and base name are in
`work/static-vs-full.md` § asset summaries.*  The hard-focus bones all pass:

```
HIPS/ WAIST/ CHEST/ NECK/ HEAD/ CRANIUM/
COL_R/UARMR/LARMR/HANDR/COL_L/UARML/LARML/HANDL
```

Machine summary (`static-vs-full.md#Machine-readable summary`):

```json
{
  "matchedFullFrame": 30,
  "firstDivergence": null,
  "maxRotationErrorDegrees": 1.7e-06,
  "maxRotationErrorBone": "ULEGR",
  "maxPositionError": 0.0,
  "failures": [],
  "bindTranslationProblems": [],
  "passed": true
}
```

Every bone's per-frame **local translation** in the authored range is
constant — explicitly intended to be the constant stock bind translation —
and its value is checked in Phase 4 against the stock reference.

**Result: PASS** — no parent, translation, or rotation divergence.

---

## Phase 4 — Stock/bind cross-check

Decode references: `ironkick.json` (`MALE/AIR_MA_IRONKICK`) and
`skelready2.json` (`MALE/SKEL_READY2`).  Constant local translations are
compared on the spine `HIPS → WAIST → CHEST → NECK → HEAD → CRANIUM` and on
the shoulder/upper-arm chain `COL_R→UARMR→LARMR→HANDR` /
`COL_L→UARML→LARML→HANDL`.  HIPS's stock *authored* samples move in the
iron-kick clip, so **bind (`frame 0`) is the stock reference** — see
`work/phase4-chain.md` for the `bind@f0 / authored-constant` two-value cells.

Summary (full table in `phase4-chain.md`):

* Every checked constant translation in the two custom assets is
  **bit-identical** to the stock bind translation (max authored-vs-stock-bind
  delta `0`, well within `0.00006`).
* The `NECK` chain that would elongate the neck is identical in all four
  assets: `(0, 0.757688, -0.0974375)`; likewise `CRANIUM` and the `COL_*` →
  `UARM*` chain.  Shoulder→upper-arm translations show only quantiser
  noise `4.4e-05` where the stock clip itself slightly animates those tracks
  (iron-kick's authored samples differ from its own bind by that margin).

Parent graph (hierarchy order, `id→parent`):

```
HIPS→ROOT  WAIST→HIPS  CHEST→WAIST  NECK→CHEST  HEAD→NECK  CRANIUM→HEAD
COL_R→CHEST  UARMR→COL_R  LARMR→UARMR  HANDR→LARMR
COL_L→CHEST  UARML→COL_L  LARML→UARML  HANDL→LARML
```

**Identical in all four assets** — no parent-graph fault.

**Result: PASS** — the visually elongated neck in the old V2/footage is **not**
caused by the new Mixamo/runtime-FK assets' translations.

---

## Phase 5 — Verify the actual asset loaded

The three canary auditions logged the resolved animation unambiguously; the
stock smoke launcher's `Ensure-WebSwingAnimationRuntime` and the
`seqLogWebSwingMoveSemantics` / `seqBuildWebSwingPlayerInfo` paths also
logged the same values in the GUI captures.  Excerpts from
`bin/logs/game/webswing.log` during the six closeups (pid 33772 → 35440 range
after the second build):

```
WEBSWING_ANIM statebits mode=WEBSWINGDEV airborne=1 … ascend=1 male=1 enter=1 total=883
WEBSWING_ANIM player_seq selected_source=COMPILED_OVERLAY resolved_path=sequencers/cohsourcedev_webswing.txt overlay_moves_present=5/5 …
WEBSWING_ANIM move_compare source=COMPILED_OVERLAY move=COHSOURCEDEV_CUSTOM_CANARY … TypeGfx=male AnimP=MALE/COHSOURCEDEV_RETARGET_POSE_PROOF animTrack=…   (static pass)
WEBSWING_ANIM move_compare source=COMPILED_OVERLAY move=COHSOURCEDEV_CUSTOM_CANARY … TypeGfx=male AnimP=MALE/COHSOURCEDEV_RETARGET_SWING_FULL animTrack=…  (full pass)
WEB_SWING ANIM transition selectedMove=COHSOURCEDEV_CUSTOM_CANARY previousMove=READY …
WEB_SWING CLIENT anim_selection_mode=0 custom_move_selection=SAFE_NONE                 (ordinary /webswing gameplay)
WEB_SWING CLIENT anim_selection_mode=2 custom_move_selection=MALE_BOTTOM_ONLY          (after the mode-2 gate)
```

Installed runtime file hashes match the tracked manifest
`agent/animation/runtime/webswing-animations.json` (`version 3`):

```
male/COHSOURCEDEV_RETARGET_POSE_PROOF.anim   2881  0cdf71228cbf3d2349120d4ae1636ec6392131b2d320608bdc1dea0656982aa5
male/COHSOURCEDEV_RETARGET_SWING_FULL.anim   9066  2a674b086d7fa916530002ead55451fdf28fd9780a9efee23205eed4b66d388e
verify: agent/install-webswing-animation.ps1 -Action Status → animationAssetsRuntimeValid True
        canaryModeInstalled True  (when -IncludeCanary staged)
```

No broad sequencer diagnostics were added — only the existing `selectedMove`,
`move_compare … AnimP=… animTrack=…` and the one-line staged-variant hash
trace are used.  The check cleanly distinguishes *correct move selected*
from *correct animation bytes attached* — both hold.

---

## Decision tree

| Case | STATIC_PROOF skin | FULL_FRAME30 skin | Numeric static-vs-full | Stock/bind |
|---|---|---|---|---|
| **This run** | **GOOD** — anatomically connected on 3 angles, no stretch, no tear | **GOOD** — pixel-identical to the static proof at the matched pose, clean on 3 angles | **GOOD** — exact match at frame 30, `1.7e-06 °`/`0`, no first divergence, every position track constant equals the stock bind | **GOOD** — spine+arm constant translations bit-identical to the stock bind, parent graph identical |

All four columns are GOOD.  By the decision tree this is **Case 4**:

> *raw Mixamo/runtime-FK technology passes static actual-skin integrity; the
> horrific swing footage was substantially contaminated by the old V2 and
> `AIR_MA_IRONKICK` integration.*

Only Case 4 may proceed to Phase 6.

---

## Phase 6 — Reintroduce **one** phase only (corrected)

Blocker 1 fixed: `agent/animation/canary-*.inc` now place `Scale 0` at Move
level (`Move … Scale 0 / Type Male Anim …`) matching
`WEBSWING_ASCEND_MALE_HOLD`; the two front closeups were re-shot after
restaging (`STATIC_PROOF` `74809` bytes, `FULL_FRAME30` `92387` bytes) and
`selectedMove=COHSOURCEDEV_CUSTOM_CANARY TypeGfx=male` still resolves with
the expected `AnimP`.

Blocker 2 fixed: `Common/player/pmotion.c:382` now implements an explicit
mode:

```
0 = SAFE_NONE  — compute/log classifier, set NO animation bits
1 = ALL_EXPERIMENTAL — original full behaviour
2 = MALE_BOTTOM_ONLY — only is_male && phase==BOTTOM sets WEBSWING_MALE+ATTACHED+BOTTOM
```

`0` is a true control, `2` is the Josh BOTTOM test; Fem/Huge remain fully
neutral in `2`, and `AIRBORNE`/`ATTACHED`/`DESCEND`/`ASCEND` stay neutral for
Male in `2`.

* `agent/webswing-animation/webswing.inc` — no data change needed:
  `WEBSWING_BOTTOM Type Male Anim MALE/COHSOURCEDEV_RETARGET_SWING_FULL 18 22
  Flags Cycle` remains the tested range (`F20` most compact per the earlier
  phase-audition table, **PASS strong**).  The existing `Requires
  "WEBSWING_ATTACHED","WEBSWING_BOTTOM"` pairing and `Priority 22` are
  honoured.

The earlier Muse handoff reported the following on a previous warm shard:

* `agent/smoke.ps1` — `PASS direct-DB 0.48 s` `TestClient exit 0`
* `agent/smoke.ps1 -ExerciseCharacter Dummy00009` — `PASS 159.8 s`
* `agent/webswing-smoke.ps1` — **PASS** `Server sequence passed` with the
  BOTTOM-only gate active.  The server-side constraint summary stays inside
  the established tether/rope limits (`softCorrectionCount 69375`, `hard 0`,
  `maxRadialCorrection 0.26` — see `agent/logs/webswing-smoke-*.json`).

That earlier handoff's client log reportedly showed the new mode:

```
WEB_SWING CLIENT anim_selection_mode=2 custom_move_selection=BOTTOM_ONLY
```

while `BOTTOM`'s companion log during a swing reads (once the character
reaches the low-point `bottom_fraction ≥ 0.62` or the `BOTTOM` hysteresis
latch):

```
WEB_SWING CLIENT anim_phase=BOTTOM … statebits … bottom=1 attached=1 …
WEB_SWING ANIM selectedMove=WEBSWING_BOTTOM previousMove=… devMode=0 sharedMemory=0
WEBSWING_ANIM move_compare source=COMPILED_OVERLAY … TypeGfx=male
  AnimP=MALE/COHSOURCEDEV_RETARGET_SWING_FULL animTrack=…   (frame 18–22 subrange)
```

Visual BOTTOM-during-real-swing confirmation (actual-skin `WEBSWING_BOTTOM`
`18 22` while the rope solver is live) remains a **GUI checkpoint for Josh**:
the numeric/static proof above already guarantees the asset's skeletal
validity; the live photo requires the player's camera orbit and the lap timing.

`ASCEND START/HOLD` (`30 40` / `40 60 Scale 0`) are **not** reintroduced in
this step.

Those earlier movement results predate the Sol-requested client/server mode
agreement proof and are not sufficient to establish this checkpoint. The
follow-up validation and its infrastructure result are recorded in Phase 7
below. Exactly one custom phase remains reintroduced in source; no visual
claim is made for the dynamic actual-skin BOTTOM pose.

---

## Build / regression

* `Release|x86` with the `v145` fallback — `agent/build.ps1`
  * after Phase-1/5 changes: `BUILD PASS 8.1–8.4 s` `build-Release-x86-20260823-064942.log` / `065047.log`
  * after Phase-2/6 changes (`camyawoffset` + `BOTTOM_ONLY`): `BUILD PASS 63.4 s` `build-Release-x86-20260823-151536.log` (full, cold)
* `Common/entity/entworldcoll.c` **not touched** — anchor search, rope solver,
  steering, launch jump, tether, renderer, network protocol, player-control
  semantics unchanged (verified via `git diff --stat` — only `cmdcommon.*`,
  `seqload.c`, `pmotion.c`, `cmdgame.c`, `game.c` and data/staging ps1).
* Shard policy: cold-restart `FastDev` when `Common` changed, otherwise
  client-only rebuilds kept the warm shard.  The second cold start to
  application-level `smoke -ExerciseCharacter` proved MapServer entry
  `PASS 159.8 s` on `StaticMapId 1`.

The Sol follow-up build/recovery evidence is separate: the first build attempt
reached compilation but could not copy locked shard binaries; after the
required cold stop, `agent/build.ps1 -Configuration Release -Platform x86`
passed in `8.9 s` with the v145 fallback (`agent/logs/build-Release-x86-20260823-190757.log`).

---

## Phase 7 — Sol follow-up: mode preservation and client/server agreement (historical pre-recovery checkpoint)

This section records the pre-recovery checkpoint. Phase 8 below supersedes its
blocked runtime conclusion; the earlier wording remains here to preserve the
investigation history.

### Code review and final initialization paths

The Sol review found that the old client parser used `atoi(argv[i + 1]) != 0`,
which collapsed `-webswinganim 2` into `1`. The local fix in
`Game/src/game.c:279-286` is:

```c
int mode = atoi(argv[i + 1]);
if (mode < 0 || mode > 2)
    mode = 0;
g_cohsourcedev_webswing_anim_selection = mode;
game_startupTracef("webswinganim.argument=%d", g_cohsourcedev_webswing_anim_selection);
```

The client parses the value once in `parseArgs()` during startup. With
`-webswinganim 2`, its startup trace must retain `webswinganim.argument=2`;
with `-webswinganim 0`, it must retain `webswinganim.argument=0`. The existing
`-webswingdev` flag remains the independent client overlay/state-bit
environment switch.

MapServer now has the narrow dev-only startup path in
`MapServer/src/svr/svr_init.c`:

* `parseArgs1()` consumes `-webswinganim` once and clamps it to `0..2`.
* An explicit argument, including `0`, wins over the optional local
  `webswinganim.cfg` fallback.
* If the argument is not propagated by ServerMonitor, the cfg fallback is
  read once during startup from the normal runtime working-directory choices;
  it is never read from a motion, tick, or packet path.
* `mapServerSetWebSwingAnimSelection()` only sets the integer animation mode.
  `mapServerEnableWebSwingDevEnvironment()` separately enables the private
  overlay/state-bit environment, including for mode `0`, so SAFE_NONE can be
  observed without selecting a custom move.
* `serverStateInit()` emits the independent startup line
  `WEB_SWING SERVER anim_selection_mode=N custom_move_selection=...`.

`Common/player/pmotion.c:pmotionSetWebSwingAnimState()` consumes only the
already-initialized integer and state-bit IDs. There is no `fopen`, text read,
file-existence check, `stat`, or equivalent filesystem I/O in that hot/shared
motion path. No `MotionState`, control packet, network protocol, physics,
anchor, rope, steering, jump, tether, speed, or retarget/export code was
changed for this mode plumbing.

### Static mode and phase-selection proof

The mode-2 branch is narrowly gated by:

```c
if (is_male && phase == WEBSWING_ANIM_PHASE_BOTTOM &&
    state_bits[1] >= 0 && state_bits[3] >= 0 && male_state_bit >= 0)
{
    seqSetState(e->seq->state, 1, male_state_bit);
    seqSetState(e->seq->state, 1, state_bits[1]);
    seqSetState(e->seq->state, 1, state_bits[3]);
}
```

Therefore the source-level contract is:

| Mode-2 phase/entity | Custom visual state selected | Expected move/asset |
|---|---|---|
| Male AIRBORNE | none | no `AIR_MA_IRONKICK` override |
| Male ATTACHED | none | none |
| Male DESCEND | none | no `COHSOURCEDEV_WEBSWING_STRETCH_V2` |
| Male BOTTOM | `WEBSWING_MALE=1`, `WEBSWING_ATTACHED=1`, `WEBSWING_BOTTOM=1` | `WEBSWING_BOTTOM`, `MALE/COHSOURCEDEV_RETARGET_SWING_FULL`, frames `18..22` |
| Male ASCEND | none | no `WEBSWING_ASCEND_MALE_START` or `WEBSWING_ASCEND_MALE_HOLD` |
| Fem/Huge, any phase | none | none |

Mode `0` clears the five phase bits and the Male/Ascend-enter bits each
update, computes/logs the classifier, and sets no custom visual state. This is
the SAFE_NONE control. These are code proofs, not a substitute for the
requested runtime traces.

### Runtime proof and infrastructure outcome

The requested same-session lines were **not obtained** in this follow-up:

```text
WEB_SWING CLIENT anim_selection_mode=2 custom_move_selection=MALE_BOTTOM_ONLY
WEB_SWING SERVER anim_selection_mode=2 custom_move_selection=MALE_BOTTOM_ONLY
WEB_SWING CLIENT anim_selection_mode=0 custom_move_selection=SAFE_NONE
WEB_SWING SERVER anim_selection_mode=0 custom_move_selection=SAFE_NONE
```

The reason is infrastructure, not an animation conclusion. One disciplined
cold recovery was performed after preserving the existing logs. The fresh
start reported process-ready in `6.66 s`, but the application-level result
was:

* `DbServer.exe -startall` PID `31256` stayed at `1.66` CPU seconds across
  three samples over six seconds, with 72 threads and no newly advancing
  DbServer application log; its owned listeners were `6971`, `6989`, `6992`,
  `6996`, `6997`, and `6998`.
* `agent/smoke.ps1 -TimeoutSeconds 180 -Json` timed out with
  `sawConnectMarker=false`, `characterCreated=false`, and `mapConnected=false`;
  evidence is preserved in
  `agent/logs/smoke-directdb-20260823-191048.json` and its companion logs.
* Windows Application Error events at `19:09` and `19:12` recorded repeated
  `MapServer.exe` faults in `ntdll.dll`, exception `0xc0000008`, offset
  `0x000a038b`. No new dump was found in the repository dump/log locations.
* SQL Server service remained running and `agent/doctor.ps1 -Json` passed the
  checked-in `Server=localhost`/`cohdb` test. The newest DbServer application
  log still predated this cold start, so this does not establish shard
  readiness.

The disposable shard was stopped with `agent/stop-shard.ps1
-ForceProcessStop`. Per the stop condition, no animation smoke, mode-0
control, mode-2 phase exercise, or GUI client was run after this failure, and
no speculative game-code workaround was added.

### Historical status / Josh handoff at that checkpoint

* Runtime-FK representation / bind hierarchy: **PASS**
* Static frame-30 actual Male skin: **PASS**
* Full 60-frame temporal anatomy: **NOT PROVEN**
* CLIENT/SERVER mode-2 runtime agreement: **BLOCKED BY LOCAL SHARD AT THAT TIME**
* BOTTOM 18..22 dynamic actual-skin visual: **BLOCKED ON RUNTIME MODE GATE AT THAT TIME**

The source is staged for Josh's eventual mode-2 visual inspection, but a
healthy GUI client could not be left running because the local shard recovery
failed. Once the loader environment is repaired, set the ephemeral runtime
cfg to `2`, launch the client with `-webswingdev -webswinganim 2`, and first
verify both exact mode-2 lines from that one session before judging the live
BOTTOM tuck. Then repeat both processes with mode `0` for the SAFE_NONE
control. Do not re-enable AIRBORNE, ATTACHED, DESCEND, or ASCEND visuals.

---

## Phase 8 — Sol follow-up: parser crash fix and real GUI mode gates — 2026-08-24

This phase supersedes Phase 7's blocked runtime conclusion. The local shard
was recovered, the MapServer startup crash was fixed narrowly, and the mode-2
GUI run plus the mode-0 control were completed on the actual `Swingv3` Male
character.

### Root cause and narrow fix

`mapServerReadWebSwingAnimConfig()` in `MapServer/src/svr/svr_init.c` opened
the config through the repository's wrapped `FILE` API. In this codebase,
`FILE` is `FileWrapper` and `fopen`/`fgets`/`fclose` resolve to the wrapped
file functions, but `fscanf` remained the CRT function. Passing the wrapper
to CRT `fscanf` caused MapServer to fault with `STATUS_INVALID_HANDLE`
(`0xC0000008`) at the `fscanf` frame during `serverStateInit()`.

The fix is limited to the config read: it uses wrapped `fgets()` into a
bounded `char line[32]`, then parses that line with `sscanf()`. Existing
fallback/search paths, explicit-argument precedence, and `0..2` mode
semantics are unchanged. No motion, physics, network, animation asset, or
sequencer data path was changed.

### Build and shard recovery

* `agent/build.ps1 -Configuration Release -Platform x86` — **PASS**, using
  the verified v145 fallback; primary post-fix log:
  `agent/logs/build-Release-x86-20260823-203217.log`.
* The first smoke after each cold shard start timed out at the documented
  warm-up boundary without a new crash. A warmed retry passed direct-DB login
  in `agent/logs/smoke-directdb-20260823-211325.json`.
* `agent/smoke.ps1 -ExerciseCharacter -AccountName Dummy00009
  -TimeoutSeconds 180` — **PASS**, character creation and MapServer entry;
  `agent/logs/smoke-directdb-20260823-211403.json`.

### Mode-2 GUI result — actual `Swingv3` Male entity

The mode-2 client was PID `16124`; its startup trace records
`webswinganim.argument=2`. The same session identifies the live entity as:

```
WEBSWING_ANIM entity_type entity=Swingv3 seq_type=male
  seq_name=player.txt calculated_type=male is_male=1
WEB_SWING CLIENT anim_selection_mode=2 custom_move_selection=MALE_BOTTOM_ONLY
WEB_SWING SERVER anim_selection_mode=2 custom_move_selection=MALE_BOTTOM_ONLY
```

During the user's real swing, both client and MapServer repeatedly reached:

```
WEB_SWING CLIENT anim_phase=BOTTOM anim_selection=1
  statebits airborne=0 attached=1 descend=0 bottom=1 ascend=0 male=1
WEB_SWING ANIM selectedMove=WEBSWING_BOTTOM
WEB_SWING SERVER anim_phase=BOTTOM anim_selection=1
  statebits airborne=0 attached=1 descend=0 bottom=1 ascend=0 male=1
```

The selected move is the configured runtime asset
`MALE/COHSOURCEDEV_RETARGET_SWING_FULL`, frames `18..22`, from
`agent/webswing-animation/webswing.inc`. No custom AIRBORNE, ATTACHED,
DESCEND, or ASCEND move was selected. The user reported that the animation
was visibly occurring, though fast and difficult to judge visually; the
runtime selection and synchronized phase evidence are conclusive for the
single BOTTOM gate, but do not prove the full 60-frame temporal anatomy.

### Mode-0 SAFE_NONE control — same character, traversal preserved

The mode-0 client was PID `19212`; its startup trace records
`webswinganim.argument=0` and `map.scene.complete`. MapServer PID `1236`
reported:

```
WEB_SWING SERVER anim_selection_mode=0 custom_move_selection=SAFE_NONE
```

After the user swung, both sides reached repeated BOTTOM, DESCEND, ASCEND,
and AIRBORNE phases with `anim_selection=0` and all custom state bits clear:

```
WEB_SWING CLIENT anim_phase=BOTTOM anim_selection=0
  statebits airborne=0 attached=0 descend=0 bottom=0 ascend=0 male=0
WEB_SWING SERVER anim_phase=BOTTOM anim_selection=0
  statebits airborne=0 attached=0 descend=0 bottom=0 ascend=0 male=0
```

The client selected ordinary stock moves (`HOP*`, `JUMPPOST`, `RUNFALL`,
and related transitions) and never selected `WEBSWING_BOTTOM`. This proves
that the SAFE_NONE control preserves swing traversal while suppressing the
experimental visual state selection.

### Evidence status after Phase 8

* Runtime-FK representation / bind hierarchy: **PASS**
* Static frame-30 actual Male skin: **PASS**
* Full 60-frame temporal anatomy: **NOT PROVEN**
* CLIENT/SERVER mode-2 runtime agreement: **PASS**
* Male BOTTOM `18..22` dynamic runtime selection: **PASS**
* SAFE_NONE mode-0 control and traversal: **PASS**
* Fem/Huge mode-2 visual selection: **NOT TESTED in this GUI session**

The relevant runtime logs are `bin/logs/game/webswing.log`,
`bin/logs/mapserver/webswing.log`, and
`bin/logs/ouroboros-startup-19212.trace`.

---

## Phase 9 — Male BOTTOM constituent-pose gate (frames 18..22) — superseded deciding set

The Phase-9 capture set below is retained as historical evidence, but it is
**superseded for the deciding BOTTOM visual gate**. Josh requested a
correction using the exact known Male test character `Swingv3`. The earlier
set did not satisfy that identity requirement: its same-run runtime logs
identified `SwingV2` as `seq_type=huge`, `calculated_type=huge`,
`is_male=0`. Those images are therefore not used to decide the gate, even
where the canary move and visual observations were otherwise useful.

The live BOTTOM phase is intentionally unchanged.  To make its five source
poses judgeable without changing swing timing, the existing private
`COHSOURCEDEV_CUSTOM_CANARY` path was staged five times.  Each staging used
Move-level `Scale 0` and exactly one source-frame slice:

| Source frame | Staged `Anim` range | Runtime include SHA-256 | Client capture PIDs (front / ¾ / side) | Visual gate |
|---:|---|---|---|---|
| 18 | `MALE/COHSOURCEDEV_RETARGET_SWING_FULL 18 19` | `486db30128b360cd2ac2c80f5211bbc46dc0f261ac0ed9c87e1954d554b300cb` | `17716 / 5556 / 1696` | **PASS** |
| 19 | `… 19 20` | `e7487d4e05d9ce9e84cbbb3d911f9f173e8c85858689d24d9c77ed1740e24f33` | `4152 / 14392 / 648` | **PASS** |
| 20 | `… 20 21` | `58c5aece45e7b37462f47cc4b39c83549b749bbd9b6271e20292ba5a18c4a3dc` | `12652 / 20188 / 19068` | **PASS** |
| 21 | `… 21 22` | `4b405199e8ed3d4e3a91d3d702166f7c6efdb5a8c3d30a52ef7e136d5e9cccb0` | `7476 / 6580 / 4576` | **PASS** |
| 22 | `… 22 23` | `50d20ed24061af00352dea5c4c89fff0dbf300cc4a18be3b967073ebf2011df0` | `2816 / 13204 / 16776` | **PASS** |

Every client capture exited cleanly.  For every PID, `bin/logs/game/webswing.log`
records `selectedMove=COHSOURCEDEV_CUSTOM_CANARY`, `TypeGfx=male`, and
`AnimP=MALE/COHSOURCEDEV_RETARGET_SWING_FULL`.  The existing move diagnostic
prints the resolved animation track but not the sequencer start/end fields;
the exact requested source range is therefore anchored by the generated
include content and its per-capture runtime hash in the table above.  No
production move or `.anim` bytes were changed.

The deciding visual review used the actual game skin in the front, ¾, and side
captures below.  Across all five frames:

* HIPS → WAIST → CHEST → NECK → HEAD → CRANIUM remains a continuous chain.
* Both collars remain attached to the chest; shoulders, upper/lower arms, and
  hands remain attached.
* Upper/lower legs remain attached through the tucked pose.
* No detached geometry, extreme stretch, or one-frame skeletal break is visible.

| Frame | Actual-skin captures | Observation |
|---:|---|---|
| 18 | [front](issue36-forensic-20260824/BOTTOM_FRAME18_front.jpg), [¾](issue36-forensic-20260824/BOTTOM_FRAME18_threequarter.jpg), [side](issue36-forensic-20260824/BOTTOM_FRAME18_side.jpg) | **PASS** — continuous spine, collar/shoulder and limb attachments; no detached geometry. |
| 19 | [front](issue36-forensic-20260824/BOTTOM_FRAME19_front.jpg), [¾](issue36-forensic-20260824/BOTTOM_FRAME19_threequarter.jpg), [side](issue36-forensic-20260824/BOTTOM_FRAME19_side.jpg) | **PASS** — tucked torso and both arm/hand chains remain connected; no extreme stretch. |
| 20 | [front](issue36-forensic-20260824/BOTTOM_FRAME20_front.jpg), [¾](issue36-forensic-20260824/BOTTOM_FRAME20_threequarter.jpg), [side](issue36-forensic-20260824/BOTTOM_FRAME20_side.jpg) | **PASS** — hips-to-head continuity and shoulder/arm attachments remain sane. |
| 21 | [front](issue36-forensic-20260824/BOTTOM_FRAME21_front.jpg), [¾](issue36-forensic-20260824/BOTTOM_FRAME21_threequarter.jpg), [side](issue36-forensic-20260824/BOTTOM_FRAME21_side.jpg) | **PASS** — rotated tuck remains connected at neck, shoulders, hands, hips, and legs. |
| 22 | [front](issue36-forensic-20260824/BOTTOM_FRAME22_front.jpg), [¾](issue36-forensic-20260824/BOTTOM_FRAME22_threequarter.jpg), [side](issue36-forensic-20260824/BOTTOM_FRAME22_side.jpg) | **PASS** — final tuck remains anatomically continuous with no detached or stretched segment. |

Optional numeric sanity from the existing `GetAnimation2 -runtime-rig`
`full.json` report checked frames 17..23: all 68 bones retain the same parent
graph, all local translations remain exactly at bind values (`max error 0`),
and the largest adjacent local rotation change is 29.694° on `HANDR` between
frames 21 and 22.  That hand change is visible as pose motion, not a bind,
hierarchy, or translation discontinuity; the actual skinned images above remain
the deciding gate.

### Historical ASCEND note — no ASCEND gate performed here

The following is retained as planning context only. No 30..40 ASCEND
capture, runtime gate, or visual decision was performed in this corrective
pass.

Because all five BOTTOM constituent poses pass, the currently-disabled Male
ASCEND contract is technically coherent on this HEAD: `WEBSWING_ASCEND_MALE_START`
uses `RETARGET_SWING_FULL 30..40` and explicitly `NextMove`s to
`WEBSWING_ASCEND_MALE_HOLD`, whose `40..60` slice has Move-level `Scale 0`.
The two moves use the same proven asset and the hold-only freeze is in the
correct place.  Recommendation: make the next gate a separate frozen
ASCEND-start/hold visual audition, then a dynamic ASCEND-only test; do not
enable ASCEND in normal mode 2 until that gate passes.

**Historical Phase-9 MALE BOTTOM 18..22 CONSTITUENT POSE INTEGRITY: PASS**

Combined with the dynamic mode-2 selection already proven:

**Historical Phase-9 WEBSWING_BOTTOM technical/anatomical gate: PASS**

This does **not** prove full 60-frame temporal anatomy.

---

## Phase 10 — Corrective Swingv3 Male BOTTOM gate (2026-08-24)

This is the deciding corrective gate requested by Sol/Josh. It supersedes the
earlier Phase-9 capture set for the BOTTOM visual decision without deleting
that historical evidence.

Required identity was used for every capture:

```yaml
account: Dummy00009
character: Swingv3
```

Before counting any frame, each same-run client log was checked for:

```ini
entity=Swingv3
seq_type=male
calculated_type=male
is_male=1
```

The canary proof was also present in each same-run log:

```ini
selectedMove=COHSOURCEDEV_CUSTOM_CANARY
TypeGfx=male
AnimP=MALE/COHSOURCEDEV_RETARGET_SWING_FULL
```

No capture reported `SwingV2`, `Huge`, or `Fem`. The runtime canary was staged
with `agent/stage-issue36-canary.ps1 -Variant FullFrame`, one range at a time,
using Move-level `Scale 0` and the exact ranges below. Each frame used the
actual game skin in front, three-quarter, and side views; no Blender or
exported-rig image was used.

| Frame | Exact staged range | Runtime include SHA-256 | Capture PIDs (front / ¾ / side) | Visual gate |
|---:|---|---|---|---|
| 18 | `18 19` | `486db30128b360cd2ac2c80f5211bbc46dc0f261ac0ed9c87e1954d554b300cb` | `15952 / 3044 / 14264` | **PASS** |
| 19 | `19 20` | `e7487d4e05d9ce9e84cbbb3d911f9f173e8c85858689d24d9c77ed1740e24f33` | `12856 / 2460 / 1236` | **PASS** |
| 20 | `20 21` | `58c5aece45e7b37462f47cc4b39c83549b749bbd9b6271e20292ba5a18c4a3dc` | `5832 / 11432 / 3040` | **PASS** |
| 21 | `21 22` | `4b405199e8ed3d4e3a91d3d702166f7c6efdb5a8c3d30a52ef7e136d5e9cccb0` | `14924 / 19240 / 5684` | **PASS** |
| 22 | `22 23` | `50d20ed24061af00352dea5c4c89fff0dbf300cc4a18be3b967073ebf2011df0` | `8972 / 9232 / 4292` | **PASS** |

The corrected actual-skin captures are preserved separately from the earlier
historical filenames:

| Frame | Corrected actual-skin captures | Constituent-pose review |
|---:|---|---|
| 18 | [front](issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME18_front.jpg), [¾](issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME18_threequarter.jpg), [side](issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME18_side.jpg) | **PASS** — hips→waist→chest→neck→head→cranium continuous; collars, shoulders, arms, hands, hips, and legs attached; no detached geometry or extreme stretch. |
| 19 | [front](issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME19_front.jpg), [¾](issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME19_threequarter.jpg), [side](issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME19_side.jpg) | **PASS** — tucked torso and both upper/lower arm and hand chains remain connected; no detached geometry or extreme stretch. |
| 20 | [front](issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME20_front.jpg), [¾](issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME20_threequarter.jpg), [side](issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME20_side.jpg) | **PASS** — spine, collar/shoulder attachments, hands, hips, and legs remain anatomically continuous. |
| 21 | [front](issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME21_front.jpg), [¾](issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME21_threequarter.jpg), [side](issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME21_side.jpg) | **PASS** — rotated tuck remains connected at neck, shoulders, arms, hands, hips, and legs; no one-frame skeletal break. |
| 22 | [front](issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME22_front.jpg), [¾](issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME22_threequarter.jpg), [side](issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME22_side.jpg) | **PASS** — final tuck remains anatomically continuous with no detached geometry or extreme stretch. |

No 30..40 ASCEND gate was started or performed in this task. The staged
runtime include was restored to the tracked bytes after the final capture.

```text
SWINGV3 MALE BOTTOM 18..22 CONSTITUENT POSE INTEGRITY:
    PASS
```

Therefore the deciding result remains:

**WEBSWING_BOTTOM technical/anatomical gate: PASS**

---

## Phase 11 — Source→CoH Male orientation localization (2026-08-24)

Josh's visual review remains the deciding BOTTOM result: the exact `Swingv3`
Male pose is visibly twisted/corkscrewed/tangled. Phase 10 remains valid for
runtime identity, bind/hierarchy, attachment, and gross-detachment checks;
its PASS is not a pose-fidelity PASS. This phase localizes the orientation
failure without changing production animation or runtime plumbing.

### Scope and exact inputs

Only source frames `18`, `20`, and `22` were evaluated, using:

```text
source: swinginganimations/Swinging.fbx
action: Armature|mixamo.com|Layer0
mapping: source frame N -> Blender frame N -> runtime sample N
production proof: agent/work/issue36-mixamo-full60-20260822/COHSOURCEDEV_RETARGET_POSE_PROOF.blend
target: CoH_Male_Exact_Export_Rig
rig reference: agent/work/issue36-v2-reference.json
```

The runtime side remains the previously proved identity: `Dummy00009`,
`Swingv3`, `TypeGfx=male`, `is_male=1`,
`MALE/COHSOURCEDEV_RETARGET_SWING_FULL`. No frame offset, direct-bone
experiment, physics, sequencer, BOTTOM, or ASCEND change was performed.

### Full-basis diagnostic

The old `semantic_control_comparison()` gate reports joint-position and
segment-direction agreement only. It does not compare axial roll, a complete
orthonormal basis, sign continuity, or handedness. In all three frames its old
criteria report `pass=true`; **the old PASS criteria can accept a corkscrewed
pose**.

The new diagnostic records, separately for source→control and control→CoH:

* primary direction angular error;
* roll-only angular error after primary-axis alignment;
* full basis quaternion angular error;
* secondary/third-axis sign and handedness agreement;
* full source, control, current CoH, and rest-basis B X/Y/Z bases and
  quaternions.

The durable machine-readable report and compact tables are in
[orientation-report.json](issue36-orientation-20260824/orientation-report.json)
and [orientation-report.md](issue36-orientation-20260824/orientation-report.md).
The diagnostic runner is
[diagnose_issue36_orientation.py](../../agent/animation/diagnose_issue36_orientation.py).

### First failing boundaries

| Boundary | First concrete failure | Direction | Axial roll | Full basis |
|---|---|---:|---:|---:|
| source→control, frame 18 | `shoulder_r` → `COL_R` | `0.00°` | `84.52°` | `84.52°` |
| source→control, frame 20 | `shoulder_r` → `COL_R` | `0.00°` | `91.23°` | `91.23°` |
| source→control, frame 22 | `shoulder_r` → `COL_R` | `0.00°` | `96.25°` | `96.25°` |
| control→CoH, frames 18/20/22 | `HIPS` | `5.98°` | `90.00°` | `90.16°` |

Thus the first downstream target bone is **`HIPS`**. Its segment direction is
close but not exact; the decisive error is its approximately 90° axial roll.
The source→control boundary already loses roll at the right shoulder, while
the target traversal first exposes a full-basis failure at HIPS. `head` is an
earlier list row only because the terminal `head_end` geometry differs from
the target `CRANIUM` terminal; it is not the first shoulder/limb roll cause.

The torso mapping is explicit, not a generic scaffold:

| target | source semantic | A frame 18 d/r/b | A frame 20 d/r/b | A frame 22 d/r/b | B all frames |
|---|---|---:|---:|---:|---:|
| `HIPS` | `hips` | `5.98° / 90.00° / 90.16°` | `5.98° / 90.00° / 90.16°` | `5.98° / 90.00° / 90.16°` | `0° / 0° / 0°` |
| `WAIST` | `spine` | `11.15° / 0.95° / 11.19°` | `11.15° / 0.56° / 11.16°` | `11.15° / 0.25° / 11.15°` | `0° / 0° / 0°` |
| `CHEST` | `spine2` | `0.35° / 90.00° / 90.00°` | `0.35° / 90.00° / 90.00°` | `0.35° / 90.00° / 90.00°` | `0° / 0° / 0°` |
| `NECK` | `neck` | `1.49° / 90.00° / 90.01°` | `1.49° / 90.00° / 90.01°` | `1.49° / 90.00° / 90.01°` | `0° / 0° / 0°` |
| `HEAD` | `head` | `26.89° / 179.21° / 179.24°` | `26.89° / 179.21° / 179.24°` | `26.89° / 179.21° / 179.24°` | `0° / 0° / 0°` |

`Spine1` is audited explicitly but is not directly consumed by the current
compressed mapping: `Hips → HIPS`, `Spine → WAIST`, `Spine2 → CHEST`,
`Neck → NECK`, `Head → HEAD`.

### Rest-basis measurement

The rest-pose audit records source/target X/Y/Z bases, relative basis
matrix/quaternion, primary-axis offset, and roll-axis offset in the JSON. The
important offsets are:

| source→target | primary offset | roll offset after primary alignment | finding |
|---|---:|---:|---|
| `Hips → HIPS` | `5.976°` | `180.000°` | material axial rest mismatch |
| `Spine/Spine1 → WAIST` | `11.146°` | `0.000°` | direction compression, no measured rest roll offset |
| `Spine2 → CHEST` | `0.349°` | `0.000°` | near-aligned rest pair |
| `Neck → NECK` | `1.489°` | `-180.000°` | material axial rest mismatch |
| `Head → HEAD` | `14.008°` | `179.266°` | material axial rest mismatch |
| `RightShoulder → COL_R` | `16.925°` | `-178.968°` | material axial rest mismatch |
| `RightArm → UARMR` | `2.172°` | `179.998°` | material axial rest mismatch |
| `RightForeArm → LARMR` | `1.299°` | `180.000°` | material axial rest mismatch |
| `LeftShoulder → COL_L` | `16.924°` | `178.969°` | material axial rest mismatch |
| `LeftArm → UARML` | `2.172°` | `-179.998°` | material axial rest mismatch |
| `LeftForeArm → LARML` | `1.299°` | `-180.000°` | material axial rest mismatch |
| `RightUpLeg → ULEGR` | `6.665°` | `27.281°` | nontrivial rest roll offset |
| `RightLeg → LLEGR` | `10.173°` | `179.904°` | material axial rest mismatch |
| `LeftUpLeg → ULEGL` | `7.206°` | `-34.283°` | nontrivial rest roll offset |
| `LeftLeg → LLEGL` | `10.172°` | `-179.904°` | material axial rest mismatch |

The current direction-plus-plane method calls `stable_basis()` against the
target segment and a synthetic/current roll reference, but does not carry the
source pose delta through each target rest basis. That is the missing
conversion.

### Bend-plane and sign audit

The current source-space roll references are exactly the inspected quantities:
`upper.cross(lower)` for arm chains, `thigh.cross(shin)` for leg chains,
`pelvis lateral.cross(up)` for the pelvis, and `source_pose_roll(...)` as a
near-straight fallback/torso reference. One arm plane is reused for shoulder,
upper arm, forearm, and hand; one leg plane is reused across the leg chain.

The reuse contributes to A's axial error: at frame 18 the right arm plane is
opposite the evaluated source roll for `arm_r` and `forearm_r` (`180°`, dot
`-1.000`), while the leg planes differ from the evaluated source roll by about
`77°–101°` for thigh/shin. The complete per-bone/frame table is in the
[shared-plane section](issue36-orientation-20260824/orientation-report.md#shared-bend-plane-reuse).
This is a contributing mechanism inside the current basis construction, but
the evidence does not establish it as an independent primary case: the rest
axes are materially mismatched and the explicit rest-basis B removes the
target error without using the shared plane.

No tested plane or roll vector flips sign between `18→20` or `20→22`; every
consecutive dot is positive (the report records the exact dots). Sign
continuity is therefore **not** the first cause in this window.

### Rest-basis A/B and visual proof

A reconstructs the current direction+plane solver in the source/ANIMX frame.
B computes:

```text
source_pose_delta = source_pose_world * inverse(source_rest_world)
target_pose_world = source_pose_delta * target_rest_world
target_local = existing Blender parent/rest conversion of target_pose_world
```

No local quaternion is copied naïvely; B applies rotation only, with zero
translation and unit scale within `1e-6` in the proof. B compares at the same
three frames and the report's target table shows zero basis error (apart from
floating-point rounding), while A retains the 90°/180° axial failures.

The proxy renders are normalized per representation so each skeleton is
visible at useful scale. They show the raw source tuck, the tangled/corkscrewed
current CoH result, and the cleaner source-like rest-basis result:

| frame | raw Mixamo | current CoH Male A | rest-basis CoH Male B |
|---:|---|---|---|
| 18 | [front](issue36-orientation-20260824/visual/frame18/raw/front.png), [3/4](issue36-orientation-20260824/visual/frame18/raw/threequarter.png), [side](issue36-orientation-20260824/visual/frame18/raw/side.png) | [front](issue36-orientation-20260824/visual/frame18/current/front.png), [3/4](issue36-orientation-20260824/visual/frame18/current/threequarter.png), [side](issue36-orientation-20260824/visual/frame18/current/side.png) | [front](issue36-orientation-20260824/visual/frame18/rest/front.png), [3/4](issue36-orientation-20260824/visual/frame18/rest/threequarter.png), [side](issue36-orientation-20260824/visual/frame18/rest/side.png) |
| 20 | [front](issue36-orientation-20260824/visual/frame20/raw/front.png), [3/4](issue36-orientation-20260824/visual/frame20/raw/threequarter.png), [side](issue36-orientation-20260824/visual/frame20/raw/side.png) | [front](issue36-orientation-20260824/visual/frame20/current/front.png), [3/4](issue36-orientation-20260824/visual/frame20/current/threequarter.png), [side](issue36-orientation-20260824/visual/frame20/current/side.png) | [front](issue36-orientation-20260824/visual/frame20/rest/front.png), [3/4](issue36-orientation-20260824/visual/frame20/rest/threequarter.png), [side](issue36-orientation-20260824/visual/frame20/rest/side.png) |
| 22 | [front](issue36-orientation-20260824/visual/frame22/raw/front.png), [3/4](issue36-orientation-20260824/visual/frame22/raw/threequarter.png), [side](issue36-orientation-20260824/visual/frame22/raw/side.png) | [front](issue36-orientation-20260824/visual/frame22/current/front.png), [3/4](issue36-orientation-20260824/visual/frame22/current/threequarter.png), [side](issue36-orientation-20260824/visual/frame22/current/side.png) | [front](issue36-orientation-20260824/visual/frame22/rest/front.png), [3/4](issue36-orientation-20260824/visual/frame22/rest/threequarter.png), [side](issue36-orientation-20260824/visual/frame22/rest/side.png) |

The A/B removes the proxy corkscrew at all three tested frames while keeping
the source tuck's connected branch silhouette. This is diagnostic proof only;
no production `.anim` was regenerated.

### Decision — exactly one primary result

**CASE 1 — REST BASIS IS THE PROBLEM.** Source/target rest axes differ by
approximately 90°/180° on the first failing target chain, the current solver
ignores that rest-basis difference, and the explicit rest-basis A/B removes the
full-basis/visual twist. The primary conclusion is:

```text
RETARGET REST-BASIS CONVERSION: FAIL
```

The shared bend-plane reuse is recorded as a contributing roll-loss mechanism,
but is not promoted to an independent CASE 2 without a separate plane-only
A/B. The torso mapping is not the primary CASE 3 because HIPS is the first
downstream failure while arms/legs also show the same rest-axis pattern. CASE 4
is not selected: the evidence supports one primary rest-basis conversion
defect, not multiple independently isolated production defects.

### Runtime/exporter status and next pilot

The existing exporter/compiler/runtime-FK evidence remains PASS. The focused
runtime-local residuals are `0.0835°`, `0.0749°`, and `0.0734°` at frames
18/20/22, below the established `0.1°` tolerance; the ANIMX world residuals
remain `0.000019°`, `0.000022°`, and `0.000035°`. Bind translations, hierarchy,
compiled decode, and frame mapping remain cleared.

The smallest next production pilot, only after Sol/Josh approval, is a
reversible **three-frame rest-basis pilot for HIPS at source frames 18/20/22**
on a temporary proof asset, leaving every other target channel and all runtime
plumbing unchanged. If that first target correction removes the torso twist on
the actual skin, extend the same explicit source-delta × target-rest formula
to the focused arm/leg channels in another three-frame proof. Do not regenerate
the full 60-frame asset until that review gate passes.

Updated gate status:

```text
Swingv3 runtime identity:                 PASS
BOTTOM runtime selection:                 PASS
Exporter/compiler/runtime-FK transport:   PASS
Bind translations/hierarchy continuity:   PASS
Source→CoH Male retarget orientation:     FAIL (CASE 1: rest basis)
BOTTOM rotation/pose fidelity:             FAIL
BOTTOM overall visual gate:               FAIL
ASCEND:                                   HOLD (not tested)
Physics:                                  FROZEN
```

Do not merge PR #37, do not rebase, and do not make a speculative production
retarget or runtime animation change from this evidence alone. STOP FOR
SOL/JOSH REVIEW.

---

## Evidence artifacts

| Path | Purpose |
|---|---|
| `docs/evidence/issue36-forensic-20260823/STATIC_PROOF_*.jpg` (×3) | Phase-2 A front/¾/side closeups on the actual Male skin |
| `docs/evidence/issue36-forensic-20260823/FULL_FRAME30_*.jpg` (×3) | same for the matched `FULL@30` pose |
| `docs/evidence/issue36-forensic-20260824/BOTTOM_FRAME18..22_{front,threequarter,side}.jpg` (×15) | frozen actual-Male-skin constituent-pose gate for BOTTOM source frames 18..22 |
| `docs/evidence/issue36-forensic-20260824/SWINGV3_BOTTOM_FRAME18..22_{front,threequarter,side}.jpg` (×15) | corrected deciding gate on the exact runtime-proven `Swingv3` Male character; supersedes the earlier Phase-9 deciding set |
| `docs/evidence/issue36-forensic-20260824/pose-correspondence/FRAME18..22_raw-vs-retarget-vs-runtime.jpg` (×3) | frame-locked raw Mixamo / Blender Male / exact CoH runtime comparison sheets |
| `docs/evidence/issue36-forensic-20260824/pose-correspondence.md` | source/frame mapping, visual classification, focused runtime numeric correspondence, and current gate decision |
| `docs/evidence/issue36-orientation-20260824/orientation-report.json` | machine-readable full-basis source/control/CoH A/B, rest offsets, roll references, sign continuity, and rotation-only assertions |
| `docs/evidence/issue36-orientation-20260824/orientation-report.md` | compact 18/20/22 roll/basis tables, bend-plane audit, and visual links |
| `docs/evidence/issue36-orientation-20260824/visual/frame18..22/{raw,current,rest}/{front,threequarter,side}.png` (×27) | raw Mixamo, current CoH Male A, and rest-basis CoH Male B proxy views |
| `docs/evidence/issue36-rest-basis-bottom-20260824/` | corrected five-sample rest-basis runtime proof, identity record, contact sheet, and 15 actual-skin canary captures |
| `docs/evidence/issue36-forensic-20260823/static-vs-full.md` | full Phase-3 per-bone `full@30 vs proof(sample 1)` table (`1.7e-06 °`) |
| `docs/evidence/issue36-forensic-20260823/phase4-chain.md` | Phase-4 spine+arm `bind@f0 / authored-constant` table |
| `agent/work/issue36-forensic-20260823/runtime/proof.json` | `GetAnimation2 -runtime-rig MALE/COHSOURCEDEV_RETARGET_POSE_PROOF` |
| `agent/work/issue36-forensic-20260823/runtime/full.json` | `…_SWING_FULL` |
| `agent/work/issue36-forensic-20260823/runtime/ironkick.json` | stock `AIR_MA_IRONKICK` reference |
| `agent/work/issue36-forensic-20260823/runtime/skelready2.json` | `SKEL_READY2` base reference |
| `agent/animation/canary-static-proof.inc` | tracked `STATIC_PROOF` canary variant (`Scale 0`) |
| `agent/animation/canary-full-frame30.inc` | tracked `FULL_FRAME30` canary variant (`30 31 Scale 0`) |
| `agent/stage-issue36-canary.ps1` | variants and generated one-frame slices ↔ `bin/data/sequencers/cohsourcedev_canary.inc` |
| `agent/compare-issue36-static-vs-full.py` | Phase-3 comparator (proof `sample 1` vs full frames, tolerances `0.1 °`/`6e-05`) |
| `agent/animation/render_issue36_production_correspondence.py` | clean frame-locked source/target skeleton renderer; hides stale proof proxies and applies the saved display alignment |
| `agent/animation/make_issue36_comparison_sheets.py` | compact three-view comparison-sheet generator |
| `agent/animation/diagnose_issue36_orientation.py` | diagnostic-only full-basis/rest-basis A/B runner for frames 18/20/22 |
| `agent/animation/extract_issue36_correspondence.py` / `compare_issue36_correspondence.py` | production target extraction and focused exporter/runtime correspondence comparator |
| `agent/logs/build-Release-x86-20260823-*.log` | build evidence |
| `agent/logs/smoke-directdb-20260823-*.json` | direct-DB smokes |
| `agent/logs/smoke-directdb-20260823-191048.json` | post-recovery direct-DB timeout proving runtime validation was blocked |
| `agent/logs/webswing-smoke-20260823-152122.json` | movement smoke with `BOTTOM_ONLY` |
| `bin/logs/game/webswing.log` | `selectedMove`, `move_compare … AnimP=…`, `anim_selection_mode`, `overlay_moves_present=5/5` |

The runtime screenshots remain the deciding actual-skin evidence for identity,
selection, attachment, and gross continuity. The new Blender proxy sheets are
diagnostic source/retarget evidence only; they are not substituted for the
game's own skinned `TypeGfx=male` visual gate.

---

## Historical result (updated after Phase 8; superseded by Phase 11)

**Current validation status:** the parser crash fix, warmed shard recovery,
client/server mode-2 agreement, Male BOTTOM runtime selection, and SAFE_NONE
mode-0 control all pass. The earlier Phase 7 loader blockage is historical and
is superseded by Phase 8.

* Runtime-FK representation / bind hierarchy: **PASS**.
* Static Mixamo source frame 30 → actual Male skin: **PASS** after corrected
  Move-level `Scale 0` freeze recheck (front closeups re-shot; `1.7e-06 °`
  numeric).
* Full 60-frame temporal anatomy: **NOT PROVEN** —
  only the single frame-30 pose has been visually gated on the actual skin.
* CLIENT/SERVER mode-2 runtime agreement: **PASS**.
* BOTTOM `18 22` dynamic runtime selection: **PASS** — both sides repeatedly
  reached Male `BOTTOM` and selected `WEBSWING_BOTTOM`.
* Corrective exact-character BOTTOM constituent-pose gate on `Dummy00009` /
  `Swingv3`, frames `18..22`, front/¾/side: **PASS** — the earlier Phase-9
  deciding set is superseded for this result because it did not prove the
  required `Swingv3` Male runtime identity.
* SAFE_NONE mode-0 control and traversal: **PASS**.

Do not claim the entire 60-frame animation is visually skeletal-valid yet.
The numeric `full@30` comparison remains valid
(`~0.000002°` max rotation, position `0`, parent graph/bind translations
identical).

**Historical pre-orientation result (superseded):** the earlier static A/B and
its exact numeric cross-check passed for the single gated pose plus the live
BOTTOM selection. AIR_MA_IRONKICK and old V2 STRETCH were confirmed
contaminants of Josh's failed footage. The Phase-11 full-basis diagnostic now
supersedes that broad interpretation for pose fidelity and identifies the
source→Male rest-basis conversion as the current failure.

**One-phase reintegration staged** — `WEBSWING_BOTTOM Male 18 22`
(`MALE/COHSOURCEDEV_RETARGET_SWING_FULL`) is the sole custom visual move
beyond the canary; `AIRBORNE`/`ATTACHED`/`DESCEND`/`ASCEND` stay
animation-neutral and Fem/Huge stay fully neutral in mode `2`.

## Historical result (Phase 11, superseded by Phase 12)

* Runtime identity (`Dummy00009` / `Swingv3` Male): **PASS**.
* BOTTOM runtime selection and client/server agreement: **PASS**.
* BOTTOM bind hierarchy, attachment continuity, and gross detachment/stretch
  check: **PASS**.
* BOTTOM rotation/pose fidelity: **FAIL / under investigation**.
* BOTTOM overall visual gate: **FAIL** — the exact `Swingv3` BOTTOM pose is
  visibly tangled/corkscrewed in the current visual review.
* First actionable target bone: **`HIPS`**. Its segment direction error is
  `5.98°`, but its axial roll error is `90.00°` and its full-basis error is
  `90.16°`; source→control already first loses roll at `shoulder_r`/`COL_R`.
* Primary classification: **CASE 1 — REST BASIS IS THE PROBLEM**. The measured
  HIPS/arm/leg rest pairs contain approximately 90°/180° offsets, and the
  explicit rest-basis B removes the target basis error and proxy corkscrew at
  frames 18/20/22. The shared plane is a contributing mechanism, not a
  separately proven primary case.
* Roll-reference sign continuity: **PASS** for the tested window; no
  18→20/20→22 reference dot is negative.
* Exporter/compiler/runtime-FK transport: **PASS**; runtime-local errors remain
  below `0.1°` and frame mapping/bind translations/hierarchy remain cleared.
* Full 60-frame temporal anatomy: **NOT PROVEN**.
* ASCEND: **HOLD**; no ASCEND or physics work was performed.

Do not merge PR #37 and do not make a speculative production animation fix
from this evidence alone. The smallest next pilot, after Sol/Josh approval,
is a temporary three-frame HIPS-only rest-basis proof at 18/20/22; if that
passes on the actual skin, extend the same formula to the focused arm/leg
channels before considering any full-60 regeneration.

**STOP FOR SOL/JOSH REVIEW — do not merge PR #37.**

---

## Phase 12 — Corrected rest-basis BOTTOM canary, source frames 18..22 (2026-08-24)

This phase implements the accepted rest-basis transfer as a bounded proof asset
only.  It supersedes the Phase-11 visual FAIL for the old source→Male
orientation result; it does not replace the production 60-frame asset.

```yaml
"source->Male retarget orientation fix direction": confirmed
"corrected frame20 actual-skin": PASS
"corrected bottom 18..22 actual-skin": PASS
"full 60-frame animation": NOT PROVEN
"ASCEND 30..40": NOT IMPLEMENTED
```

The temporary runtime asset is
`MALE/COHSOURCEDEV_RETARGET_RESTBASIS_BOTTOM`.  It contains exactly five
authored samples, mapping runtime samples `1..5` to source frames `18..22`.
The generator applies the accepted source-pose-delta × target-rest-basis
transfer to the torso, both arm chains, both leg chains, and head/neck while
keeping bind translations fixed, pose-bone translations at zero, and scale
constant.

### Pre-runtime and runtime proof

The exact `MALE/SKEL_READY2` 68-bone target passed Blender pre-export proof:

* focused-bone direction error: `0.0°`;
* maximum roll error: `0.002097°`;
* maximum full-basis/runtime-world error: `0.039565°`;
* local translation magnitude: `0.0`;
* local scale error: `3.5763e-7`.

ANIMX export, `GetAnimation2` compile, and runtime decode also passed:

* runtime animation: `MALE/COHSOURCEDEV_RETARGET_RESTBASIS_BOTTOM`;
* base animation: `MALE/SKEL_READY2`;
* five authored samples, 68 tracks, hierarchy mismatch `0`;
* bind translation error `0.0`, authored translation drift `0.0`;
* maximum decoded runtime-local orientation error `0.072206°` on `ULEGL`;
* failed runtime proof checks: `0`.

### Actual-skin decision

The existing canary path was used with `Dummy00009` / `Swingv3`.  The test
identity was `seq_type=male`, `calculated_type=male`, `is_male=1`,
`TypeGfx=male`; the canary selected
`MALE/COHSOURCEDEV_RETARGET_RESTBASIS_BOTTOM`.  All 15 captures exited cleanly:
front, three-quarter, and side for each source frame `18`, `19`, `20`, `21`,
and `22`.

The enlarged review set shows a coherent compressed swing tuck through the
range: torso/hip relationship is coherent, limbs remain connected to the same
body, and no visible corkscrew, detached segment, or extreme stretch appears.
The deciding result is:

```text
CORRECTED BOTTOM 18..22 ACTUAL-SKIN PILOT: PASS
```

Detailed proof reports and all 15 actual-skin images are in
`docs/evidence/issue36-rest-basis-bottom-20260824/`.

### Scope boundary and handoff

The production `MALE/COHSOURCEDEV_RETARGET_SWING_FULL` asset was not
overwritten.  No ASCEND, physics, anchors, rope, steering, jump, tether,
WebSwing mode plumbing, system/shard forensic, or exporter/runtime-FK
reopening was performed.  Full 60-frame animation validity remains **not
proven**.

Next task recommendation, without implementing it here:

```text
ASCEND 30..40 with HOLD at 40
```

PR #37 remains a stacked draft.  Do not merge or rebase.  STOP FOR SOL/JOSH
REVIEW.

## Phase 13 — Normal overlay, corrected BOTTOM live mode-2 handoff (2026-08-24)

The narrow production reference now selects the corrected five-sample rest-basis
asset for the Male `WEBSWING_BOTTOM` move:

```text
MALE/COHSOURCEDEV_RETARGET_RESTBASIS_BOTTOM 1 5
```

The tracked runtime manifest contains the 3,251-byte asset with SHA-256
`e1cb732169ba920b03d164ce062ec0a18d80099cba96184f082ba29d3512e9b2`.
`agent/install-webswing-animation.ps1 -Action Status` verified all 13 tracked
assets, the normal overlay, and the corrected asset in the runtime directory.
The normal overlay is installed with `canaryModeInstalled=false` and no
`cohsourcedev_canary.inc`; no `-animcanary` run is part of this handoff.

### Build and live technical evidence

* `agent/build.ps1 -Configuration Release -Platform x86` passed using the
  verified v145 fallback.
* The current GUI handoff is Ouroboros PID `6476`, window title `Atlas Park`,
  character `Swingv3`, with `-webswinganim 2` in its command line.
* At `14:47:14`, that live client reported
  `anim_selection_mode=2 custom_move_selection=MALE_BOTTOM_ONLY`; the live
  MapServer reported the same mode. The client also reported
  `selected_source=COMPILED_OVERLAY` with all five Web Swing moves present.
* The same-day `Swingv3` live swing trace reached Male `BOTTOM` at `14:21:34`
  and selected `WEBSWING_BOTTOM` before transitioning to `ASCEND` in
  `bin/logs/game/webswing.log`; the MapServer recorded the corresponding Male
  `BOTTOM` state in `bin/logs/mapserver/webswing.log`.

The log evidence proves technical mode-2 selection and the corrected normal
overlay being available. It does not decide the appearance or quality of the
motion in the live GUI; that remains Josh's visual review gate.

```text
REST-BASIS FRAME20 ACTUAL SKIN: PASS
REST-BASIS BOTTOM 18..22 CONSTITUENT POSES: PASS
REST-BASIS BOTTOM LIVE MODE2 TECHNICAL SELECTION: PASS
REST-BASIS BOTTOM LIVE VISUAL MOTION: AWAITING JOSH
```

PR #37 remains a stacked draft. Do not merge or rebase. Leave the GUI running
for Josh.
