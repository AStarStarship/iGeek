# iGeekVirusWorld AGENT_PLAN — Genetic Virus Evolution in ASCII

Status: DRAFT for Captain review
Author: astar-mary
Date: 2026-09-12 03:00 UTC

## 0. Purpose and the viral question

We want to understand what it takes for a self-replicating pattern to
evolve complexity in a constrained environment. This is NOT a game —
there is no player, no score, no win condition. The experiment is: put
genetic viruses in a terminal grid, let them drift, reproduce, and
mutate, and watch what happens to the shape distribution over
generations. Does complexity emerge? Does it plateau? Does it collapse?

The hypothesis: if genetic evolution produces structure, it must show
up as (a) increasing vertex counts (more complex polygons), (b)
increased shape diversity (different DNA → different shapes), and
(c) generational turnover (new shapes replacing old). The terminal is
the microscope: every frame is a photograph of the population's
morphology.

Scope guard: this is a simulation + measurement plan. It is not a
biology paper. Deliverables are a buildable world, a baseline agent,
and a metrics protocol.

## 1. What already exists (verified by reading the repos)

### The new code (this rewrite, 2026-09-12)
- `Host.h/.hxx` — Grid surface. Width × height CHA buffer. SetCell,
  GetCell, InBounds, Clear, Render. No GUI, no pixels. The host IS the
  terminal.
- `Virus.h/.hxx` — Genetic polygon virus. DNA is a 16-character ASCII
  string (nucleotide alphabet: . - | + *, encoding radial 0-4). Shape
  is derived from DNA: spikes (radial >= 2) become polygon vertices
  at 8-way directions. Drifts on a bounded Lissajous path. Mutates one
  nucleotide per call (adjacent radial step).
- `VirusPopulation.h/.hxx` — Population manager. Owns an array of
  Virus (value semantics, max 64). Handles Step, Reproduce (age >=
  20 frames → spawn mutated child), Cull (oldest die when over max).
- `Vireworld.h/.hxx` — Top-level simulation. Init(w, h, pop), Step
  (advance + reproduce + cull + track generations), Render, Summary
  (population, born, generation, vertex count).
- `_Seams/` — Standard seam tree: _Config.h, _Seams.h, _Main.cpp,
  _Tests.hxx, 01.Core.hxx (test: 48x24 grid, 8 viruses, 25 frames),
  02.Release.hxx (empty).
- `_Legacy/` — 38 old Java-port files (GeneticPolygon, BezierPanel,
  Host.java, Virus.java, etc.) archived for reference. Do not
  compile.

### The old code (archived in _Legacy/)
- Main.cpp — A Java JPanel port with C++ syntax errors (CellEvolutionaryInterface
  extends JPanel, paintComponent, addMouseListener). This was the
  "dumb Java 2D game engine" — it should never have been needed.
- GeneticPolygon.h/.hxx — Base class for virus shapes. Bezier curve
  movement, collision detection, trail rendering. All Java 2D API.
- Virus.h/.hxx — Extended GeneticPolygon with drift, Bezier path,
  collision, trail. "Oh shit! I need to stop programming in class !!!"
- VirusPopulation.h/.hxx — Population with addVirus, selectRandomVirus,
  update, draw, boxCollidingViruses. Java 2D Graphics.
- BezierPanel, GeneticPolygonPanel, etc. — Swing panels for GUI.

### The framework (iGeek/)
- `Env` — Room with pure-virtual ComputeReward. Reward compares
  achieved vs desired EnvGoal.
- `Gym` — Room; AddEnv(Env*), LoadDLL(CHA* library).
- `Multiverse` — Room; container for many Envs.
- `EnvGoal` — Operand holding (observation, desired_goal, achieved_goal).
- `Agent.h` — Russell & Norvig reference port (table-driven, reflex,
  model-based reflex, tree/graph search, BFS).

### Reference worlds
- `iGeekPolygonWorld/` — Sister project with the same Java-port DNA.
  Same GeneticPolygon code, same Bezier curves, same panels.
- `iGeekPacWorld/` — Cleanest terminal-first pattern. Simple grid,
  ASCII characters, no GUI.
- `iGeekCarWorld/` — AgentMaze on a grid. Closest to what VirusWorld
  should be.

## 2. Conventions we must follow (from ASCIICrabs AGENTS.md + templates)

- **Chimera case:** 2-space indent, root namespace unindented, K&R
  braces, ~80 cols. Types/functions UpperCamelCase; mutable members
  and locals lower_snake_case (class members trailing `_`, struct
  members without); macros UPPER_SNAKE_CASE.
- **No C++ std library.** No `<string>`, `<vector>`, `<iostream>`.
  Use Crabs types: ISN, ISC, IUW, CHA, BOL, FPD, STA, etc.
- **Module format:** `.h` decl, `.hpp` templates, `.hxx` single-TU impl.
- **No exceptions** in low-level paths; return status codes (ISN error).
- **Seam testing:** in-order tree tests under `_Seams/`, selected via
  SEAM macro. Run the built binary to execute.
