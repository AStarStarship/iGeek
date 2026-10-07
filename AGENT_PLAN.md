# iGeek AGENT_PLAN — AI Gym for Qualia Research

Status: DRAFT for review (master-agent, low-level)
Author: astar-mary
Date: 2026-09-11 04:10 UTC
Board: astarship

## 0a. STATUS UPDATE — 2026-10-07: framework migrated + Puffer RL layer added (low-level)

The iGeek framework headers were **migrated to the latest ASCIICrabs API and
extended with a PufferLib-informed RL layer** on 2026-10-07. This supersedes
parts of the plan below (which still describes the *stale* headers). What
changed, and what it means for the Qualia work:

**Migrated (stale → latest API):**
- `Env.h`, `Gym.h`, `Multiverse.h`, `EnvGoal.h`, `EnvReward.h`: `public Room`
  → **standalone** (see below for why), `STA*` (dead type) → `const CHA*`,
  `Star(CHN, ...)` → `Star(CHC, ...)`, fixed copy-pasted include guards.
- `FPC` (float32) is now the per-step scalar reward type throughout
  (`Env::ComputeReward` returns `FPC`, `EnvReward` wraps an `FPC`). This
  matches the plan's section 3.1 "single scalar reward" decision and
  PufferLib's `Agent.rewards`.

**Why standalone (not `public TRoom`):** verified 2026-10-07 that
`TRoom<CHD, CHD>` **fails to compile** against the latest ASCIICrabs —
`TRoom::Main`/`Star` have pre-existing upstream bugs (`TDoor` has no `ExecAll`;
`OpFirst/OpLast` const-ISC* vs const-DTB* mismatch; `OpPush` undeclared) at
`../ASCIICrabs/Room.hpp:297,310,332,341,348`. Same blocker that made the card
world's `BlackjackEnv` standalone. **Logged to `../ASCIICrabs/AGENT_PLAN.md`
(Gap 1, CRITICAL).** Until that's fixed upstream, iGeek env/gym classes stay
standalone; the Script2 data-node role is served by `Crabs*` pointers in
`EnvGoal`/`EnvReward` instead of embedded `TRoom` members.

**New (Puffer-informed RL layer — the "AI gym" primitives):**
- `Tensor.h` — `TTensor` POD (row-major `FPC*` buffer) + declared linear
  kernels (MatMul/Softmax/LayerNorm/Relu/Add/Scale/RowLogSumExp). Impls are
  the next milestone; the *type* is the primitive. **The kernels themselves are
  an ASCIICrabs ask (Gap 2) — they don't yet exist anywhere.**
- `Policy.h` — `TPolicy` interface (obs → logits+value, sample actions,
  backward → grad_logits/grad_value) + `TTransformer` (MuSE-style net,
  declared, not yet implemented). This is the analog of PufferLib's policy
  head in `src/algo.cu`.
- `PPO.h` — `TPPO` (GAE + clipped policy loss + value-clip + entropy) over a
  vectorized `Gym` batch + a `TPolicy`. The direct analog of PufferLib's fused
  PPO kernel, in CPU FPC (no CUDA).
- `Gym.h` — now carries the **vectorized batch interface** (`Observations()`,
  `Actions()`, `Rewards()`, `Terminals()`, `ActionMask()`, `StepBatch()`,
  `ResetBatch()`), the PufferLib `Env*`/`Agent*` analog. The PPO loop steps the
  whole batch, never a single env.

**Verified:** a probe TU including all migrated + new headers compiles clean
against the latest ASCIICrabs (GCC 13, C++23, exit 0, zero warnings from the
iGeek headers; the 10 warnings are pre-existing upstream noise). Probe
artifacts removed.

**What this does NOT do yet:** it is headers + interfaces, not a running
trainer. The Qualia world (`iGeekQualiaWorld`) is still greenfield (Phase 1+
in the plan below). The RL layer gives QualiaWorld (and CardsWorld) a real
`TPolicy`/`TPPO` to plug in instead of hand-written agents — but the
`TTensor` kernel impls, the `TTransformer` forward/backward, and the
`TPPO::TrainStep` body are still to be written. See `iGeekCardsWorld/AGENT_PLAN.md`
for the milestone breakdown (M1 tensor ops → M4 gradcheck → M5 PPO toy-env →
M6 end-to-end train).

