# Issue 37 — Fast-development shard tooling

Verified 2026-08-20 on `agent/issue-36-web-swing`, Release/x86 runtime.

## Measured cold starts

The benchmark starts from no local shard processes and ends only after
`smoke.ps1 -ExerciseCharacter` proves character creation and MapServer entry.
ServerMonitor/DbServer/Launcher process observations are recorded separately.

| Profile/variant | Run | Cold start → map connected | Evidence |
| --- | ---: | ---: | --- |
| Full baseline | 1 | 250.17s | `agent/logs/benchmark-shard-startup-Current-20260820-055623.json` |
| Full baseline | 2 | 255.95s | `agent/logs/benchmark-shard-startup-Current-20260820-060038.json` |
| Full representative median | — | **253.06s** | same two baseline records |
| FastDev, TSR off, Chat retained | 1 | 252.63s | `agent/logs/benchmark-shard-startup-FastDev-20260820-060722.json` |
| FastDev, TSR off, Chat retained | 2 | 253.68s | `agent/logs/benchmark-shard-startup-FastDev-20260820-061140.json` |
| FastDev representative median | — | **253.16s** | same two FastDev records |
| FastDev, TSR on | 1 | 300.73s | `agent/logs/benchmark-shard-startup-FastDev-20260820-061605.json` |
| FastDev, TSR off, Chat disabled | 1 | 248.58s | `agent/logs/benchmark-shard-startup-FastDev-20260820-062117.json` |

The selected FastDev median is effectively unchanged from Full: 0.10s slower,
or -0.04%. FastDev does not currently reduce cold MapServer startup time. Its
value is reduced process load plus warm-shard reuse: `PLAY-COH` leaves a healthy
compatible shard in place and only launches the client, avoiding unnecessary
full rebuild/restart cycles. TSR is disabled
because its measured first-map result was 47.57s slower than the TSR-off FastDev
median. ChatServer remains enabled: its disabled variant passed headless map
entry, but the normal GUI/chat path was not accepted on that narrower evidence.

## Selected service set

FastDev keeps ServerMonitor, DbServer, Launcher, normal MapServer, and ChatServer.
It disables the Account, Auction, Arena, Mission, Raid, Stat, Turnstile, and
LogServer launch blocks; sets NoStats/UseLogServer to off; disables master/request
beacon launch and beacon clients; and disables TSR preload. The final process
audit showed no BeaconServer or BeaconClient. No candidate caused a fatal loop or
headless map-entry failure. AuthServer may still appear as a ServerMonitor-owned
legacy process, but direct-DB login continues to bypass it and no AuthServer
database is required.

## Validation evidence

- Final FastDev character/map smoke: `agent/logs/smoke-directdb-20260820-065740.json`.
- Jump-height smoke passed: `agent/logs/jump-height-smoke-20260820-063602.json`.
- Web Swing smoke passed with the prepared development account: `agent/logs/webswing-smoke-20260820-063703.json`.
- Warm repeated `play-local` kept the shard running and did not duplicate an
  existing Ouroboros process.
- Profile transforms FastDev → Full → FastDev were idempotent and the guarded
  Full hashes reproduced the original configuration exactly. A stale generated
  state hash was refused and reset before the successful round-trip; no source
  configuration was overwritten destructively.

## Commands

Normal iteration:

```powershell
.\PLAY-COH.cmd
```

Full integration launch:

```powershell
.\PLAY-COH.cmd --full
```

Restart scopes for rebuilds:

```powershell
.\REBUILD-AND-PLAY-COH.cmd                 # client-only; preserve warm shard
.\REBUILD-AND-PLAY-COH.cmd --fast-shard     # full build; restart FastDev
.\REBUILD-AND-PLAY-COH.cmd --full           # full build; restart Full
```