- **Copyright:** `// Copyright AStarship <https://astarship.net>.`
- **Build:** `g++ -std=c++2b -g -o Vireworld _Main.cpp -I. -I.. -I../../ASCIICrabs`
  (C++2b required for ASCIICrabs BSeq.hpp constexpr static locals)
- **ASCIICrabs is EDITABLE** (2026-09): the Captain holds a master
  copy; if a change torches the build, the master copy is the rebuild
  source.

## 3. What works today (verified by build + run)

- Build: clean compile with g++ 13.3.0, -std=c++2b. No errors, some
  warnings from ASCIICrabs (AlignUpArray, Puff.hxx large constants).
- Run: 48×24 grid, 8 viruses, 25 frames.
  - Frame 1-19: 8 viruses drifting on Lissajous paths, Gen 0, Vertices 0.
  - Frame 20: reproduction triggers → 16 viruses, Gen 1, Vertices 0.
  - Frame 25: 16 viruses, Gen 1, Vertices 0.
  - Unit tests pass: "Unit tests completed successfully! (:-)+=\<"
- The viruses start as circles (all '.' DNA) and need more mutation
  cycles to develop spikes. The current test runs only 25 frames —
  not enough for interesting morphology.

## 4. What's missing (the plan)

### Phase A: Make it actually evolve (this is the hard part)
The core problem: "It is hard to make genetic polygon viruses make
sense." The DNA→shape mapping is too simple. Here's what needs to
happen:

1. **DNA encoding.** Current: 16 chars, 5 nucleotides, radial 0-4.
   This gives max 16 spikes but they're all in 8 fixed directions.
   The polygon is always a subset of an octagon. Need:
   - [ ] Vary the angle per nucleotide (not just 8 fixed octants).
   - [ ] Encode both radial AND angle in the DNA (e.g., 2 chars per
         nucleotide, or a larger alphabet).
   - [ ] Consider: 32-bit DNA word → 32 radial values at 360/32 =
         11.25° spacing. True polygon, not just an octagon.
   - [ ] Mutation should be a point mutation (one nucleotide) OR a
         crossover (swap segments between two parents). Both?

2. **Shape rendering.** Current: 8-way octant with fixed dx/dy. This
   looks like a plus sign, not a polygon. Need:
   - [ ] Render actual line segments between vertices (Bresenham on
         the grid).
   - [ ] The polygon should be visible: outline, not just dots.
   - [ ] Consider: render the DNA string itself next to the virus
         (the genome IS the data — show it).

3. **Reproduction model.** Current: asexual, clone + mutate.
   - [ ] Add sexual reproduction: two viruses that collide swap DNA
         segments (crossover).
   - [ ] Add selection pressure: viruses that are more complex
         (more vertices) reproduce faster? Or: viruses that are
         too complex use more energy (smaller, slower)?
   - [ ] Add environment: the grid has regions with different
         mutation rates or food (food = energy to reproduce).

4. **Metrics.** We need to MEASURE evolution, not just watch it.
   - [ ] Track per-generation: mean vertex count, max vertex count,
         shape diversity (unique DNA strings), population size.
   - [ ] Print a metrics table at the end: gen, pop, mean_verts,
         max_verts, diversity.
   - [ ] This is the data the Captain will use to decide if
         complexity is emerging.

### Phase B: iGeek integration (make it an Env)
The framework expects an Env with ComputeReward. How does a virus
simulation fit?
- [ ] `VirusEnv : Env` — the world IS the environment. The "reward"
  is the complexity metric: reward = mean_vertices * diversity.
  Higher reward = more complex and diverse population.
- [ ] The "agent" is the evolution process itself. The agent's
  "action" is the mutation rate / reproduction rate / selection
  pressure. The agent "learns" by trying different evolutionary
  strategies and getting rewarded for the ones that produce
  complexity.
- [ ] This is where the "gym" metaphor fits: the gym is the
  environment, the agent is the evolutionary strategy, the reward
  is the complexity metric.
- [ ] Connect to iGeek/Gym.h: AddEnv(VirusEnv*).

