# Remaster Profile v1 — Defaults That Move The Screenshot

Branch: `agent/remaster-profile-v1` (base `agent/atlas-statue-geometry-v1`, start `020a2801`).
Date: 2026-08-21. All runtime evidence in this document was produced on an
isolated parallel shard from this worktree (see "Runtime isolation" below).

## What shipped

Remaster Profile v1 makes the game look noticeably better at stock defaults by
composing capabilities the renderer already has, instead of adding new systems:

| Setting | Previous default (Recommended) | Remaster v1 default |
|---|---|---|
| shadowMode | SHADOW_STENCIL (stencil blob shadows)* | SHADOW_SHADOWMAP_HIGH |
| shadow shader / size / cascades | — | HIGHQ / 1024 / 3 (middle distance) |
| SSAO | disabled | strength HIGH, resolution HIGH_QUALITY (full-res AO), blur BILATERAL_DEPTH |
| Water | WATER_MED | WATER_HIGH |
| modernPresentation | 0 | 1 (filmic shoulder in the GLSL final pass) |
| modernBloom | 0 | 1 (soft-knee energy-limited bloom) |
| Bloom / DOF / desaturate / aniso / mip | REGULAR / on / on / 4 / -1 | unchanged |
| Cubemap | OFF | OFF (kept; see rationale) |
| modernMaterials / modernLighting | 0 | 0 (kept opt-in, per scope) |

\* Runtime note: on a **clean** registry the old builds actually came up at
`SHADOW_SHADOWMAP_LOW` because `gfxSettingsFixup()`'s legacy version-4
"upgrade" forced it; see "Fixups" below.

Architecture: the remaster composition is the new nominal
`gfxGetRecommendedAdvancedSettings()` on the capable-hardware class (the same
`DX10_CLASS || (GLSL && ARBFP)` chip test the preset ladder already uses);
weaker hardware keeps the legacy values, and `gfxSettingsApplyRestrictions()`
still clamps anything the current driver cannot do. Performance/Minimum and
safe mode are untouched; Performance explicitly resets shadow/ambient so the
remaster base cannot leak into it. There is no new preset slot, so existing
saved `graphicsquality` slider values map exactly as before. The presentation
flags stay runtime commands (`modernPresentation 0` etc.), preserving the
exact previous-presentation control.

