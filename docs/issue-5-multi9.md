# Issue #5: Multi9 GLSL pilot verification

Evidence captured on 2026-08-16 from `agent/glsl-pilot-materials` after the
Multi9 pilot initializer fix. Numeric program IDs below are build-local and
come from the runtime shader-manager diagnostics; the symbolic `BMB_*` flags
are the source of truth.

## Supported coverage matrix

| Symbolic variant | `BMB_*` flags | Fragment ID | Pilot vertex kind / ID | Status |
| --- | --- | ---: | --- | --- |
| Full LQ | `BMB_DEFAULT` | 120 | `kPilotVertexKind_BumpMulti` / `bump_dual_multi` / 234 | Verified active |
| Full HQ | `BMB_HIGH_QUALITY` | 121 | `kPilotVertexKind_BumpMultiHQ` / `bump_dual_multi HQ` / 253 | Implemented; unverified in the static pilot pairing |
| Single LQ | `BMB_SINGLE_MATERIAL` | 136 | `kPilotVertexKind_BumpMulti` / `bump_dual_multi` / 234 | Verified active |
| Single HQ | `BMB_SINGLE_MATERIAL \| BMB_HIGH_QUALITY` | 137 | `kPilotVertexKind_BumpMultiHQ` / `bump_dual_multi HQ` / 253 | Implemented; unverified in the static pilot pairing |
| Building | `BMB_BUILDING` | 152 | `kPilotVertexKind_BumpMulti` / `bump_dual_multi` / 234 | Verified active |

The runtime log registered the expected static vertex IDs as `234` and `253`.
The supported LQ variants activated through ID `234` in the Founders Falls
capture. The HQ fragment IDs were also reached, but the current static view
paired them with generated vertex ID `255`, the skinned multitex path, so the
pilot correctly declined them and left them on ARB/Cg. No claim is made that
the HQ static vertex ID `253` was reached by this view.

Intentionally ARB/Cg-only pairings remain:

- `DRAWMODE_BUMPMAP_MULTITEX_RGBS` / RGBS baked-lighting multitex;
- `DRAWMODE_BUMPMAP_SKINNED_MULTITEX` / skinned multitex (the observed HQ
  fallback was vertex ID `255`);
- `BMB_CUBEMAP`, `BMB_PLANAR_REFLECTION`, and `BMB_SHADOWMAP` Multi9
  permutations outside the five registered pilot targets.

## Deterministic runtime view

`FoundersCanal_01` is the static map 10 view used for Multi9 coverage:

```text
position  4497.49 60.00 991.49
camera    pitch 0.2500, yaw 0.7854, roll 0.0000, distance 30
time      16
features  shaderDetail=3, useWater=2, multi=1
```

The Atlas Plaza CityHall/East/North/West views were also run through the
representative pilot suite; they do not bind Multi9. FoundersCanal is the
deterministic static-map view that binds the five fragment IDs above.

## Formal parity evidence

The control image was created explicitly with baseline adoption; adoption was
not counted as a pass. Formal pilot comparisons used the hardened comparator
(`pixelTolerance=12`, `changedPercent` hard limit 6%, `meanDelta` advisory).

| Capture | Control baseline | Pilot result | `changedPercent` | `meanDelta` | Verdict |
| --- | --- | ---: | ---: | ---: | --- |
| FoundersCanal_01 | `regression-20260816-223140.json` | `regression-20260816-223221.json` | 0.6956% | 1.3807 | PASS |
| FoundersCanal_01 follow-up | same control | `regression-20260816-223425.json` | 0.6268% | 1.3010 | PASS |

The two PASS results cover the three deterministically active supported
variants: Full LQ, Single LQ, and Building. The two HQ variants remain
implemented-but-unverified for the static pilot pairing because the only HQ
pairing reached in the view is the intentionally unsupported skinned path;
their renders completed successfully through ARB/Cg fallback.

Representative existing pilot regression also remained green: the four-shot
Atlas Plaza suite passed all four shots in
`regression-20260816-223512.json` (changedPercent 0.2436%, 0.0194%, 1.5095%,
and 0.0071%). The reported meanDelta advisories were not hard failures under
the #4 policy.

## Evidence-backed fix

The five Multi9 `tPilotMaterial` positional initializers had only one
brace-wrapped six-element location array. The trailing values consequently
shifted into the boolean fields and set `failed=true`, so the pilot skipped
every Multi9 target before activation. The fix adds the missing
`locFxFor[6]` braces to those five initializers. No Multi9 shader math or new
permutations were added.

The pilot-gated startup diagnostic now prints the resolved target matrix:

```text
GLSL pilot: Multi9 targets full=120 fullHQ=121 single=136 singleHQ=137 building=152
```

The fallback boundary was exercised in the same capture:

```text
GLSL pilot: BLENDMODE_MULTI HQ bind declined, vertex program 255 not registered
GLSL pilot: BLENDMODE_MULTI single HQ bind declined, vertex program 255 not registered
```

## Build and smoke checks

- `agent/build.ps1 -Configuration Release -Platform x86`: PASS, v145 fallback;
  final log `agent/logs/build-Release-x86-20260816-222649.log`.
- `agent/smoke.ps1 -ExerciseCharacter -AccountName Dummy00009 -TimeoutSeconds
  180`: PASS after the documented shard warm-up; character creation and
  MapServer entry verified in `smoke-directdb-20260816-223129.json`.
