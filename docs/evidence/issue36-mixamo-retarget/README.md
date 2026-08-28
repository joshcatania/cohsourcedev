# Issue 36 Mixamo-to-CoH retarget evidence

Captured on 2026-08-22 from `swinginganimations/Swinging.fbx`, action frame 30
at 30 fps. The source FBX is intentionally untracked and is not part of this
evidence directory.

| Artifact | Purpose | SHA-256 |
| --- | --- | --- |
| `source-frame30.png` | Source Mixamo joint geometry at frame 30 | `3E9F8BC2E8181BF47A184170314DAB28AD1A23FE61E1A762E081F692830C5A04` |
| `coh-runtime-fk-frame30.png` | CoH Male proxy reconstructed with native runtime FK | `5A2BACF8AD973D40965F791C06339167930DD2E871AB3D9661C0111C3DEB7764` |
| `runtime-static-frame30.jpg` | One-pose asset selected on the actual Male game skin | `4E8F952C3F37B41D38B13986B8931C5AC508FB473E636C5DC15B4DBEEDF2D304` |
| `runtime-full-midclip.jpg` | A different visible frame from the 60-frame runtime clip | `F6240EB5A24FF2EFAE46D03089DE23FAFFD2055E80796459063B7864F4C62030` |

The static and full screenshots were captured through the existing
`COHSOURCEDEV_CUSTOM_CANARY` sequencer move. Runtime diagnostics resolved the
actual skin as `TypeGfx=male` and the selected animation as
`MALE/COHSOURCEDEV_RETARGET_POSE_PROOF` for the static gate and
`MALE/COHSOURCEDEV_RETARGET_SWING_FULL` for the full-clip gate.

The full local reports and generated authoring files remain under:

- `agent/work/issue36-mixamo-runtimefk-v3-20260822/`
- `agent/work/issue36-mixamo-full60-20260822/`