### Phase C: Terminal experience (make it watchable)
The Captain said "it needs to work in the terminal in ASCIICrabs
first." The current output is 25 static frames printed to stdout.
That's a movie in text form — but it scrolls off the screen.
- [ ] Add a "live mode" that clears the screen each frame (ANSI
  escape: \033[2J\033[H) and redraws. Watch the viruses move.
- [ ] Add a legend: what each character means (o = circle, O = core,
  + = spike, * = big spike, . = empty).
- [ ] Add a status bar: frame, population, generation, vertex count,
  diversity.
- [ ] Add keyboard controls (CIn.h): 'q' to quit, 's' to pause,
  'm' to force mutation, 'r' to reset.
- [ ] Speed control: frames per second (default 10, 1-60 range).

### Phase D: Delete or keep the Java stuff
The Captain said "We can delete it, we can turn it into something
else." The _Legacy/ folder has 38 files of Java 2D code. Options:
- [ ] DELETE _Legacy/ entirely. The Java code was "completely half
  baked" and "dumb." It served its purpose (teaching the Captain
  what NOT to do).
- [ ] KEEP _Legacy/ as a historical artifact. The old GeneticPolygon
  code has interesting ideas (Bezier movement, trail rendering,
  collision detection) that could be ported to the new terminal
  format.
- [ ] PORT the interesting parts: the Bezier curve movement could
  become a grid-based path (a virus follows a curved path through
  the grid). The trail could be a fading ASCII trace.

## 5. Decision points (for the Captain)

1. **DNA model:** 16 chars / 5 nucleotides (current, simple) vs
   32-bit DNA word (true polygon) vs 64-bit (more complex shapes)?
   The Captain said "it is hard to make genetic polygon viruses make
   sense" — the DNA model is the core of that difficulty.

2. **Reproduction:** Asexual (current) vs sexual (crossover) vs
   both? Sexual reproduction creates more diversity but is more
   complex to implement.

3. **Selection pressure:** None (neutral drift) vs complexity
   selection (more vertices = faster reproduction) vs energy
   selection (more vertices = more energy = slower reproduction)?
   This determines whether complexity emerges or is selected against.

4. **iGeek integration:** Make it a full Env with ComputeReward
   (Phase B) or keep it standalone (just a simulation)? The iGeek
   framework adds the "gym" layer — is that needed for the virus
   experiment, or is the simulation self-sufficient?

5. **Java code:** Delete _Legacy/ or keep as reference? The Captain
   said "I can't change the past" but also "we can delete it."

6. **Live mode:** ANSI clear-screen (watchable) vs static frames
   (scrollable, easier to diff)? For the Captain's 96-core Xeon
   with 96GB (soon RTX 5060 Ti), live mode is trivial. For the
   current 10-core/24GB, it's also fine (it's just text).

## 6. Build and test commands

```bash
cd ~/AStarStarship/iGeekVirusWorld/_Seams
g++ -std=c++2b -g -o Vireworld _Main.cpp -I. -I.. -I../../ASCIICrabs
./Vireworld
```

Expected output (first 5 lines):
```
................................................
................................................
...
```
Expected output (last 3 lines):
```
Frame: 25  Population: 16  Born: 16  Gen: 1  Vertices: 0
Unit tests completed successfully! (:-)+=<
```

### Switching seams
Edit `_Seams/_Config.h`:
- `#define SEAM VIREWWORLD_CORE` → runs 01.Core.hxx test (default)
- `#define SEAM VIREWWORLD_RELEASE` → runs 02.Release.hxx (empty)

### Adding a new seam
1. Add `#define VIREWWORLD_NEWTING  3` to `_Seams/_Seams.h`
2. Create `_Seams/03.NewTing.hxx` following the 01.Core.hxx pattern
3. Include it in `_Seams/_Tests.hxx` and add to TTestTree<>
4. Set `SEAM` to 3 in `_Seams/_Config.h`

## 7. Files affected by this plan

```
iGeekVirusWorld/
  AGENT_PLAN.md          (this file — for Captain to edit)
  _Config.h              (project config — new)
  _ConfigDefault.h       (defaults, includes ASCIICrabs defaults)
  _ConfigHeader.h        (includes ASCIICrabs/_ConfigHeader.h)
  _ConfigFooter.h        (includes ASCIICrabs/_ConfigFooter.h)
  _Package.hxx           (package: includes ASCIICrabs + all modules)
  _Test.h                (includes ASCIICrabs/Test.h)
  _Undef.h               (undefines YOUR_MOM)
  _Debug.h               (debug macros)
  _Release.h             (release macros)
  Host.h                 (grid surface declaration)
  Host.hxx               (grid surface implementation)
  Virus.h                (genetic virus declaration)
  Virus.hxx              (genetic virus implementation)
  VirusPopulation.h      (population manager declaration)
  VirusPopulation.hxx    (population manager implementation)
  Vireworld.h            (top-level simulation declaration)
  Vireworld.hxx          (top-level simulation implementation)
  _Seams/
    _Config.h            (seam config: SEAM = VIREWWORLD_CORE)
    _Seams.h             (seam numbers: CORE=1, RELEASE=2)
    _Main.cpp            (single-TU entrypoint)
    _Tests.hxx           (test tree)
    01.Core.hxx          (core test: 48x24, 8 viruses, 25 frames)
    02.Release.hxx       (release test: empty)
    pch.h                (MSVC precompiled header, no-op on GCC)
    CMakeLists.txt       (CMake build, requires -std=c++2b)
  _Legacy/               (38 old Java-port files, archived)
  README.md              (project readme)
```

## 8. Next steps (in order)

1. Captain edits this AGENT_PLAN.md — resolves the 6 decision points
   in Section 5.
2. Based on Captain's decisions, implement Phase A (evolution).
3. Build, run, observe. If complexity emerges, capture metrics.
4. Phase B: iGeek integration (if Captain wants the gym layer).
5. Phase C: Terminal experience (live mode, controls).
6. Phase D: Delete or port the Java code.
