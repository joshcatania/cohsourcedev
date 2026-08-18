# Issue #13 — modern presentation v1 evidence

This is the evidence follow-up for the opt-in native-GLSL presentation curve at commit `468a2a8fa985300c16579e347cd77140239cf5a`. The committed visual artifact is the compact [OFF/ON contact sheet](evidence/issue-13-modern-presentation-contact-sheet.jpg); the full-resolution captures remain local under `agent/captures/issue13/`.

## Curve and scope

The modern branch retains the existing exposure calculation first:

```text
scaleCalc = (expectedMiddleLum - adaptedLum) * toneMapWeight * 0.4
exposure  = max(0, 1 + scaleCalc)
exposed   = color * exposure
```

For `modernPresentation 1`, each RGB component then uses the bounded shoulder:

```text
shoulder = max(exposed - expectedMiddleLum, 0)
filmic   = exposed / (1 + shoulder * 0.35)
result   = mix(exposed, filmic, clamp(toneMapWeight * 0.75, 0, 1))
```

The same operation is applied to the frame sample and blurred sample before the unchanged bloom selection/composite. `expectedMiddleLum` is the existing `.x` value from `calcExpectedLumParam()`, `adaptedLum` is the existing sampled adapted luminance, and `toneMapWeight` is the existing authored `.y` value. This gives a conservative highlight shoulder while retaining readable midtones and the current exposure input. The operation is applied independently per RGB component; it is **not mathematically hue-preserving**. In the supplied evidence I do not see a clear hue rotation or objectionable saturation shift, so no curve adjustment is made here.

No bloom extraction/composition, DOF, desaturation ordering, adaptation, material/world shader, water, ARB/Cg path, render pass, or texture allocation changed.

## Capture pairs

All names below are exact files in the local capture artifacts. The committed contact sheet uses the `paired-*` files.

| Scene | modern OFF | modern ON | Notes |
|---|---|---|---|
| Atlas day / City Hall | `paired-before-modern-off/AtlasPlaza_CityHall_03_modern-off.jpg` | `paired-after-modern-on/AtlasPlaza_CityHall_03_modern-on.jpg` | Stable day view; blue sky and character remain readable, with a restrained highlight reduction. |
| Atlas night | `paired-before-modern-off/AtlasPlaza_NightEast_01_modern-off.jpg` | `paired-after-modern-on/AtlasPlaza_NightEast_01_modern-on.jpg` | Emissive skyline remains visible; ON is darker in the unlit foreground. |
| Talos / AddGlow | `paired-before-modern-off/TalosArrive_01_modern-off.jpg` | `paired-after-modern-on/TalosArrive_01_modern-on.jpg` | Strong highlight/background change; framing differs between captures, so geometry is not used as a curve verdict. |
| Founders canal | `paired-before-modern-off/FoundersCanal_01_modern-off.jpg` | `paired-after-modern-on/FoundersCanal_01_modern-on.jpg` | Water and truck remain intact; ON is more restrained in bright cyan and pale surfaces. |
| Character close-up | `paired-before-modern-off/AtlasPlaza_Closeup_01_modern-off.jpg` | `paired-after-modern-on/AtlasPlaza_Closeup_01_modern-on.jpg` | Idle-animation/camera phase differs; useful for broad costume readability only, not pixel alignment. |

The alternate final warm-shard captures are also retained locally:

| Scene | modern ON final |
|---|---|
| Atlas day / City Hall | `after-modern-on-final/AtlasPlaza_CityHall_03_modern-on-final.jpg` |
| Atlas night | `after-modern-on-final/AtlasPlaza_NightEast_01_modern-on-final.jpg` |
| Talos / AddGlow | `after-modern-on-final/TalosArrive_01_modern-on-final.jpg` |
| Founders canal | `after-modern-on-final/FoundersCanal_01_modern-on-final.jpg` |
| Character close-up | `after-modern-on-final/AtlasPlaza_Closeup_01_modern-on-final.jpg` |

## Quantitative results

The OFF parity regression remained within the existing 6% hard limit:

| OFF regression shot | changedPercent |
|---|---:|
| CityHall | 2.0198% |
| East | 0.0212% |
| North | 1.4989% |
| West | 0.0141% |
| Founders | 1.22% |

Modern ON deltas are informational, not parity failures. They are measured at 320 px comparison width with the repository comparator (`pixelTolerance=12`):

| Pair | changedPercent | meanDelta |
|---|---:|---:|
| Atlas day / City Hall | 99.83% | 26.94 |
| Atlas night | 34.37% | 21.62 |
| Talos / AddGlow | 91.11% | 63.78 |
| Founders canal | 30.31% | 17.92 |
| Character close-up | 56.71% | 35.09 |

The large ON values are expected from a global presentation change; Talos and the close-up additionally contain capture-time framing/animation variance.

## Review observations

- Atlas day: the ON image rolls bright sky/surface values more gently without crushing the blue sky or costume.
- Atlas night: the ON image is visibly darker in the foreground. This is the one aspect that looks worse subjectively; skyline/emissive readability remains present, and the evidence does not show a hue failure.
- Talos/AddGlow: the ON image has less harsh bright background response. Because the pair is not geometrically identical, this is directional visual evidence only.
- Founders: bright cyan/water-adjacent areas and pale truck surfaces are less clipped; no obvious hue rotation is visible.
- Character: costume remains readable, but idle/camera phase changes the composition; it is not a reliable pixel-level comparison.

The evidence does not expose a reproducible curve defect requiring a targeted code adjustment. The modern branch remains opt-in and default-off.

## Cost and validation

The shoulder adds approximately 10–15 scalar/vector ALU operations per final fragment, depending on compiler lowering. It adds no pass, sampler, texture, or allocation.

- Release/x86 build with the verified v145 fallback: PASS.
- Direct-DB character/map smoke: PASS.
- `-glslPilot 0 -modernPresentation 1` legacy control: clean exit; no GLSL activation or compile/link/uniform failures.
- Modern OFF representative regression: PASS within the existing hard limits.
- Modern ON captures: completed with no black, full-white, or NaN-style frame failure observed.

Implementation SHA: `468a2a8fa985300c16579e347cd77140239cf5a`

The evidence follow-up commit containing this report and the contact sheet is the final branch SHA reported in the issue comment.