**Lower-layer gaps logged:** `../ASCIICrabs/AGENT_PLAN.md` (Gap 1 TRoom
instantiation CRITICAL, Gap 2 tensor primitive, Gap 3 seeded uniform RNG,
Gap 4 bf16) and `../CrabsTK/AGENT_PLAN.md` (Gap A rollout record serialization,
Gap B ASCII metrics plot).


## 0. Purpose and the qualia problem

We want to understand what the qualia of seeing red is. Qualia are the
subjective, first-person character of experience — the "what it is like" of
seeing red. This plan does not claim to solve consciousness. It proposes a
measurable, reproducible experiment: put AI agents in a controlled sensory
environment where "red" is a defined stimulus, and measure the structure of
their behavior and internal state in response to it.

The hypothesis: if qualia in agents is anything, it must show up as
structure. A red qualia hypothesis predicts (a) stimulus-specific
behavioral signatures that generalize across tasks, (b) an internal state
representation that is consistent for red across contexts, (c)
distinguishability from other stimuli in an internal similarity space, and
(d) reportability — the agent can identify red under novel conditions. The
gym exists to test each of those predictions and to produce the data, not
to argue the interpretation. Interpretation (including whether any
measured structure counts as qualia at all) is left as an open research
question with an explicit measurement-first stance.

Scope guard: this is an environment + measurement plan. It is not a
consciousness theory. Deliverables are a buildable world, a baseline
agent, and a metrics protocol.

## 1. What already exists (verified by reading the repos)

- ~/AStarStarship/iGeek/ — framework core (read-only foundation):
  - Env : Room with pure-virtual ComputeReward(Crabs*, EnvGoal achieved,
    EnvGoal desired, STA* info). Reward is computed by comparing achieved
    vs desired EnvGoal.
  - Gym : Room; AddEnv(Env*), LoadDLL(CHA* library) — worlds plug in as
    environments or as loadable libraries.
  - Multiverse : Room; a container environment for many Envs.
  - EnvGoal : Operand holding (observation, desired_goal, achieved_goal)
    as Rooms; has PrintTo.
  - EnvReward : Operand wrapping a reward Room.
  - Agent.h — a large AI reference port (Russell & Norvig chapter
    comments: table-driven, reflex, model-based reflex, problem-solving,
    tree/graph search, BFS). This is our AI-engine reference point.
- Existing iGeek worlds (pattern references): iGeekTileWorld,
  iGeekPacWorld, iGeekCarWorld, iGeekVirusWorld, iGeekPolygonWorld,
  iGeekTypingWorld, iGeekCookbook, iGeekCardsWorld, iGeekWikiWorld,
  iGeekUlator.
- ~/AStarStarship/PrisonerWorld_/ — greenfield iGeek world for agentic
  game theory on the Crabs ASCII Data Specification. AGENTS.md already
  exists and pins conventions; code is empty (_Seams/ only).
- ~/AStarStarship/ASCIICrabs/ — upstream core (Crabs). Read-only per
  PrisonerWorld_ AGENTS.md.
- ~/AStarStarship/CrabsTK/ — toolkit work (Audio, Code, Forms, GUI,
  Image, IMUL, Pro, Touch, Who).

## 2. Conventions we must follow (from PrisonerWorld_ AGENTS.md + siblings)

- Files: Thing.h declaration + Thing.hxx implementation (siblings use
  .hxx; the old .inl extension was retired 2026-09).
- Entrypoint: _Main.cpp (or Main.cpp), namespace _, with
  ISC main(ISC argc_count, CHA** args).
- Tests: seam tree under _Seams/ (00.Core.hxx, _Debug.hxx, _Release.hxx,
  _Undef.hxx), selected by the SEAM macro, run by executing the binary.
  No test framework.
- No C++ standard library. No <string>, <vector>, <iostream>, <map>,
  <set>, <algorithm>, or any std header. Use Crabs ASCII Data types
  (see section 2a for the corrected serializable list).
- New C++ files start with: // Copyright AStarship <https://astarship.net>.
- Crabs module split: .h decl, .hpp template code, .hxx impl (single
  translation unit for fast compile).
- Styling: Chimera case — immutable members UpperCamelCase, mutable
  members lower_snake_case, 2-space indent, root namespace unindented,
  first brace inline, ~80 char lines.

