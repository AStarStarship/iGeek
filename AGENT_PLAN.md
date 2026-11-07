# iGeek Engineering Plan

Updated: 2026-11-07.
Scope: Environment/Gym architecture, world construction, and the Slot-based
agent connection model. This plan replaces the 2026-10-29 version.

## 1. Mission and repository boundaries

iGeek is a headless-first C++ environment engine for robotics, vehicle control,
games and scientific computing. Build reproducible Worlds with explicit
physics, contiguous observations/actions and measurable agent behavior.

- iGeek owns environments, simulators, classical reference controllers,
  model-neutral Environment/Gym interfaces, deterministic replay and metrics.
- ASCIICrabs owns abstract typed storage, schemas and Room/Slot primitives.
- CrabsTK owns reusable numerical kernels and concrete OS/device adapters.
- External policies connect through documented observation/action interfaces.
  Keep private model implementations, unpublished research, internal machine
  paths, confidential fixtures and private project handoffs OUT of this tree.
- Quaternion rotation math is ordinary shared robotics infrastructure. Keep
  it available for car, arm and spacecraft attitude/control work; do not
  remove useful public mathematics because another project also uses it.
- Publishing a license or source code is not a confidentiality mechanism.
  Review release contents before publication. LICENSE.md governs this tree;
  dependency licenses and third-party notices require separate review.

## 2. Core architecture: Environment, Gym, Room

### Environment (the base)
- Holds the EnvironmentManifest (the environment contract: name, domain,
  observation/action/reward specs, modality, determinism, seam number).
- Holds a TRoom (the Chinese Room — the state-machine substrate).
- The Room IS the environment: its Floor is inline static memory (no dynamic
  allocation), its Walls hold dynamic data, and its Door Slots (BIn/BOut)
  are the I/O boundary for agents.
- A Terminal is a Slot (Door) on the Room, not its own environment.

### Gym (a collection of Environments)
- Derives from Environment (Gym : public Environment).
- Is-A Environment: a Gym is itself an environment (recursive — a Gym can
  contain a Gym via envs_[GymEnvMax] of Environment*).
- Adds vectorized batch infrastructure: observations_, actions_, rewards_,
  terminals_, action_mask_, env_count_.
- StepBatch()/ResetBatch() drive the batch of contained environments.

### Room (Chinese Room) — the I/O model
- BIn/BOut are the transport-agnostic I/O slots. They can be physically wired
  to SPI (local device) or TCP/UDP (remote agent, SSH/RPC over a pipe).
  No separate "SPI slot" or "TCP slot" types are needed — BIn/BOut IS the
  abstraction; the physical layer is an implementation detail.
- The Room round-robins its Door Slots, reading volatile data from BIn,
  calling Star reducers, and writing results to BOut.
- The manifest is written to a Wall (dynamic memory), NOT the Floor.
  The Floor has no dynamic memory.

### Memory model
- Floor: inline static memory (TDic, 2048 bytes). No dynamic allocation.
  Batch arrays are carved from the Floor via CarveBatchArrays.
