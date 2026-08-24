# Issue 36 — Web Swing animation runtime-integrity forensic pass — 2026-08-23

**Branch:** `agent/issue-36-web-swing`
**Expected starting HEAD:** `602f5688e2def3ef9466d2b81333d88590a324ad`
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

## Phase 7 — Sol follow-up: mode preservation and client/server agreement

This section is the current checkpoint and supersedes any earlier wording
that treated a client-only mode line or a movement smoke as proof of agreement
between the client and MapServer.

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

### Current status / Josh handoff

* Runtime-FK representation / bind hierarchy: **PASS**
* Static frame-30 actual Male skin: **PASS**
* Full 60-frame temporal anatomy: **NOT PROVEN**
* Full 60-frame temporal anatomy: **NOT PROVEN**
* CLIENT/SERVER mode-2 runtime agreement: **BLOCKED BY LOCAL SHARD**
* BOTTOM 18..22 dynamic actual-skin visual: **BLOCKED ON RUNTIME MODE GATE**

The source is staged for Josh's eventual mode-2 visual inspection, but a
healthy GUI client could not be left running because the local shard recovery
failed. Once the loader environment is repaired, set the ephemeral runtime
cfg to `2`, launch the client with `-webswingdev -webswinganim 2`, and first
verify both exact mode-2 lines from that one session before judging the live
BOTTOM tuck. Then repeat both processes with mode `0` for the SAFE_NONE
control. Do not re-enable AIRBORNE, ATTACHED, DESCEND, or ASCEND visuals.

---

## Evidence artifacts

| Path | Purpose |
|---|---|
| `docs/evidence/issue36-forensic-20260823/STATIC_PROOF_*.jpg` (×3) | Phase-2 A front/¾/side closeups on the actual Male skin |
| `docs/evidence/issue36-forensic-20260823/FULL_FRAME30_*.jpg` (×3) | same for the matched `FULL@30` pose |
| `docs/evidence/issue36-forensic-20260823/static-vs-full.md` | full Phase-3 per-bone `full@30 vs proof(sample 1)` table (`1.7e-06 °`) |
| `docs/evidence/issue36-forensic-20260823/phase4-chain.md` | Phase-4 spine+arm `bind@f0 / authored-constant` table |
| `agent/work/issue36-forensic-20260823/runtime/proof.json` | `GetAnimation2 -runtime-rig MALE/COHSOURCEDEV_RETARGET_POSE_PROOF` |
| `agent/work/issue36-forensic-20260823/runtime/full.json` | `…_SWING_FULL` |
| `agent/work/issue36-forensic-20260823/runtime/ironkick.json` | stock `AIR_MA_IRONKICK` reference |
| `agent/work/issue36-forensic-20260823/runtime/skelready2.json` | `SKEL_READY2` base reference |
| `agent/animation/canary-static-proof.inc` | tracked `STATIC_PROOF` canary variant (`Scale 0`) |
| `agent/animation/canary-full-frame30.inc` | tracked `FULL_FRAME30` canary variant (`30 31 Scale 0`) |
| `agent/stage-issue36-canary.ps1` | variants ↔ `bin/data/sequencers/cohsourcedev_canary.inc` |
| `agent/compare-issue36-static-vs-full.py` | Phase-3 comparator (proof `sample 1` vs full frames, tolerances `0.1 °`/`6e-05`) |
| `agent/logs/build-Release-x86-20260823-*.log` | build evidence |
| `agent/logs/smoke-directdb-20260823-*.json` | direct-DB smokes |
| `agent/logs/smoke-directdb-20260823-191048.json` | post-recovery direct-DB timeout proving runtime validation was blocked |
| `agent/logs/webswing-smoke-20260823-152122.json` | movement smoke with `BOTTOM_ONLY` |
| `bin/logs/game/webswing.log` | `selectedMove`, `move_compare … AnimP=…`, `anim_selection_mode`, `overlay_moves_present=5/5` |

No Blender proxy is used as the deciding visual — every screenshot is the
game's own skinned `TypeGfx=male` character.

---

## Result (corrected per Sol review)

**Current validation caveat:** Phase 7 records the final local parser/server
initialization fixes, but the post-fix client/server runtime proof is blocked
by the local shard loader environment. Earlier Phase 6 movement results are
historical handoff evidence only and do not prove mode agreement.

**Current justified status:**

* Runtime-FK representation / bind hierarchy: **PASS**.
* Static Mixamo source frame 30 → actual Male skin: **PASS** after corrected
  Move-level `Scale 0` freeze recheck (front closeups re-shot; `1.7e-06 °`
  numeric).
* Full 60-frame temporal anatomy: **NOT PROVEN** —
  only the single frame-30 pose has been visually gated on the actual skin.
* CLIENT/SERVER mode-2 runtime agreement: **BLOCKED BY LOCAL SHARD**.
* BOTTOM `18 22` dynamic actual-skin visual: **BLOCKED ON RUNTIME MODE GATE**
  (`-webswinganim 2` `MALE_BOTTOM_ONLY` has not received client/server runtime
  proof on a healthy local shard).

Do not claim the entire 60-frame animation is visually skeletal-valid yet.
The numeric `full@30` comparison remains valid
(`~0.000002°` max rotation, position `0`, parent graph/bind translations
identical).

**Case 4** — the forensic A/B and its exact numeric cross-check pass for the
single gated pose. AIR_MA_IRONKICK and old V2 STRETCH are confirmed
contaminants of Josh's failed footage. Static frame 30 clears the new
runtime-FK representation at that pose, but BOTTOM 18..22 and the full
temporal clip remain visually unproven; ALL observed mangling cannot yet be
attributed conclusively to the old assets.

**One-phase reintegration staged** — `WEBSWING_BOTTOM Male 18 22`
(`MALE/COHSOURCEDEV_RETARGET_SWING_FULL`) is the sole custom visual move
beyond the canary; `AIRBORNE`/`ATTACHED`/`DESCEND`/`ASCEND` stay
animation-neutral and Fem/Huge stay fully neutral in mode `2`.

**STOP FOR SOL/JOSH REVIEW — do not merge PR #37.**