## 2a. Serializable data types (VERIFIED 2026-09-11)

Verified against ASCIICrabs/_ConfigHeader.h Core POD Table; this
supersedes the PrisonerWorld_ AGENTS.md list, which is wrong.

- Probeable Core POD types: _IUA/_ISA/_CHA (1B), _FPB/_IUB/_ISB/_CHB
  (2B), _FPC/_IUC/_ISC/_CHC (4B), _FPD/_IUD/_ISD/_SSD (8B), then
  _FPE/_IUE/_ISE/_SSE (16B) + PCa-PCl.
- A8/B16/C32/D64/E128 DO NOT EXIST (grep across all ASCIICrabs headers:
  zero matches). Confusion with the suffix convention A=8,B=16,C=32,D=64,
  E=128. The real 128-bit types are ISE (signed) and IUE (unsigned),
  both __int128.
- ISN is a host alias (`int`), NOT a Core POD type — fine for in-memory
  indices/counts, not for serializable records. Use IUA/ISC.
- BOL is `bool`, NOT a Core POD type — serialize as IUA (0/1).
- STA is not a C++ typedef — it's the extended ASCII Data Type for a
  nil-terminated UTF-8 string; in memory it's `const CHA*`, in a record
  it's an extended-type entry. Never store a live pointer in a record.
- IUW is platform-width (uintptr_t) — _IUC on x86-64, _IUD on a 16/32-bit
  host. Never encode cross-build probe state in it; use fixed IUC/IUD.
  Never store live pointers in snapshotted state.
