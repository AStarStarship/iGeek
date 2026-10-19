# iGeek AGENT_PLAN — AI Gym for Qualia Research

Status: DRAFT for review (master-agent, low-level)
Author: astar-mary
Date: 2026-09-11 04:10 UTC
Board: astarship

## 0f. PUBLICATION READINESS — 2026-10-18: full review + cleanup pass (astar-mary)

Goal: get the iGeek + CrabsTK + ASCIICrabs trees ready to publish for
inspection by external experts ("big wigs") and to start attracting CrabsTK /
ASCIICrabs users. This section is the canonical problem list + fix sequence.
It supersedes the stale "open items" scattered in 0a–0e.

### 0f.0 Context: IP structure (user directive 2026-10-18)

- **iGeek** = for-profit, **source-available non-commercial** (AStarship
  Strong Source-available License: no third-party commercialization, mandatory
  improvement donations). Contains patented IP. Trade secrets are documented
  on GitHub as the de facto protection.
- **CrabsTK / ASCIICrabs** = the reusable non-iGeek code (game engine / app
  API / math that other apps reuse). ASCIICrabs was hand-built over years; it
  is the foundation, not a clone.
- **NEVER publish**: Laplacian Tokenization, Quaternion Fourier, MUSE
  (architecture), Puffer/PufferLib, private machine paths
  (`/mnt/MuhNAS/...`, `/home/astarcale/...`, `3P/...`), internal agent
  orchestration (Qwen/Gemini handoffs).

### 0f.1 DONE this session (verified)

1. **IP scrub (all three trees).** Removed/neutralized every public
   MUSE/MuSE/Puffer/PufferLib/Laplacian/quaternion-Fourier reference and every
   private path:
   - iGeek code: `Policy.h`, `PolicyNet.hxx` ("MuSE-style" → "compact
     transformer policy" — then REVERTED 2026-10-18: user clarified MUSE is a
     licensed product name and may ship; only the mechanism — monoid
     sphere→ellipsoid transform — stays private).
   - iGeek docs: `CarWorld/Presentation.md` (title, Fourier/Laplacian mentions,
     NAS + MUSETransformer paths), `CarWorld/Headless.md`,
     `CardsWorld/AGENT_PLAN.md` (11 spots), `CardsWorld/BlackjackDemo.md`.
   - ASCIICrabs: `UnificationPlan.md` (spectral-tokenizer → MUSE refs).
   - CrabsTK: `Math/Blas.h` (MUSEWorld → TMatrix), `AGENT_PLAN.md` (hermes
     scratch path), `HANDOFF.md` (Qwen/Gemini/investor lines).
   - Verified: `grep -rni 'muse|puffer|laplacian|quaternion'` across all three
     trees (excl. iGeek/AGENT_PLAN.md internal history) → 0 hits.
   - NOTE: iGeek `AGENT_PLAN.md` (this file) keeps its 0a–0e history which
     mentions Puffer/MuSE as internal record. DECISION NEEDED: strip this file
     from the public tree or accept it as internal-only (it is the most
     sensitive doc in the repo).
2. **License.** `LICENSE.md` created (iGeek) with the AStarship Strong
   Source-available License; `README.md` rewritten with repo map + build
   instructions + corrected license text (typos fixed: purchae/comrechial/
   refereed/bound-to). DECISION NEEDED: do CrabsTK + ASCIICrabs get the same
   license, or a permissive one (their READMEs currently carry MIT-style text
   that says "sell copies" — inconsistent with the non-commercial intent)?
3. **Core framework self-test (new, green).** `CMakeLists.txt` (root) +
   `_Seams/_Main.cpp` + `_Seams/core_test.cpp` + `_Seams/_Config.h` (rebuilt
   ASCII; was UTF-16) + `_Seams/00.Core.hxx`. Single-TU build links the whole
   core layer and checks: TTensor kernels vs hand values, TLinearPolicy
   determinism + sampling, TPPO over a toy Gym (finite stats), the 3 agent
   tiers, LinearProbe end-to-end. **Release + ASan/UBSan/leak: PASS (exit 0).**
   Replaced the dead `_Seams` tree (`_Main.cpp` included nonexistent
   `../../Graphics/_Package.hxx`; `01.ImageBMP.hxx`/`02.OpenGLHelloWorld.hxx`
   needed SDL/glm/glew; `_Seams.hxx` had stale `KABUKITOOLKIT_RELEASE` +
   15 `SCRIPT2_*` levels; CMakeLists was C++11-era). Dead files removed.
4. **Broken includes fixed.** `EnvGoal.hxx` + `EnvReward.hxx`
   `#include <Script2/Operand.h>` (dead project) → their own headers.
5. **Chimera+ functional casts restored in core.** ~50 C-style casts
   `(IUD)x`/`(FPC)x`/`(ISC)x` → `IUD(x)`/`FPC(x)`/`ISC(x)` across
   `Agent.hxx`, `AgentState.hxx`, `LinearPolicy.hxx`, `PolicyNet.hxx`,
   `PPO.hxx`, `Tensor.hxx`, `Metrics.hxx`.
