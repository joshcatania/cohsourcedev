# Parallel local shards via COH_PORT_OFFSET

Branch: `agent/parallel-shard-dev` (extracted from the Remaster Profile v1
work, where it was originally committed as `3b0234524`). Draft — separate
Sol review; do not merge without one.

## What this is

Setting `COH_PORT_OFFSET` in the environment shifts every default
listen/connect port in `Common/comm_backend.h` by that amount (default 0 =
historical ports exactly), so a second full shard from a different worktree
can run beside another agent's shard on one machine. `agent/start-shard-parallel.ps1`
/ `agent/stop-shard-parallel.ps1` spawn and stop such a shard from **this
worktree's bin only** (process matching is by executable path, so another
agent's processes are never touched). They bypass ServerMonitor because its
process monitor matches DbServer/Launcher by exe name system-wide and would
see the other agent's processes as its own.

Runtime configuration (local, uncommitted): `bin/data/server/db/servers.cfg`
points at an isolated `cohgfx` SQL database (`SqlInit` creates it on first
run; the one-time static-map init takes ~25 minutes) and sets
`DefaultAccessLevel 9` for capture accounts. The DbServer game-client
listener is UDP 7000(+offset) — TCP port checks will not show it.

## Port-band validation

An offset is only valid if the **effective** band stays inside the 0-65535
TCP/UDP port range. The base band is 6971-7000, public mapserver ports
continue upward from 7001(+offset) via an unbounded bind scan
(`svr_init.c`/`dbquery_init.c` walk `BASE_MAPSERVER_PORT` upward until a
bind succeeds), so headroom must be reserved below the ceiling:

- `agent/start-shard-parallel.ps1` rejects any offset where
  `7001 + offset + 512 > 65535` (i.e. `-PortOffset` must be ≤ 58022)
  before spawning anything.
- `cohDefaultPortOffset()` in `Common/comm_backend.h` defensively clamps to
  `[0, COH_MAX_PORT_OFFSET]` with the same bound, so a stray environment
  value can never produce out-of-range ports in any process.

## Why every comm_backend port shifts (audit)

- Every port in the 6971-7000 band is defined in `comm_backend.h` and
  consumed through those defines; a repo-wide audit found no hardcoded
  69xx/700x literals outside the header (the only other BASE_MAPSERVER_PORT
  users are the mapserver bind scans, which derive from the shifted define).
- Cross-process discovery is consistent by construction: servers bind the
  shifted defines, and dynamically allocated ports (mapserver public ports)
  flow peer-to-peer through the protocols from the port that actually bound,
  so nothing re-derives an unshifted default.
- Ports that must NOT shift are outside this header and are untouched:
  AuthServer's game-client listener (TCP 2106), SQL Server, and the launcher
  updater ports are not part of the shard-internal band.
- The historical `DEFAULT_SVRMON_LISTEN_PORT`/`DEFAULT_UPDATESERVER_LISTEN_PORT`
  collision (both 6993) is pre-existing upstream behavior, unchanged here;
  neither ServerMonitor nor UpdateServer runs in the parallel-shard layout.

## Concurrency rules

Never start a parallel shard while another agent owns the shared shard
without an offset; never run two parallel shards with the same offset (the
start script refuses when the target band has listeners). Clients
(capture.ps1/smoke.ps1/Ouroboros) must run from a shell with the same
`COH_PORT_OFFSET` set.