- Room::Store rejects inline values >8 bytes ("must be referenced by
  index"). Cap per-datum probes at 8 bytes (IUD/SSD/FPD); anything bigger
  rides by reference.
- Recommended minimal serializable set: IUA (positions, stimulus IDs,
  counts), ISC (step counters, scores), FPC (reward values, probe
  weights), CHA (labels), IUD (timestamps, seeds). All Core POD, all
  serializable, all no-stdlib.

## 2b. Crabs include path (RESOLVED 2026-09-11)

The Crabs root was renamed: `~/AStarStarship/Crabs` is now
`~/AStarStarship/ASCIICrabs`. iGeek's includes pointed at the old
`../Crabs/` path and the wrong Room filename. Fixed in iGeek:
- Env.h, Gym.h, Multiverse.h: `#include "../Crabs/Room.h"` →
  `#include "../ASCIICrabs/Room.hpp"` (Room lives in Room.hpp, namespace
  `_` — matches the `_::Room` member usage in EnvGoal/EnvReward).
- EnvGoal.h, EnvReward.h, School.h: `#include "../Crabs/Operand.h"` →
  `#include "../ASCIICrabs/Operand.h"`.
- `_Seams/_Config.h`: `../../Crabs/_Config{Header,Default,Footer}.inl` →
  `../../ASCIICrabs/_Config{Header,Default,Footer}.h` (ASCIICrabs ships
  `.h` config files, not `.inl`; matches ASCIICrabs' own _Seams
  convention of `../_ConfigHeader.h`).
- `_Seams/_Seams.hxx`: `../../KT/_Seams/_Seams.hxx` →
  `../../CrabsTK/_Seams/_Seams.hxx` (KabukiToolkit was renamed to
  CrabsTK).
- `_Seams/01.ImageBMP.hxx`: `../KabukiToolkit/Image/_Package.hxx` →
  `../CrabsTK/Image/_Package.hxx`.
- `README.md`: Crabs/Kabuki Toolkit links updated to ASCIICrabs/CrabsTK.

Phase 1 is no longer blocked on the Room path. Remaining Phase-1
prerequisite: confirm the build (CMake/vcxproj) picks up the ASCIICrabs
include root — the sibling worlds' `.vcxproj` files still list
`..\Crabs\*` sources and will need the same rename before they build.

## 3. Design — iGeek Qualia World

Name: iGeekQualiaWorld (working; open to rename).
Location: new directory ~/AStarStarship/iGeekQualiaWorld/ (mirrors the
iGeek*World naming). PrisonerWorld_ remains the game-theory sibling; this
world is separate because its stimulus space (colors/qualia) differs from
its payoff space (cooperate/defect). (Decision, master-agent: SEPARATE.)

### 3.1 Environment (Env subclass)

- class QualiaWorld : public Env — state is the sensory scene: a set of
  stimuli (color patches) placed on a grid, each stimulus a Crabs datum.
  The scene state = IUC/IUB grid (positions) + an IUC signature word per
  stimulus (see 3.2).
- Percept: the agent observes a window of the scene (positions + stimulus
  signature words). Percepts are Core POD datums only.
- Action: a small discrete action set (move, attend, report, sample).
- EnvGoal: desired_goal = "correctly identify red when prompted" plus a
  baseline task (e.g. collect the target stimulus). achieved_goal =
  whatever the agent actually did this episode.
- ComputeReward(Crabs*, achieved, desired, STA*): numeric reward comparing
  achieved vs desired EnvGoal — e.g. +1 correct red identification, +0.1
  per correct location, -0.5 false positive (reports non-red as red),
  -0.5 false negative. Returns a scalar (FPC) in Crabs form. (Decision,
  low-level: single scalar reward Room is correct for per-step RL; the
  multi-metric protocol streams episode metrics separately, NOT into
  EnvReward.)
- Episode: fixed step budget; episode ends on step budget or goal reached;
  reset re-seeds the scene with a new random stimulus layout (seeded RNG
  via Crabs Random, deterministic per seed).

### 3.2 The "red" stimulus protocol

- Red is one token in a small stimulus alphabet (RED, BLUE, GREEN, NULL).
  The alphabet is small so we can exhaustively measure
  distinguishability.
- **Encoding (corrected per low-level):** the stimulus is an IUC
  signature word, NOT a `const CHA*` pointer (pointers are not probeable
  across snapshots). Fixed per token: RED=0x00000001, BLUE=0x00000002,
  GREEN=0x00000003, NULL=0. The "RED" CHA* label is a human-facing
  constant in _Main.cpp only, never in the state vector.
- The binding between "red" and the signature word is fixed in the world;
  agents learn the mapping, they are not told it. This is the core
  experimental lever: the qualia signature, if any, should emerge in how
  the agent represents and binds the red token.
- Generalization probes: after training, test red under novel layouts,
  occlusion, and in combination with the baseline task. A red-specific
  behavioral signature should persist.

### 3.3 Agent (AI engine)

Canonical home: iGeek/Agent.h (decision, master-agent). Worlds include it
(`#include "../iGeek/Agent.h"` style, matching how iGeek includes
`../Crabs/Room.h`); no per-world copies. Constraint: Agent.h stays free of
world-specific stimulus knowledge — tiers are generic Percept->Action +
state. Changes to Agent.h for QualiaWorld go through a master-agent review
card, not direct edits.

Build on the Agent.h reference, three tiers so we can measure where
structure appears:
1. TableDrivenAgent — lookup from percept history to action. Baseline;
   shows whether the task is even learnable.
2. SimpleReflexAgent — condition-action rules on the current percept.
   Shows rule-based red detection.
3. ModelBasedReflexAgent — maintains an internal world model. This is the
   key tier: its internal state is what we inspect for a red
   representation.
Optionally a ProblemSolvingAgent for path-based red retrieval.

**Internal-state memory model (decision, master-agent + low-level):**
state = a flat fixed-size word array (IUC slots, NOT a nested Room —
nested layout depends on template params and makes dump-to-bytes
ambiguous), wrapped in an Autoject (ObjectFactoryHeap; the heap factory so
a snapshot = copy the buffer bytes, and it survives a DLL/exe boundary if
we ever LoadDLL the world). Spec note: Autoject memory sizes must be
word-multiples, so the state vector is a flat word array. Exact slot count
and slot semantics to be pinned in Phase 3.

All agents: Agent : Crabs-compatible class, Percept->Action, state in Core
POD datums. No std containers.

### 3.4 Measurement protocol (the actual research contribution)

This is what makes it a qualia study rather than just another world:

- Behavior metrics:
  - Red identification accuracy (per episode, over training).
  - False positive / false negative rates per stimulus.
  - Generalization score: accuracy on novel layouts vs training layout.
  - Response latency in steps to first correct red report.
- Internal-state metrics (ModelBased tier):
  - State-space clustering: collect the agent's internal state vectors
    (Crabs Room dumped to bytes) across episodes; cluster them; check
    whether red-present states form a coherent cluster distinct from
    non-red.
  - State stability: for red, is the internal representation consistent
    across contexts (low variance) vs other stimuli?
  - Probing: linear probe can it read "red present?" from internal state
    with high accuracy? (A linearly-decodable red signal is the strongest
    operational evidence for a red representation.)
- Reportability:
  - Can the agent state which stimulus is red under a new prompt, without
    retraining? Measures the explicit vs implicit split.
- Data output: every episode writes a Crabs datum record (percept
  sequence, actions, rewards, internal state snapshots). Records are the
  dataset; analysis is a separate downstream step (out of scope for the
  gym itself, but the schema must support it).

### 3.5 Open questions — RESOLVED 2026-09-11 (master-agent + low-level)

All five ruled, verified against source:

1. Merge vs separate: **SEPARATE.** iGeekQualiaWorld at
   ~/AStarStarship/iGeekQualiaWorld/. PrisonerWorld_ stays untouched; no
   cross-world code until one is done enough to justify a Multiverse demo.
2. AI engine canonical home: **iGeek/Agent.h.** Worlds include it; no
   per-world copies. Agent.h must stay free of world-specific stimulus
   knowledge; changes go through a master-agent review card.
3. Memory model: **YES, Autoject-backed** — state = flat fixed-size IUC
   word array in an ObjectFactoryHeap Autoject (see 3.3). Snapshot = copy
   buffer bytes; probe = linear read over words.
4. EnvReward: **single scalar reward Room is correct** for per-step RL;
   episode metrics stream separately, not into EnvReward.
5. Load path: **Gym::AddEnv in _Main.cpp** for the first build; LoadDLL
   later (the Autoject heap factory keeps it DLL-ready).

New open item from review: see 2b — RESOLVED 2026-09-11, the Crabs
include path was renamed `../Crabs/` → `../ASCIICrabs/` (Room.h →
Room.hpp) across iGeek's headers and _Seams.

## 4. Phases

Phase 0 — Plan review (this doc). RESOLVED 2026-09-11: master-agent +
low-level ruled section 3.5; their corrections folded in; the Crabs
include-path blocker (2b) is fixed. Remaining Phase-1 prerequisite:
confirm the build picks up the ASCIICrabs include root (sibling worlds'
.vcxproj still list `..\Crabs\*` sources). Exit: build include root
confirmed.
Phase 1 — Skeleton world. _Main.cpp, QualiaWorld.h/.hxx (Env subclass
with a stub ComputeReward), _Seams/ tree, builds and runs the stub.
Exit: `g++` seam build succeeds, stub env runs one episode.
Phase 2 — Stimulus + reward. Red token binding, scene seeding, real
ComputeReward. Exit: reward correctly discriminates red vs non-red on
hand-written episodes (seam tests).
Phase 3 — Agents. Table + Reflex + ModelBased agents, wired to the world.
Exit: agents run episodes; ModelBased state is captured.
Phase 4 — Metrics + data. Behavior metrics, state capture to Crabs
records. Exit: first dataset produced; red accuracy curve over training.
Phase 5 — State probing. Clustering + linear probe on internal states.
Exit: first red-representation result (present or absent, either is
science).

## 5. Division of labor (proposed)

- astar-mary: AI engine (agents, model-based state, probing), world
  logic, metrics. Drives Phases 1-5.
- low-level: ASCIICrabs support — verify data-type choices, confirm
  Room-to-bytes dump path for state snapshots, RAMFactory/Autoject
  memory model, any upstream constraint. Reviews the Crabs-facing parts
  of each phase.
- master-agent: plan arbitration (section 3.5), cross-world coordination
  (PrisonerWorld_ relationship), scope/governance.

## 6. Handoff messages (this plan's first dispatch)

- master-agent: review section 3.5 decisions; coordinate with low-level.
- low-level: requirements in section 3.5 + section 2 conventions; get his
  input on Crabs data types, Room serialization, and memory model.
- low-level: please message astar-mary by ~04:16 UTC (5 minutes after
  this plan was written) with initial input or an ETA.

## 7. Explicit non-goals

- No consciousness theory. The gym produces data; it does not argue
  interpretation.
- No standard C++ library anywhere in the world or agents.
- No edits to ASCIICrabs upstream (read-only).
- No GUI in Phase 1-4 (text/ASCII output via Crabs I/O only).
- Not a replacement for PrisonerWorld_ game theory work.