6. **Real bug fixed (found by the new test): LinearProbe scale mismatch.**
   `Train` normalized features by `1/max_abs` but `Predict`/`Accuracy` did
   not, so reported accuracy was computed on a different scale than training
   (stuck at 56% on separable data). Added `inv_scale_` member, applied in
   `Predict`. Probe now reaches 100% on the test target.
7. **Regression check.** BikeWorld + CarWorld (the worlds that compile the
   modified core `.hxx` files) rebuild clean after the changes.

### 0f.2 MAJOR PROBLEMS — the list (talk-out + fix sequence)

Severities: [C]ritical (publish-blocker), [M]ajor, [m]inor.

**A. IP / licensing**
- A1 [C] `iGeek/AGENT_PLAN.md` itself is the most IP-sensitive doc (Puffer,
  MuSE, "dunk on him with MuSE Transform", NAS paths in history). Strip from
  public tree OR gate it private. (DECISION)
- A2 [C] CrabsTK + ASCIICrabs have NO LICENSE file; their READMEs carry
  MIT-style "sell copies" text that contradicts the non-commercial intent.
  (DECISION: same AStarship license vs permissive)
- A3 [M] ~35 iGeek files lack the `Copyright AStarship` header (all of
  MazeWorld/, most of CarWorld/ XY.h+xy.hxx+agentmaze.hxx+_Main.cpp,
  TypingWorld/_seams, PolygonWorld/_Config.h, Ulator/_Config.h,
  WikiWorld/_Config.h).
- A4 [m] Two third-party licenses embedded (PolygonWorld/Resources/LICENSE_OFL.
  txt, TileWorld/data/herringbone/license.txt) with no top-level NOTICE.

**B. Core framework API**
- B1 [C] DONE — Script2 includes (see 0f.1.4).
- B2 [M] DONE — C-style casts (see 0f.1.5).
- B3 [M] Module-file mismatch: `LinearPolicy.hxx` declares `TLinearPolicy` but
  has no `LinearPolicy.h`; `TTransformer` declared in `Policy.h` but
  implemented in `PolicyNet.hxx` (no `PolicyNet.h`). External reader can't tell
  the canonical declaration. FIX: add the missing `.h` decls (or fold the
  decl into the existing header) so each impl maps to one declaration.
- B4 [M] `Agent.h` is 1,836 lines: ~1,423 lines of Russell & Norvig textbook
  excerpts (figures/tables/chapter text) precede the 4 agent classes.
  Copyrighted-book-content + review-quality problem. FIX: move the reference
  text to a `_Spec/`-style doc or an appendix file; keep only the classes +
  a citation in `Agent.h`.
- B5 [M] `Gym` API leaks raw internal pointers (`Observations()/Actions()/
  Rewards()/Terminals()/ActionMask()`) with no const overload and no lifetime
  doc; `LoadDLL()` is a public no-op stub; base `StepBatch()/ResetBatch()` are
  silent no-ops. FIX: document ownership (Gym owns, valid until ResetBatch /
  dtor), add const overloads, mark `LoadDLL` `@deprecated stub` or remove.
- B6 [m] `TTensor.dtype_` is set but never used ("reserved for bf16").
- B7 [M] DONE — dead `_Seams` tree (see 0f.1.3).
- B8 [m] `Multiverse` is a hollow class (named "container" but holds no
  envs; `ComputeReward` returns 0.0f). Either implement or mark experimental.
- B9 [m] `School*` (School/SchoolCourse/SchoolDegree/SchoolDepartment) extend
  `Operand` but nothing in the tree uses them; verify they compile or move to
  a clearly-labeled experimental set.

**C. Worlds (the monorepo is not uniformly publish-ready)**
- C1 [M] Only 4 of 13 worlds integrate with the RL core (BikeWorld, CarWorld,
  CardsWorld, PacWorld). The rest (HBM, MazeWorld, PolygonWorld,
  PrisonerWorld, SudokuWorld, TileWorld, TypingWorld, Ulator, WikiWorld) are
  standalone. The "AI gym" claim doesn't cover most of the tree. FIX: label
  each world's README with status: RL-integrated / simulation-only /
  experimental / legacy.
- C2 [C] Frozen SFML/legacy worlds that won't compile headless:
  `PacWorld/{Bonus,Character,Dot,GameState,PacWorld}.h` include `<SFML/...>`;
  `PacWorld/Animator.h` + `TypingWorld/A.h` + `WikiWorld/_Main.cpp` +
  `Ulator/_Main.cpp` include old `<Crabs/...>` paths (pre-rename, don't
  resolve). `PacWorld/CMakeLists.txt` is broken (unterminated string,
  references nonexistent `main.cc`, links SFML). FIX: quarantine the SFML
  files (move to `_Old/` or a `legacy/` dir) or strip them; the headless
  PacWorld env is the live code.
