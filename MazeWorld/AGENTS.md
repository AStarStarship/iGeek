# iGeekMazeWorld

ASCII terminal maze-solver video game — you drive the player, or let the AI
take over (toggle autopilot). Being converted from the legacy Script2
"igeek.autopilot" project into the new iGeek/CrabsTK format (the
`../iGeekPacWorld/` structure is the live template — see Conversion Plan).

## Kanban board

This project belongs to the **AStarship** organization; its Kanban board slug
is `astarship` (see `~/AStarStarship/AGENTS.md`). Always target `astarship`
explicitly (`--board astarship` / `board="astarship"`). Never rely on the
current-board pointer. If a task's assigned board differs, report the mismatch
before acting — do not silently switch.

## Remote

`https://github.com/AStarStarship/iGeekMazeWorld.git` (verified 2026-10-08;
the `AStarStarship` org is correct — do not treat it as a legacy Kabuki
remote).

## Code Style — Chimera+ (AStarship Standard)

All **code and filenames** in the AStarship ecosystem follow the **Chimera+**
style guide (`~/AStarStarship/ASCIICrabs/__ChimeraPlus.md`). The naming
philosophy applies to code identifiers and filenames fleet-wide.

### Core Rule: immutable vs mutable
- **Immutable** (can't change after init) → **CamelCase**
- **Mutable** (changes at runtime) → **lower_snake_case**

### Naming Summary
| Element | Convention | Example |
|---|---|---|
| Type aliases (fixed-width) | 3-letter CAPS codes | `CHA` char8, `ISC` int32, `IUD` uint64, `BOL` bool, `FPC` float32, `FPD` double64 |
| Structs / classes | CamelCase + T(POD)/A(utoject) prefix | `TArray`, `AMap`, `Crabs` |
| Struct members (mutable) | lower_snake_case | `socket_bytes`, `header_bytes` |
| Struct members (immutable) | CamelCase | `StackTotalMin`, `ColumnWidth` |
| Free functions / methods | CamelCase, type-prefixed | `CrabsInit`, `TArrayBytes`, `TMapFind` |
| Function suffixes | `_NC` (no-check), `C` (const/count), `T` (POD), `A` (autoject) | `TArrayInsert_NC`, `CSizeMin` |
| Local variables | lower_snake_case (always) | `bytes_data`, `read_cursor` |
| Macros / #define | UPPER_SNAKE_CASE | `CRABS_RUN_TESTS`, `CPU_X64` |
| Private/protected members | trailing underscore | `aobj_`, `array_` |
| Namespaces | `namespace _ { ... }` | single shared underscore namespace |
| DB names / tables / columns | lower_snake_case (mutable) | `user_sessions`, `order_items` |
| Client-facing API / headers | CamelCase (immutable contract) | `OrderService`, `PaymentGateway` |
| Filenames (client contract) | CamelCase | `OrderService.h` |
| Filenames (host/storage) | lower_snake_case | `user_sessions.py` |

### Header Boilerplate (C/C++)
```
// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_<NAME>_<H|HPP>
#define CRABS_<NAME>_<H|HPP>
#include "..."
#if SEAM >= CRABS_<NAME>
... body ...
#endif
#endif
```

### Formatting
- 2-space indent
- Opening brace same line (functions/control), next line (structs/classes)
- `D_ASSERT()`, `D_COUT()`, `D_RETURNT(type, val)` for debug/assert/return
- `NILP` = nullptr
- `alignas(ACPUCacheLineSize)` for hot structs
- Doxygen `/** @param @return @pre @link @see @code */` comments

### The One-Sentence Version
**Immutable → CamelCase, mutable → lower_snake_case; macros UPPER_SNAKE;
private/protected get a trailing `_`; types are short CAPS width-codes
(CHA/ISC/IUD/BOL); structs are CamelCase with T(POD)/A(utoject) prefix;
functions are CamelCase with T/A kind prefixes and _NC/C markers; locals are
snake_case; everything lives in `namespace _`; every file starts with
AStarship copyright, `#pragma once`, a `CRABS_*` guard, and `#if SEAM >=
CRABS_*` gating.** The name tells you the kind, width, access level, and
mutability — before you read the body.

## Conversion Plan (legacy Script2 → iGeek/CrabsTK)

The current tree is the original **igeek.autopilot** Script2/MSVC project:
UTF-16-LE sources, `SI4`/`CH1` type codes, `<module_config.h>` /
`script2/module_header.inl` config, `KABUKI_AUTOPILOT_*` seam macros, `namespace
kabuki::hello_machina_spectulatrix` seams, `GetAsyncKeyState` console I/O, and
a Windows `.vcxproj`. It does not build on this (Linux, g++) box. The legacy
seams/ dir and `.vcxproj` are reference-only; do not try to build them.

Target shape — **copy the structure of `../iGeekPacWorld/`** (note: astar-mary
is actively working on `../iGeekPacWorld/` and the `../iGeek/` engine, so re-check
both before writing code against them):

- `MazeWorld.h` / `MazeWorld.hxx` — `MazeWorldEnv : Env` (single env) +
  `MazeWorldGym : Gym` (vectorized batch for TPPO), mirroring
  `PacWorldEnv`/`PacWorldGym` in `../iGeekPacWorld/PacWorld.h`.
  - Env: grid of cell types (wall/empty/start/finish), the ASCII robot
    occupies 2 horizontal cells and moves orthogonally (no diagonal),
    seeded deterministic maze layout per episode, fixed `MaxSteps` budget,
    terminal on reaching the finish.
  - Observation: fixed-length local window around the robot (one-hot per
    cell channel + 1 progress feature), same fixed-length scalar vector pattern
    as PacWorld.
  - Actions: 5 (Stay, Up, Down, Left, Right) — the legacy 8-direction enum
    is dropped for the headless version (the robot is horizontal-only).
- Legacy `MazeAgent` policies become `PolicyPilot`-style agents:
  `PolicyPilotManual` (human: A/D turn, W/S accel/decel, E toggle, Q quit —
    the interactive console game loop) and
  `PolicyPilotDepthFirstRoundRobin` (autopilot; the legacy code's own readme
    notes the solver "doesn't work right now" — treat the DFS solver as
    unverified until a probe proves it).
- `_probe_mazeworld.cpp` / `_probe_mazeworld.hxx` — build + run probe
  (env steps deterministically per seed, obs fixed-length, Gym batches,
  PPO trainstep no-crash). Copy `_probe_pacworld.cpp` and adapt.
- `_verify_mazeworld.cpp` — verifier: autopilot (and/or PPO policy) solves
  a small seeded maze; the learning claim requires a real reward increase,
  not a single delta. Copy `_verify_pacworld.cpp` and adapt.
- `_Seams/` — seam tree when the framework is stable; keep it minimal at
  first (see the PacWorld seams).
- `README.md` — world description, design, verified claims, build commands,
  milestones. Copy the PacWorld README shape.
- Drop after conversion: `__igeek.autopilot.vcxproj*`, `seams/0[01].*.h`,
  `test.h`, `maze.h`, `agentmaze.h/.inl`, `xy.h/.inl`, `_main.cpp`,
  `_config.{h,inl}`, `_impl.inl`, `_debug.inl`, `_release.inl`,
  `_undef.inl`, `_seams.inl` (legacy reference only — keep until the new
  tree builds, then delete). `docs/` markdown templates may be kept or
  replaced to match sibling worlds.

## Build (probe, g++ C++23)

```
cd iGeekMazeWorld
g++ -std=c++2b -O2 -I. -I../ASCIICrabs/_Seams -I../ASCIICrabs -I.. \
    _probe_mazeworld.cpp -o /tmp/probe_mazeworld
/tmp/probe_mazeworld        # expect MAZEWORLD_PROBE_OK, exit 0
```
Verifier (learning + ASan-clean; run the ASan build for safety):
```
g++ -std=c++2b -O1 -g -fsanitize=address -I. -I../ASCIICrabs/_Seams \
    -I../ASCIICrabs -I.. _verify_mazeworld.cpp -o /tmp/mw_verify
/tmp/mw_verify              # expect VERIFY_TRAIN_PASS, exit 0
```

## No-Stdlib + no-exceptions rules

- **No C++ standard library** in Crabs-family code. No `<string>`, `<vector>`,
  `<iostream>`, `<map>`, `<set>`, `<algorithm>` or any std header. Use
  ASCIICrabs/CrabsTK types and console I/O (`CIn`/`COut` from
  `ASCIICrabs/CIn.h`/`COut.h`; `CInKey()`/`CInState()` exist there for the
  interactive loop). Probes use raw `write(1/2, ...)` for I/O, as the
  PacWorld probes do.
- **No exceptions** in low-level paths; return status codes / `BOL`.
- Fixed-size arrays (no VLAs, no heap except ASCIICrabs Autoject when
  genuinely needed).
- Serializable state uses Core POD types only: `IUA` (positions, cell
  types, counts), `ISC` (step counters, scores), `FPC` (rewards), `CHA`
  (labels), `IUD` (seeds, timestamps).

## Verification discipline

- Never claim the maze solver or any RL policy "learns" / "solves" without a
  probe or verifier run behind it (see the PacWorld README for the bar:
  determinism check, fixed-length obs, no-crash trainstep, and for learning
  claims a real reward trend over training).
- Legacy sources are unverified (the original seams say "doesn't work right
  now"). Re-verify every migrated behavior; do not port a claim.
- Profile at `-O2` before calling anything a bottleneck (the PacWorld
  "2 min per TrainStep" scare was a `-O0` artifact).
- ASan at `envs=8` (GymEnvMax) is the memory-safety gate; smaller batches
  mask overflow bugs.

## License

Copyright [AStarship™](https://astarship.net). Legacy files carry the
original 2014-2019 Cale McCollough MPL-2.0 header; new files carry the
AStarship copyright line. Provenance of the legacy code is the Captain's —
do not relicense or strip legacy headers without instruction.