Rationale for keeping cubemaps off: the measured A/B showed the
water-HIGH+cubemap-LOW combination darkened the City Hall view by ~22 mean
luminance with no compensating win at Founders' Falls, cubemap reflection
cost is a full extra scene pass per updated face, and the audit had already
ranked cubemaps low-leverage. Water HIGH is kept (matches the Quality preset,
directly improves the Founders' Falls canals).

## Runtime isolation (concurrent-agent support)

The shared shard runs another agent's branch whose wire protocol has drifted
(SCMD enum insertions, ServerControlState bitfield insertions, loose
sequencer data), which deterministically crashed this branch's clients during
entity receive (`entrecv.c` access violations). Rather than touching the
shared runtime, this worktree now runs a fully isolated parallel shard:

- `Common/comm_backend.h`: every default port shifts by `COH_PORT_OFFSET`
  (env var, default 0 = historical ports unchanged). One header change; all
  bind/connect sites derive from it, including the mapserver port scan.
- `agent/start-shard-parallel.ps1` / `agent/stop-shard-parallel.ps1`:
  spawn/stop a shard from **this worktree's bin only** (all matching is by
  executable path, so another agent's processes are never touched). They
  bypass ServerMonitor because its process monitor matches DbServer/Launcher
  by exe name system-wide and would see the other agent's processes as its
  own.
- `bin/data/server/db/servers.cfg` (local, uncommitted): `SqlDbName cohgfx`
  (fresh database created on first run) and `DefaultAccessLevel 9` for the
  capture accounts. Aux DBs `cohchat`/`cohauc`/`cohacc` were created empty;
  like on the main shard those services run degraded (chat/auction SQL
  schema is not provisioned) and this does not block gameplay or captures.
- Clients (capture/smoke/Ouroboros) connect with the same
  `COH_PORT_OFFSET` set in their environment.

First-run note: a fresh `cohgfx` database makes DbServer's static-map
startup pass take ~25 minutes (one-time schema/table init); subsequent
restarts reach "DbServer Ready" in ~5 s. The game-client listener is UDP
(`netInit(list, udp_port, tcp_port)` — the client port is the **UDP** slot),
so TCP port checks will not show it.

## Verified previous default (Phase 0A)

Clean file-backed registry, fresh launch, FEATTRACE + registry dump after
clean exit (`shaderDetail=3` is `SHADERDETAIL_HIGHEST_AVAILABLE`; the enum is
not ordered low-to-high):

```
graphicsquality=0.500000 (Recommended)  shaderdetail=3   usewater=2 (MED)
shadowmode=2 (see fixup note)  ambientstrength=0 (SSAO off)  cubemapmode=0
usebloom=1 (REGULAR)  bloommagnitude=1.0  usedof=1  usedesaturate=1
texaniso=4  antialiasing=0  miplevel=-1  glslPilot=1
modernPresentation=0  modernBloom=0  modernMaterials=0  modernLighting=0
```

Quality/Ultra effective deltas (verified in source + `gfxUpdateShadowMapAdvanced`/
`gfxUpdateAmbientAdvanced` expansion): Quality = shadowmap LOW (FAST shader!,
1024, 3 cascades) + SSAO HIGH_PERFORMANCE (half-res, fast blur, LOW strength)
+ water HIGH + cubemap LOW; Ultra = shadowmap HIGH (HIGHQ, 1024, 4 cascades
far) + SSAO HIGH_QUALITY (on DX10-class: HIGH strength, trilateral,
quality-res) + water ULTRA + cubemap HIGH.

## Persistence fix (requested vs effective)

Two poisoning classes existed, both confirmed in source and one documented in
AGENTS.md history (a run that lost GFXF_MULTITEX permanently saved
`shaderdetail=0`):

1. `gfxApplySettings()` applied `gfxSettingsApplyRestrictions()` to the
   caller's struct and then saved that struct, so a launch with temporarily
   degraded capabilities (e.g. FBO-less `-useFBOs 0`, safe-mode-adjacent
   states) permanently wrote the clamped values as the user's preference.
2. `gfxGetSettings()` derived shaderDetail/bloom/DOF/desaturate/ambient
   strength from **current** `rdr_caps.features`, so a transiently degraded
   run could push feature-derived "off" values into the saved requested
   settings through every re-apply path (options UI, resolution fallbacks in
   win_init, renderer-finalize reapply).

Fix (`gfxSettings.c`, `autoResumeInfo.c`):

- `gfxApplySettings()` keeps an unclamped snapshot and stores **that** in
  `globalGfxSettingsForNextTime` (what gets persisted); the restricted copy
  is still what runs this launch.
- `gfxGetSettings()` reads the capability-sensitive quality fields
  (shadowMode, cubemap, water, shaderDetail, bloom/DOF/desaturate, ambient
  strength/res/blur) from the requested store when filled, so UI/reapply
  flows cannot demote saved preferences from transient runtime state. The
  trailing `gfxSettingsApplyRestrictions()` in `gfxGetSettings()` was removed
  (every apply path restricts on its own).
- Clean-registry runs no longer run the legacy version-4 "upgrade"
  (the defaults now keep `version = GFXSETTINGS_VERSION`), which had been
  forcing fresh installs to `SHADOW_SHADOWMAP_LOW`.

Registry format and explicit user changes are unchanged; safe mode still
skips saving the advanced set.

**Pass-gate results (all on the final binary):**

| Gate | Result |
|---|---|
| clean first run → remaster active → clean exit → relaunch | PASS (registry after relaunch: shadowmode=4, ambient 3/3/5, usewater=3, version=5) |
| degraded launch (`-useFBOs 0`: no FBOs → shadow/SSAO/bloom clamped at runtime) → exit → normal relaunch | PASS (saved requested remaster intact: shadowmode=4, ambientstrength=3, usewater=3) |
| explicit user change (shadowmode=1 stencil) → run → exit | PASS (user's 1 persists; unrelated remaster fields keep their values) |

## Visual A/B evidence

Scenes: the established capture identities — `AtlasPlaza_CityHall_03`,
`AtlasPlaza_East_01` (day), `AtlasPlaza_NightEast_01` (night),
`FoundersCanal_01` (map 10). "Stock" = previous-default capture set from the
pre-change binary (stencil shadows + fixup-LOW, no SSAO, stock presentation);
"Remaster" = clean-registry default run of the final binary (no overrides).
Comparison via `agent/compare-captures.ps1` (tolerance 12):

| Scene | changedPercent | meanDelta | Notes |
|---|---|---|---|
| AtlasPlaza_CityHall_03 | 22.6% | 12.0 | whole-frame: plaza shadowing, contact AO, filmic rolloff |
| AtlasPlaza_East_01 | 26.5% | 12.7 | previously a weak-signal (sky-heavy) scene; still clearly moves |
| AtlasPlaza_NightEast_01 | 4.5% | 4.7 | see night caveat below |
| FoundersCanal_01 | 69.6% | 37.1 | canal AO contacts, shadow maps, WATER_HIGH reflections |

A same-day exposure-adaptation freeze makes `changedPercent` bounce upward
when the two runs lock different exposure phases (an earlier same-settings
pair measured 99.9%/mean 38 for a ~10-level uniform offset); the numbers
above are the honest mid-range, and both exceed the 8–15% "screenshot-moving"
band on the day scenes with no clipping pathologies (clip-high 0% everywhere;
clip-low ≤1.5% at Founders AO contacts).

Luminance sanity (mean / ground-mean, stock → remaster): CityHall
131.4/118.2 → 121.2/112.5; East 134.0/121.7 → 132.1/116.8; Founders
65.2/47.8 → 42.5/26.4 (shadowing+AO ground the canals; not crushed);
night 22.6 → 20.2.

Images: `agent/captures/remaster-ab/{stock,remaster-final2}/<scene>.jpg` and
the side-by-side `agent/captures/remaster-ab/remaster-ab-contact-sheet.jpg`.

**Night caveat:** on this worktree's shard the hour-0 night scene renders
~57% near-black in **all** configurations, including stock, while the
committed baseline (`agent/captures/AtlasPlaza_NightEast_01.jpg`, mean 110)
is a bright moonlit city. `timeset`/`timescale` were verified working
(repeated day captures stay day; repeated night stays night), the client code
is identical, and no moon-phase system exists in `sun.c` — the difference is
environment state of the fresh shard/database (world date/weather), not the
graphics profile. Night deltas between stock and remaster are therefore
small on this shard (the scene is mostly black); the modernPresentation
darker-night tendency documented in issue #13 remains the thing to re-check
on the primary shard environment.

## GLSL coverage with the remaster active

From the remaster capture stdout (pilot coverage diagnostics, logged once
per distinct pairing):

- Native GLSL dominates: >10,000 `GLSL pilot: ... active` lines per
  Founders capture (MODULATE, MULTIPLY, water, multi families).
- Expected synchronized fallback (unchanged from the issue #11 inventory):
  `BUMPMAP_COLORBLEND_DUAL variant=BMB_HIGH_QUALITY|BMB_SHADOWMAP` and
  `MULTI variant=BMB_HIGH_QUALITY|BMB_SHADOWMAP` (skinned, HQ) — the
  shadow-map fragment variants were already classified
  `unported-fragment-variant`; the remaster simply draws them more often.
  One `MODULATE/skin_bump HQ` pairing declines on vertex pairing
  (`pilot-target-declined-on-vertex-pairing; legacy fragment request is
  synchronized`), also pre-existing.
- No rendering corruption observed in any capture (clean exits, screenshots
  produced; no visual anomalies flagged by the pixel stats).

Per the milestone rules these fallbacks are accepted as-is; nothing was
ported for coverage percentage.

## Performance

Methodology: steady-state in-world sampling. Two interleaved pairs
(stock → remaster → stock → remaster) on the parallel shard: each run
launches the client with autologin into Atlas Park, waits out the load
(45 s), then samples `nvidia-smi utilization.gpu` at 500 ms for 40 s and
closes the client. 1280x720 windowed, RTX 3060, concurrent with the other
agent's runtime (hence the paired design). Reproduction:
`agent/work/steady-perf.ps1` (run with `COH_PORT_OFFSET` set).

```
stock    meanGPU=27.0% (min 4,  max 96)
remaster meanGPU=32.6% (min 18, max 56)
stock    meanGPU=28.1% (min 3,  max 50)
remaster meanGPU=29.6% (min 14, max 68)
=> stock 27.6% vs remaster 31.1%: +3.5pp, 1.13x relative (pairs 1.21x / 1.05x)
```

The remaster's utilization floor is consistently higher (min 14-18 vs 3-4)
because the SSAO/shadow/reflection passes never idle; the mean sits well
under the ≤1.25x preferred target and far under the 1.40x kill threshold,
consistent with issue #25's ~14% relative for the depth features alone plus
the cheap presentation pass. Whole-capture-window utilization sampling was
abandoned as a method: it mixes startup/compile phases with the render
window and, combined with the intermittent map-entry race, produced
unreproducible numbers (documented in `agent/work/remaster-perf/`).

## Known issues surfaced (out of scope, documented for follow-up)

1. **Intermittent map-entry entity race**: under rapid capture cycling,
   clients occasionally crash in `entReceiveDeletes` (`entrecv.c:216/227`)
   or `tricks.c:814` — on the same-code shard too, so it is a pre-existing
   timing race, not branch drift. The other agent's client hit a nearby
   crash the same morning. Repro: ~25 rapid capture sessions; fix belongs
   to entity/comm, not this milestone.
2. **Cross-branch wire drift**: SCMD enum / ServerControlState insertions
   and loose sequencer overrides on the Web-Swing branch make that server
   binary incompatible with this branch's clients (deterministic
   `entrecv.c:216` crash). Mid-enum wire inserts should get protocol
   version bumps in the future.
3. **Night environment state** (above).
4. Aux servers (chat/auction/account) run SQL-degraded on the local
   development shard for want of schema provisioning; non-blocking for
   gameplay/captures.

## Reproduction

```powershell
# from D:\github\coh-graphics, with the shared shard untouched:
$env:COH_PORT_OFFSET = '2000'
.\agent\start-shard-parallel.ps1 -PortOffset 2000     # first run: ~25 min DB init
.\agent\smoke.ps1 -ExerciseCharacter -AccountName Dummy00009
.\agent\remaster-ab.ps1 -Label stock -NoModern         # previous-default set
# (for a true stock-default binary, build the parent commit)
Remove-Item -Recurse bin\registry-keys                 # clean first-run state
.\agent\remaster-ab.ps1 -Label remaster                # remaster defaults, no overrides
.\agent\remaster-perf.ps1 -Pairs 3                     # paired GPU A/B
```

Controls: `-modernPresentation 0 -modernBloom 0` (previous presentation),
`-glslPilot 0` (legacy renderer), registry keys for depth settings
(`agent/remaster-ab.ps1` writes/restores them).