- C3 [M] `TileWorld` is a fork mid-port: 11 spine files still carry SFML-era
  includes + `<TVec.h>`/`<_ConfigHeader.h>` angle-bracket includes that assume
  a specific -I; `_Seams/_Main.cpp` calls `iGeek::Env(...).Run()` via
  `../../iGeek/Env.hxx` (path + namespace that don't match the current root).
  No CMakeLists. FIX: finish the headless port or quarantine TileWorld.
- C4 [M] Divergent duplicates: `CarWorld/AgentMaze.h` vs `MazeWorld/AgentMaze.h`
  are different encodings (one UTF-16); CarWorld also has `XY.h`+`xy.hxx`+
  `agentmaze.hxx` (no copyright) duplicating MazeWorld `Xy.h`.
  `PolygonWorld/Agent.h` is a second copy of root `Agent.h` (same
  `IGEEK_AI_AGENT_T` guard → redefinition hazard). FIX: dedupe; one canonical
  Agent.h, one AgentMaze.
- C5 [M] Probe/debug files in source tree: `CardsWorld/{_diag_ppo.cpp,
  _probe_ppo.cpp}`, `PacWorld/{_probe_pacworld.cpp,_probe_pacworld.hxx,
  _verify_pacworld.cpp}`. FIX: move to `_Seams/` or delete.
- C6 [m] Empty/ghost dirs: `VirusWorld/` (only .vs + x64), `CardsWorld/_seams`
  (empty), `SudokuWorld/_Seams` + `SudokuWorld/data` (empty), `iGeek/x64/Debug`.
- C7 [m] 3 untracked new worlds (PrisonerWorld, SudokuWorld, HBM) not in the
  publish baseline. DECISION: ship them (commit) or exclude from this publish.
- C9 [C] **DONE — CardsWorld unblocked (was: cannot compile against current
  ASCIICrabs, pre-existing).** Root cause: `ASCIICrabs/_Package.hxx:13`
  included `Floor.hxx`, deleted upstream 2026-10-13 (commit `4da1d3b`,
  message ".") without removing the include — broke every world building
  through the package root. Fix applied this session: deleted the one dead
  include line (verified zero other references). CardsWorld rebuilt from
  scratch against fixed upstream: **seam tests exit 0** (Core card logic,
  Gym tensor ops, PolicyNet determinism + gradcheck max err 179e-6, PPO
  learns the blackjack rule — mean reward 0 → 10000 after 300 steps).
  The old 10-08 binary's PPO segfault (exit 139) was the symptom of the
  upstream drift, not a CardsWorld bug.
- C10 [m] Docs with absolute private paths fixed this session (were [C]):
  `CardsWorld/BlackjackDemo.md` (cd path), `CarWorld/Headless.md`,
  `CrabsTK/IMUL/ImplementationPlan.md` (handoff header + 2 path refs —
  orchestration metadata scrubbed). `AStarCale` GitHub-handle refs in
  CrabsTK (issue templates, IMUL READMEs) are the user's own org identity —
  acceptable, not scrubbed.
- C8 [M] Build matrix inconsistent: CMake only in BikeWorld/CarWorld/
  CardsWorld/HBM/PacWorld(broken)/SudokuWorld. MazeWorld/PolygonWorld/
  TileWorld/TypingWorld/Ulator/WikiWorld have only stale .vcxproj. (Root core
  CMake added this session — 0f.1.3.) FIX: add CMake to the shippable worlds
  or mark the rest legacy.

**D. ASCIICrabs**
- D1 [C] `TRoom` is broken and stays broken in a published tree —
  `Room.hpp` `Main()`/`Star()` have 3 verified bugs (TDoor has no `ExecAll`;
  OpFirst/OpLast const-ISC* vs const-DTB* mismatch; `OpPush` undeclared).
  Every new user who instantiates `TRoom` hits a landmine. FIX: repair the 3
  bugs, or clearly mark `TRoom::Main/Star` as deprecated/unsupported with a
  note pointing to the standalone pattern iGeek uses.
- D2 [M] No-stdlib violations in the core: `Interrupts.h:12`
  `struct RoomCrashException : public std::exception` (and `Room.hpp::Main`
  does `try/catch`); `Puff.hpp:9`/`CIn.hxx`/`COut.hxx` `<iostream>`;
  `Clock.hxx` `<ctime>`; `File.h` `<dirent.h>`; `conio.h` `<termios.h>/
  <unistd.h>`; `_ConfigHeader.h` `<cstdint>/<intrin.h>`. The iostream/
  exception ones contradict the no-stdlib claim in the README. FIX: document
  the exact no-stdlib boundary (which headers are "platform I/O" and allowed)
  or replace the std::exception + iostream paths.
- D3 [m] Test files at repo root: `_test_occlusion.cpp`, `test_regex.cpp`
  should live under `_Seams/`.
- D4 [M] `README.md` still says "Crabs" and links
  `github.com/AStarStarship/Crabs` (old repo name). FIX: rename to
  ASCIICrabs + correct link.
- D5 [M] `UnificationPlan.md` (183 lines) + `CoreUpdatePlan.md` (288 lines) at
  repo root are internal planning docs. FIX: move to `_Spec/` or an internal
  dir, or gate from public.

**E. CrabsTK**
- E1 [M] No CMakeLists at root (only in Hatch templates); README is 31 lines
  (mission + license, no build instructions). The only build doc is the
  HANDOFF.md g++ one-liner (links `-lGL`). FIX: add a root CMake + a real
  "Build" section.
- E2 [M] Subdir quality spread: GUI/ is a 1-file stub, Hatch/ is a template
  dir, Math/ is a single Blas.h, Audio/Forms/IMUL/Pro/Touch/Who have no
  `_Package.h`. The "toolkit" reads as a grab-bag of 12 subdirs at wildly
  different completeness. FIX: per-subdir status labels (complete / partial /
  stub) in the README.
- E3 [m] std headers: Who/TMessage.hpp + TRoom.hpp `<cstring>/<cstdlib>/
  <atomic>`, TRoster.hpp `<ctime>`, Math/Blas.h `<cblas.h>` (MKL — fine,
  but undocumented dependency). FIX: document the MKL/OpenBLAS option.
- E4 [m] CrabsTK/Pro|Touch|Who `_Undef.h` include `<ASCIICrabs/_Footer.h>` —
  verify the path (file is actually `_ConfigFooter.h`).

**F. _Spec (the RFC that gets the expert eyes)**
- F1 [C] Top `README.md` content table out of sync: 8 of 50 links broken —
  `Data/HomoTuples`, `Addressing/MostLongitudinalBit`, `Protocol/Time`,
  `Protocol/Operations`, `Protocol/Handshake` (actual: GenericHandshake.md),
  `Protocol/CongestionControl` (actual: CongestionControlTable.md),
  `Crabs/MonoidChineseRoom` (actual: ASCIIListChineseRoom.md), and
  `../Requirements.md` (resolves outside _Spec where it doesn't exist).
- F2 [M] Sub-READMEs incomplete: Data/ 6 unlinked (Types, Numbers, Strings,
  BSequences, Clock, ExtendedTypes, TypeAliases, MetaTypes, Ideas),
  Overview/ 3, DNS/ 4, Protocol/ 9, Addressing/ 2. The whole DNS/ section is
  absent from the top table.
- F3 [M] Stale refs: `Protocol/Slots.md:46` links to
  `github.com/AStarStarship/KabukiToolkit/wiki/...` (dead org name);
  README links `github.com/AStarStarship/Crabs` (old name). Spec is branded
  "SCRIPT" throughout while the code is Crabs/ASCIICrabs — three names
  (SCRIPT, Crabs, ASCIICrabs) with no mapping.
- F4 [m] README self-declares "plenty of errors" — fine as RFC honesty, but
  with 8 broken links + 20+ unlinked files it reads unfinished.

### 0f.3 FIX SEQUENCE (post-compact execution plan)

**Window 1 — publish blockers (do first).**
- ~~C9: delete dead `#include "Floor.hxx"` from `ASCIICrabs/_Package.hxx` +
  rebuild CardsWorld~~ — DONE this session (verified, seam tests exit 0).
- A1 decision + strip/gate `iGeek/AGENT_PLAN.md` from public (or accept).
- A2 decision + LICENSE files for CrabsTK + ASCIICrabs; fix their README
  license text.
- A3: add missing copyright headers (scriptable — ~35 files).
- C2: quarantine PacWorld SFML files + fix/strip `PacWorld/CMakeLists.txt`;
  strip the dead `<Crabs/...>` includes in TypingWorld/WikiWorld/Ulator.
- D1: repair or deprecate `TRoom` (the single most-landmine-prone API).
- F1: fix the 8 broken top-level _Spec links (cheap, high visibility).

**Window 2 — core API polish.**
- B3: module-file decl/impl mapping (LinearPolicy.h, PolicyNet.h).
- B4: Agent.h book-text extraction (move ~1,400 lines to an appendix doc).
- B5: Gym ownership docs + const overloads + LoadDLL status.
- B6/B8/B9: dtype_, Multiverse, School* status labels.
- D2: document/fix the no-stdlib boundary.
- D4/D5: ASCIICrabs README rename + move internal plans.
- E1/E2: CrabsTK root CMake + subdir status labels.

**Window 3 — worlds + spec completeness.**
- C1/C8: per-world status labels + CMake for shippable worlds.
- C3: finish or quarantine TileWorld.
- C4: dedupe AgentMaze / Agent.h copies.
- C5: move probes to _Seams or delete.
- C6: remove ghost dirs.
- C7 decision: ship the 3 untracked worlds or exclude.
- F2/F3/F4: complete the _Spec TOC (sub-READMEs), fix stale org/repo names,
  add the SCRIPT↔Crabs↔ASCIICrabs name mapping.
- E3/E4: CrabsTK MKL doc + _Footer.h path check.
- A4: top-level NOTICE for the two third-party licenses.

### 0f.4 Open decisions (need the user before Windows 1–2)
1. A1: strip `iGeek/AGENT_PLAN.md` from the public tree, or keep it internal?
2. A2: CrabsTK + ASCIICrabs license — same AStarship non-commercial, or
   permissive (MIT-style) for the reusable libs?
3. C7: ship PrisonerWorld/SudokuWorld/HBM in this publish, or exclude?
4. D1: repair TRoom (3 bugs) or deprecate it publicly?

## 0e. STATUS UPDATE — 2026-10-08 (overnight): MONOREPO DONE + PPO actually learns now (astar-mary)

This is the biggest structural change since the plan was written: **iGeek is now
the single engine repo.** Section 0d's "single-repo reorg planned (NOT executed)"
is executed, plus the PPO core turned out to have a degenerate-policy bug that
made training silently do nothing — fixed and verified.

**1. Monorepo: all worlds moved into `iGeek/` (done, committed).**
- `iGeek/` now contains: the framework (Env/Gym/Multiverse/Tensor/Policy/PPO/
  Agent/AgentState/Metrics/School), plus `PacWorld/`, `TileWorld/`, `CarWorld/`,
  `Ulator/`, `WikiWorld/`, `CardsWorld/`, `MazeWorld/`, `PolygonWorld/`,
  `TypingWorld/`, `VirusWorld/`, `Cookbook/`. No more `iGeek*World` siblings at
  the AStarship top level (verified: `ls iGeek*` → only `iGeek`).
- `iGeekVirusWorld.Old` archived into `iGeek/VirusWorld/_Old`.
- Include depths fixed: iGeek-core includes `../iGeek/X` → `../X`; ASCIICrabs
  stays at the AStarship top level (still `../../ASCIICrabs` from a world).
- Commits on branch `Issue10`: `87d5805` (framework cleanup: .inl→.hxx, agent/
  RL/metrics layers, TileWorld spine, seams migrated), `5b268cc` (monorepo move
  + include depths), `2bfcf12`/`4085fbc` (de-nest CardsWorld/PacWorld/VirusWorld
  — git had tracked them as gitlinks), `6788ce5` (reference-name strip + PacWorld
  fixes), `4ad6a6c` (PacWorld probe overflow fix). Working tree clean.
- **NOT pushed** (user owns pushes). `origin/Issue10` is stale; `origin/master`
  has PR #11 merged (55d7e85). Issue10 needs a PR or push by the user.
- **Stale sibling includes remain** in the frozen SFML/Qt-era worlds (Script2,
  KabukiToolkit, SFML dirs no longer exist in the tree). Those worlds' .vcxprojs
  won't build headless anyway. Decision pending: leave as-is or strip.

**2. PPO root cause + fix: argmax sampling made training a no-op (the real bug).**
- Symptoms: CardsWorld's PPO seam test segfaulted pre-move (stale binary masked
  it — "gradcheck PASSED" was from a build that predated the PPO test running).
  After the ASan fix, the test ran but *failed*: `r1 > r0` false, reward flat at
  0.0 across 300 steps, weights just decaying.
- Two bugs found and fixed:
  1. `TToyGym` (CardsWorld `_Seams/06.PPO.hxx`) never allocated `Gym::actions_`
     → `TPPO::CollectRollout` wrote `gym_.Actions()[i]` to null (SEGV). Fixed:
     toy gym allocates/frees `actions_`.
  2. **The load-bearing one:** `TLinearPolicy::SampleActions` was pure argmax —
     a degenerate deterministic policy. Zero exploration → PPO's gradient carries
     no signal, the value head can't bootstrap, weights drift but reward never
     moves. Replaced with seeded-stochastic sampling (seeded LCG + CDF over the
     softmax), matching the reference fused-kernel's `sample_logits` pattern.
- Verified: CardsWorld full seam suite passes under ASan/UBSan (exit 0). The
  PacWorld PPO probe (same TPPO + TLinearPolicy path) benefits too.

**3. Reference-name strip (user directive 2026-10-08).**
- All `Puffer`/`PufferLib`/`puf_*` references removed from iGeek source + docs.
  Replaced with neutral terms: "vectorized-batch", "fused PPO kernel",
  "reference PPO fork", "preallocate-at-init pattern". The `~/3P/PufferLib` clone
  stays (it's a real reference), but iGeek's code no longer names it.
- Kept: `TypingWorld/item.hxx` "Pufferfish" (a fish species, not the lib).
- Verified: `grep -ri puffer` in iGeek (excl .git, excl Pufferfish) → 0 hits.

**4. PacWorld env + probe fixes (verified by actual runs).**
- `PacWorldEnv::Reset` now rewinds `rng_` to `seed_` before regenerating the
  maze. Without the rewind, consecutive episodes saw consecutive LCG states and
  the layout drifted every episode (not i.i.d. per seed).
- `_probe_pacworld.cpp`: `OK()`/`FAIL()` had hardcoded `write(1, what, 96)` — a
  global-buffer-overflow caught by ASan. Fixed to strlen. Then a *second*
  overflow: the greedy-value print block used a 64-byte stack buffer that
  overflowed once the greedy values grew, and the write landed on corrupted data
  → the probe hung after printing "ok: ppo trainstep". Fixed: 128-byte buffer,
  bounded copies, hard cap. Ad-hoc verification (11/11 checks): ASan clean,
  -O2 clean, `PACWORLD_PROBE_OK` exit 0.
- **The trend is real now:** probe prints `greedy before=-10 after=239` (reward
  in hundredths) — one TrainStep already moves the needle on PacWorld.
- `_verify_pacworld.cpp`: `PASS linear-learns (envs=8) before=0.5 after=65.5`,
  `PASS transformer-runs (envs=8)`, `VERIFY_TRAIN_PASS`, exit 0.

**5. What this means for the Qualia plan (sections 3–5).**
- The engine layer the Qualia world sits on is now a single repo with a *working*
  RL core (not just headers). Phase 1 (skeleton `QualiaWorld`) can proceed on
  `iGeek/` directly — no more cross-repo includes.
- The agent framework (0b) + RL core (0a/0d) + now stochastic sampling = the
  `TModelAgent` tier can be driven by a real `TPolicy`/`TPPO` instead of a
  hand-written update, which is exactly what the qualia study needs to measure
  internal-state structure under learning.
- The "red representation" probe (section 3.5) is the natural next experiment:
  train a `TLinearPolicy` on a red-token stimulus, then run the `LinearProbe`
  (Metrics.h) over the `AgentState` words to see if "red present" is
  linearly-decodable from internal state.

**Open items (not done):**
- Push `Issue10` (user owns). `origin/Issue10` is 8 commits behind.
- Stale sibling includes in frozen SFML/Qt worlds (Script2/KabukiToolkit/SFML) —
  decision: leave or strip.
- CrabsTK + ASCIICrabs housekeeping (stale includes, dead references) — not started.
- `TTransformer` full block backprop wired into `TPPO` (currently only
  `TLinearPolicy` has `SgdStep`; the transformer runs clean but its gradients
  aren't used by the PPO update yet). That's the next RL milestone.
- `QualiaWorld` Phase 1 skeleton — unblocked, not started.

## 0c. STATUS UPDATE — 2026-10-08: astar-mary verified the framework layer (t_197929d1)

astar-mary **independently re-verified** the framework layer low-level
reported in section 0b. Built a fresh probe TU against the current repo state
(all files already in place, nothing rebuilt from scratch) and ran the full
check suite:

- `g++ -std=c++2b -O2 -I. -I../ASCIICrabs/_Seams -I../ASCIICrabs -I..`
  — compiles clean, zero warnings from the iGeek headers.
- Runtime: all six probe checks pass (AgentState round-trip, TTableAgent
  lookup, TReflexAgent rules, TModelAgent state accumulation, BehaviorMetrics
  tallies, LinearProbe end-to-end 100% train+eval accuracy on a synthetic
  linearly-separable target).
- `-fsanitize=address,undefined` — clean (no ASan/UBSan reports from the
  framework files).

The framework is confirmed ready for Qualia World Phase 1+. Phase 1
(skeleton world: `_Main.cpp`, `QualiaWorld.h/.hxx`, `_Seams/`) can now
proceed on this foundation.

## 0d. STATUS UPDATE — 2026-10-08 (later): RL core verified headless + TileWorld headless spine + single-repo reorg planned (astar-mary)

This session did three things that bear on iGeek as the single engine repo.
All verified; nothing here changes the Qualia plan above — it adds the
headless-RL + game-world engine layer the Qualia world will sit on.

**1. RL core verified headless + fast; 4 memory-safety bugs fixed.**
`TPPO::TrainStep()` on `PacWorldGym` (the PacWorld env, still at
`iGeekPacWorld/` until the reorg) runs ~650–2,600 steps/s at `-O2` and is
learning (greedy reward climbs over training). The prior "one TrainStep = 2
min" was the `-O0` probe build, not a real bottleneck. Running the real `TPPO`
loop at batch≥8 under ASan surfaced 4 bugs (all in the RL core, all fixed):
(1) bias-broadcast overflow — `TTensorAdd(logits, b_, logits)` with a `[K,1]`
bias and `[B,K]` logits → new `TTensorAddBiasInPlace` (Tensor.h/.hxx), 10 call
sites in LinearPolicy.hxx + PolicyNet.hxx; (2) `TLinearPolicy::SampleActions`
LSE out-buffer `[1,1]`→`[B,1]`; (3) VLAs in `TPPO::CollectRollout`→fixed
`GymEnvMax`; (4) the duplicate `TTransformer::SampleActions` LSE bug (caught by
the verifier — fix the class, not just the reported site). ASan-clean on both
policies at envs=8.

**2. Headless geometry + occlusion primitives landed in ASCIICrabs/.**
`ASCIICrabs/TVec.h` (`TVec2F`/`TVec2I`/`TFloatRect`/`TIntRect`, VHT-conformant,
no stdlib) and `ASCIICrabs/TOcclusion.h` (`TOcclusionMap`: occluder grid =
static collision tiles ∪ dynamic `occlude_=1` sprites, ≤8 observers, DDA
line-of-sight, per-observer visibility grid, no stdlib). This **resolves the
open dependency in section 0b** ("`TVec2F`/`TFloatRect`/`TSprite` are hers to
build") — `TVec2F`/`TFloatRect` now exist and are tested
(`ASCIICrabs/_test_occlusion.cpp`, 9 hand-verified cases, ASan-clean,
`OCCLUSION_TEST_PASS`). `TSprite` is deliberately NOT a render object: in the
headless core a "sprite" is data only (`_::IUD texture_id` + `_::BOL occlude_`
on the entity), and the occlusion map is the spatial-awareness layer a
renderer (or an RL observation) reads.

**3. TileWorld headless spine ported to `iGeek/TileWorld/`.**
The SFML `iGeekTileWorld/` ECS core was stripped to a headless, no-stdlib spine
at `iGeek/TileWorld/`: Entity (owns `_::TVec2F pos_`), ComponentHitbox,
ComponentMovement, Tile, TileRegular, TileSpawner(Enemy), TileMap (with
`TOcclusionMap` wired in — rebuilds occluders + recomputes visibility each
`Update`), Enemy, EnemySystem, Player. `Render()` deleted (headless);
`std::pow`/`std::stringstream`/`std::cout` all replaced; `Enemy::Chase` uses a
no-`<cmath>` Newton–Raphson `1/sqrt`. **Incomplete:** the headless `_main`
(demo: build a map, spawn player+enemies, step 200, print the visibility
field) was being written when the session ended — that is the immediate next
task. Attribute/Skill/Animation/weapons/Inventory are 2nd-pass (out of scope
for the spine).

**4. Single-repo reorg planned (NOT executed).** Decision: `iGeek/` becomes
the single engine repo; each `iGeek*World` moves into `iGeek/<base>` (PacWorld,
TileWorld, CarWorld, …), `.git` dropped; non-iGeek repos (CrabsTK, ASCIICrabs)
stay separate. Dry-run verified (`scratch/reorg_dryrun.sh`); ~9 include-path
edits (`../iGeek/X`→`../X`); headless port is canonical `iGeek/TileWorld/`,
SFML original deleted locally (safe in frozen remote). **Blocked on the user's
commits/push** (user owns tickets/branches; `iGeek` has 3 unpushed commits +
69 dirty files, `iGeekCardsWorld` 3 unpushed — push both before any `.git`
deletion). A subagent commit (`8e58b0b`) was made and then undone via
`git reset --soft`; the working tree is back to the user's pre-commit state.

**Invariants added this session (apply to all iGeek engine code):** functional
casts `IUD(x)`/`ISC(x)`/`FPC(x)` (never C-style); no VLAs; profile at `-O2`
before claiming a bottleneck; ASan is the gate for batch≥8; grep for sibling
copies when fixing a bug. See `iGeekPacWorld/README.md` (speed table + the 4
bugs) and `CrabsTK/AGENT_PLAN.md` (the "What the TileWorld headless
extraction NEEDS" section) for the full writeup.

## 0b. STATUS UPDATE — 2026-10-08: agent framework layer built + verified (low-level)

The iGeek **framework layer** astar-mary builds on (plan sections 3.3-3.5) is now
**implemented and verified**. All in `~/AStarStarship/iGeek/` (branch Issue10),
compiled + run against the latest ASCIICrabs (GCC 13, C++23), ASan/UBSan clean,
all probe checks pass (exit 0). Kanban task `t_197929d1` dispatched to
astar-mary on the `astarship` board.

New files (world-agnostic — NO red/blue/green knowledge in the framework):
- `AgentState.h/.hxx` — the internal-state memory model (decision 3.5 #3):
  flat fixed-size IUC word array in an `ObjectFactoryHeap` Autoject;
  snapshot = copy buffer bytes; probe = linear read over words. DLL-ready.
- `Agent.h/.hxx` — the three GENERIC agent tiers (section 3.3), canonical home,
  per the constraint that this file stays free of world-specific stimulus
  knowledge: `AgentBase` (Percept->Action + state contract), `TTableAgent`
  (tier 1, history->action), `TReflexAgent` (tier 2, condition-action on the
  current percept), `TModelAgent` (tier 3, maintains an internal `AgentState`
  world model and exposes `State()` for the metrics to probe).
- `Metrics.h/.hxx` — the measurement protocol (section 3.4): `BehaviorMetrics`
  (correct/FP/FN/latency + rates), `StateRecord`/`StateRecordSet` (capture
  internal state vectors tagged by context label, fixed-capacity, no
  std::vector), and `LinearProbe` (full-batch logistic regression over the
  state words). Verified end-to-end: on a synthetic linearly-separable target
  the probe reaches 100% train+eval accuracy, proving the
  capture->records->probe pipeline works.

Verification: standalone probe TU (`_probe_agents.cpp`, since removed) ran all
six checks -> "ALL iGeek AGENT-LAYER PROBE CHECKS PASSED" (exit 0);
`-fsanitize=address,undefined` clean (the one UBSan misaligned-store note is a
pre-existing upstream ASCIICrabs `Puff.hpp` issue, not in these files).

What this does NOT do: it is the framework, not the world. `iGeekQualiaWorld`
(Phase 1+) is still astar-mary's. The `TModelAgent` internal update is a
deliberately simple generic linear transform (accumulate percept words into
slots) so the tier is testable in isolation; specializing it (or adding a
ProblemSolvingAgent tier) goes through a master-agent review card per the
section 3.3 constraint.

Open dependency (astar-mary, hetero-tuple): `TVec2F`/`TFloatRect`/`TSprite` are
hers to build. I did NOT hand-roll float-vector/sprite types in the framework —
the qualia scene state stays plain IUC scalar datums (grid positions + signature
words, per plan 3.2) until her types land, so no collision is expected.


## 0a. STATUS UPDATE — 2026-10-07: framework migrated + vectorized RL layer added (low-level)

The iGeek framework headers were **migrated to the latest ASCIICrabs API and
extended with a vectorized-batch RL layer** on 2026-10-07. This supersedes
parts of the plan below (which still describes the *stale* headers). What
changed, and what it means for the Qualia work:

**Migrated (stale → latest API):**
- `Env.h`, `Gym.h`, `Multiverse.h`, `EnvGoal.h`, `EnvReward.h`: `public Room`
  → **standalone** (see below for why), `STA*` (dead type) → `const CHA*`,
  `Star(CHN, ...)` → `Star(CHC, ...)`, fixed copy-pasted include guards.
- `FPC` (float32) is now the per-step scalar reward type throughout
  (`Env::ComputeReward` returns `FPC`, `EnvReward` wraps an `FPC`). This
  matches the plan's section 3.1 "single scalar reward" decision and
  the flat per-env reward vector.

**Why standalone (not `public TRoom`):** verified 2026-10-07 that
`TRoom<CHD, CHD>` **fails to compile** against the latest ASCIICrabs —
`TRoom::Main`/`Star` have pre-existing upstream bugs (`TDoor` has no `ExecAll`;
`OpFirst/OpLast` const-ISC* vs const-DTB* mismatch; `OpPush` undeclared) at
`../ASCIICrabs/Room.hpp:297,310,332,341,348`. Same blocker that made the card
world's `BlackjackEnv` standalone. **Logged to `../ASCIICrabs/AGENT_PLAN.md`
(Gap 1, CRITICAL).** Until that's fixed upstream, iGeek env/gym classes stay
standalone; the Script2 data-node role is served by `Crabs*` pointers in
`EnvGoal`/`EnvReward` instead of embedded `TRoom` members.

**New (vectorized-batch RL layer — the "AI gym" primitives):**
- `Tensor.h` — `TTensor` POD (row-major `FPC*` buffer) + declared linear
  kernels (MatMul/Softmax/LayerNorm/Relu/Add/Scale/RowLogSumExp). Impls are
  the next milestone; the *type* is the primitive. **The kernels themselves are
  an ASCIICrabs ask (Gap 2) — they don't yet exist anywhere.**
- `Policy.h` — `TPolicy` interface (obs → logits+value, sample actions,
  backward → grad_logits/grad_value) + `TTransformer` (MuSE-style net,
  declared, not yet implemented). This is the analog of the fused-kernel policy
  head in `src/algo.cu`.
- `PPO.h` — `TPPO` (GAE + clipped policy loss + value-clip + entropy) over a
  vectorized `Gym` batch + a `TPolicy`. The direct analog of the fused-kernel fused
  PPO kernel, in CPU FPC (no CUDA).
- `Gym.h` — now carries the **vectorized batch interface** (`Observations()`,
  `Actions()`, `Rewards()`, `Terminals()`, `ActionMask()`, `StepBatch()`,
  `ResetBatch()`), the vectorized-batch `Env*`/`Agent*` analog. The PPO loop steps the
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
.vcxproj still list `..\Crabs\*` sources). **RESOLVED 2026-10-08:**
astar-mary verified the build picks up the ASCIICrabs include root —
probe TU compiles clean with `-I../ASCIICrabs` against the current
repo state (see section 0c). Framework layer is confirmed ready.
Exit: build include root confirmed. **DONE.**
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
  logic, metrics. Drives Phases 1-5. Framework layer verified
  2026-10-08 (section 0c); Phase 1 is unblocked.
- low-level: ASCIICrabs support — verify data-type choices, confirm
  Room-to-bytes dump path for state snapshots, RAMFactory/Autoject
  memory model, any upstream constraint. Reviews the Crabs-facing parts
  of each phase.
- master-agent: plan arbitration (section 3.5), cross-world coordination
  (PrisonerWorld_ relationship), scope/governance.
- sudoku: SudokuWorld built using iGeek project-types only (IUD, ISC).
  No C++ standard library code. Built with CMake targeting Release.
  Uses backtracking solver. Verified working with classic Wikipedia
  puzzle (exit 0).

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