- Walls: dynamic memory. The manifest DIC/JSON lives here. Variable-size
  environment state (environments aren't all the same size) uses dynamic
  allocation or one large preallocated block.
- Environments require dynamic memory or one giant block of memory because
  they aren't uniform in size.

## 3. World inventory and status

### Worlds with real dynamics (StepBatch calls physics/logic Step)
- Car2D (Worlds/Car2D/) — ACarGym, TCarControl, 3-action steer
- Arm (Worlds/Arm/) — AArmGym, 2-revolute-joint arm, 2D+3D mixed
- Bike (Worlds/Bike/) — ABikeGym, planar bicycle model
- Pac (Worlds/Pac/) — PacWorldGym, classic Pac-Man
- Sudoku (Worlds/Sudoku/) — ASudokuGym, constraint solver

### Worlds needing work
- Terminal (Worlds/Terminal/) — StepBatch() is a PLACEHOLDER. TerminalKernel
  (Crabs Operand with Star reducers for pwd/ls/echo/cd/clear/exit) exists in
  Terminal.hxx but is UNWIRED. Next: mount TerminalKernel as a Slot on the
  Room, route actions through TermStep(), observation = real screen state.
- Cards (Worlds/Cards/) — BlackjackGym is STANDALONE (doesn't derive from
  Gym/Environment). Has its own manifest_ + batch interface. Inconsistent
  with the other 6 worlds. Decision: make it derive from Environment for
  consistency, or accept the exception.
- Maze2D, Polygon, Prisoner, Space, Tile — status unverified; need individual
  inspection to confirm whether they have real dynamics or are placeholders.

## 4. Current evidence

- Clean build: 0 errors (cmake --build build --clean-first, single-TU
  _Seams/_Seams.cpp).
- Full suite: "Unit tests completed successfully! (:-)+=<", failures=0
  everywhere (iGeek core, CARWORLD, CAR_ROOM, SudokuWorld, PacWorld manifest
  checks PASS).
- All 7 envs have EnvironmentManifest filled (name/domain/obs/action/reward
  contract) and validated in their seam tests.
- obs→observation rename: 513 substitutions across 52 files, word-boundary
  safe. Zero leftover obs_ observation-array identifiers.
- World directories moved to ./Worlds/<name>/; all include paths updated.

## 5. Open work (priority order)

### P0: Build out the environments
1. Terminal: wire TerminalKernel as a Slot on the Room. Replace StepBatch()
   placeholder with real TermStep() routing. Observation = terminal screen
   state from the kernel.
2. Blackjack: decide standalone vs Environment-derivation. If deriving,
   refactor to : public Environment like the other 6.
3. Verify Maze2D, Polygon, Prisoner, Space, Tile: do they have real
   dynamics or are they placeholders?

### P1: Manifest on a Wall
- EnvironmentManifestDump/Load (Floor TList serialization) has ZERO callers.
- The manifest must be written to a Wall (dynamic memory), not the Floor.
- A joining agent reads the manifest from the Wall to learn the environment
  contract before connecting.

### P2: BIn/BOut physical wiring
- BIn/BOut are transport-agnostic. Document how they map to:
  - SPI (local device: GPIO, bit-bang, or hardware SPI peripheral)
  - TCP/UDP (remote agent: socket pair, SSH tunnel, or RPC over a pipe)
- No new Slot types needed — the existing BIn/BOut abstraction handles both.

### P3: Release hygiene
- Label each World's README as tested headless, simulation-only, experimental
  or legacy, with an actual build entry point and measured limitations.
- Review stale graphics dependencies, source-header attribution, licenses.
- Keep compiled binaries, generated traces, machine-local evidence out of
  release payloads.

## 6. Engineering and verification contract

- C++23; ASCIICrabs types only (NO C++ standard library — "their code is
  dogshit"); functional casts IUD(x)/ISC(x); Uniprinter for strings.
- Chimera+ naming: immutable→CamelCase, mutable→lower_snake_case; _D suffix
  = default type macros; _P = bare params, _A = typename decl lists.
- No trailing const on member functions. No const refs.
- 2-space indent; .h/.hpp/.hxx module conventions.
- Fixed-capacity/preallocated contiguous buffers for hot paths; dynamic
  memory for variable-size state (Walls, not Floor).
- No VLAs, no C++ standard containers, no heap allocation in simulation ticks.
- Write a failing behavior test first, fix the smallest owned scope, then run
  Release, relevant sanitizers, deterministic replay and affected consumers.
- Measure with ClockNowFPD() and 64-bit counters. Report setup/IO scope,
  control frames, physics substeps, simulated time, wall time and batch size.

## 7. Build entry points

Single-TU build (current):
    cd /home/astar/AStarStarship/iGeek/_Seams
    cmake -S . -B build
    cmake --build build          # or --clean-first for full verify
    ./build/iGeek                # full suite; look for "Unit tests completed"

The _Seams/_Seams.cpp rollup is the only compiled translation unit. All world
sources are #included from it via the seam table.

## 8. Coordination and change discipline

- The coordinator is the sole editor of this plan.
- Preserve dirty/untracked work. No commits, pushes, resets, stashes, branch
  changes, blanket reverts or deletion without explicit human authorization.
- Read root/nested AGENTS.md and actual API definitions before implementation.
- Check live Git state; old branch names and completion messages are not proof.
- If a board record is explicitly requested, always target board `astarship`.
  Keep task bodies public-safe; a card is not evidence that work executed.
- Agent files: max 3 per agent (Task/Report/Context). One Report file, no
  numbered per-task. Reports = drafts → Papers/.

## 9. Key file map

- Environment.h/.hxx — base env: manifest_ + room_ + accessors
- Gym.h — Gym : public Environment; batch arrays
- EnvironmentManifest.h/.hxx — POD manifest (17 fields, observation_count),
  Validate/ToDic/DicTo/PrintJson/Dump/Load
- Worlds/<World>/ — the 13 world dirs (sources)
- _Seams/<World>/ — seam test units (NOT moved; still old names)
- _Seams/_Seams.cpp — rollup + seam table
- ASCIICrabs/Room.hpp — TRoom (Chinese Room), ROM_D, CRState, Slot/Door model
- ASCIICrabs/Clock.h/.hxx — ClockNowFPD
- _Agent/MaryContext.md — Mary's session handoff (detailed)

## 10. Conventions / user preferences

- No C++ standard library — ASCIICrabs only
- No trailing const on member functions; no const refs
- Chimera+ naming (see section 6)
- User dislikes Git; agent does not own git state; never restore deleted files
- User wants concise, non-hype answers; communication via code and proof
- Check in code frequently (user: agents cause dumpster fires)
- User's son is David. Never address the user as David.
