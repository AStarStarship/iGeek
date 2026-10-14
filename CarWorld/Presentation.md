# Presentation evidence: CarWorld and MUSE

Copyright AStarship <https://astarship.net>.

## Two results, kept separate

### Native car simulation and classical control — newly measured

AStarship's in-house iGeek/TileWorld/ASCII Crabs stack runs a deterministic,
headless, continuous 2D car plant with four friction-limited tire contacts.
It does not rely on a model server or external physics engine.

- **900/900 goal stops**, 100 initial-pose seeds on each of nine fixed fixtures:
  straight, curves, obstacle detour, gravel, ice, uphill, downhill, launch and
  ice-plus-curves. Worst final goal error 0.9554 m; final speeds below 0.1 m/s.
- **75,109.8 control frames/s**, median of three one-thread CPU runs: about
  **1,502x simulated real time**. Each frame has four FP64 RK4 substeps.
  Includes fixture setup/planning and driver/physics/observations, excludes
  process startup and CSV/trace logging. This is NOT neural training throughput.
- **Physics-sensitive negative controls:** from 10 m/s, two seconds of full
  braking yields approximately 0 m/s on asphalt versus 7.875 m/s on ice;
  unequal left/right grip causes yaw. Fixed-throttle obstacle driving crashes;
  the reference driver detours or stops for an unexpected wall.
- Both classical speed-command and raw-actuator Gym interfaces perform finite
  native PPO updates. **No learned car autopilot has been demonstrated.**
- Release and sanitizer builds pass four CTest entries; the frozen source
  archive rebuilds and passes independently. BikeWorld/PacWorld regressions pass.

The successful drives use a CLASSICAL controller. The fixed-fixture sample
is not random-road coverage, a deployment guarantee, calibrated real-car
validation, precision parking or a GPU benchmark.

Executable:
`/home/astarcale/AStarStarship/iGeek/CarWorld/build/carworld`

Run from the iGeek root:

```sh
CarWorld/build/carworld --scenario all --episodes 100 --seed 7
```

Exact metrics:
`/home/astarcale/AStarStarship/iGeek/CarWorld/build/verified_900_02/drives.csv`

Counts, timing samples, hashes and trace checks:
`/home/astarcale/AStarStarship/iGeek/CarWorld/build/verified_900_02/verification_summary.json`

Frozen executable and project-owned build inputs:
`/home/astarcale/AStarStarship/iGeek/CarWorld/build/verified_900_02/bin/`
`/home/astarcale/AStarStarship/iGeek/CarWorld/build/verified_900_02/code/`

Model equations, assumptions, build commands and limits: `Headless.md`.

### Token ordering and limited-capacity learning — existing archived pilot

This is a separate supervised text experiment, not the car controller and not
reinforcement learning. It predicts selected completion-token histograms from
prefix-token histograms. It does not generate ordered next-token sequences.
The fitting matrix is **132 paragraphs x 128 token types**, with exclusive
paragraph/article/corpus incidence tiers; it is not an observation/action table.
The ordering algorithm is the existing center-error seriation optimizer, not a
Fiedler-eigenvector computation. Keep older spectral-geometry experiments distinct.

The vocabulary and ordering are fitted on 22 training articles, with five
validation and five held-out test articles. Fixed low-dimensional address
features restrict what a 544-parameter model can access. Three neural seeds,
matched architecture/update budgets and train-only fitting are compared.

| Address ordering / features | Trainable parameters | Held-out selected-token NLL |
|---|---:|---:|
| Training unigram | 0 | 3.980631 |
| Frequency / binned | 544 | 3.972587 +/- 0.000176 |
| Random / binned | 544 | 3.977488 +/- 0.003241 |
| Train-fitted seriation / binned | 544 | **3.940664 +/- 0.000303** |
| Train-fitted seriation / Fourier | 544 | 3.942529 +/- 0.000290 |
| Dense learned control | 4240 | 3.922565 +/- 0.009693 |

Lower NLL is better; units are nats per scored selected suffix token. Seriation
improves the matched binned frequency control by **0.031923 nats/token**.
The non-angular binned result is slightly better than Fourier under seriation,
and the larger dense learned model is best here. This supports useful layout
bias for a restricted task, NOT a requirement for radians or a general LLM win.

Important denominator: only **4,894 of 10,379** eligible test suffix occurrences
are selected/scored (**47.153%**). These SDs are variation over three model seeds,
not population confidence intervals. One bounded corpus/split with five test
articles does not establish broad generalization or corpus-scale MUSE quality.

Archive:
`/mnt/MuhNAS/AStar/Training/muse_runs/laplacian_learning_20261010_01/results.json`

Full protocol/report:
`/home/astarcale/AStarStarship/MUSETransformer/Research/laplacian_learning_pilot.md`

For this car task, the 31 original archive hashes were rechecked and metrics
recomputed from all 21 run records; the current 14 targeted learning/wiki/scale
tests passed. The original protocol records checkpoint replay verification.
This task did not retrain those 21 models or re-run that checkpoint audit.

## How to explain the connection

RL repeatedly observes a state, chooses an action, receives a reward and observes
the next state. Policy updates favor actions associated with better returns.
A learned graph representation could encode those observations, but RL does
not inherently require a square correlation table, token ordering or a Laplacian.

Frequent words can be highly connected graph hubs. Placing them centrally is a
layout hypothesis, not automatically a measured compression/prediction gain.
A small or large token address does NOT mean its connections are unlikely;
probability must come from measured graph weights or model predictions. Merely
permuting the full matrix preserves information and pairwise dot products;
layout can matter when a limited feature map/local architecture uses proximity.

The honest bridge is: a working in-house simulator supplies controllable,
repeatable observations/actions/rewards; a separate matched experiment can test
whether a MUSE/Laplacian representation improves learned control. That bridge
has not yet been built. Do not attribute classical driving success to MUSE.

## Next evidence needed

1. Train a raw-action driving policy over complete episodes; address native
   PPO's per-rollout reset/horizon limitation explicitly, without conflating
   short interface smoke with full-route learning.
2. Predeclare held-out layouts, low-grip corners, obstruction cases, seeds,
   vehicle-parameter distributions and sensor errors. Compare learned policies
   with the classical driver and matched representation controls.
3. For 3D, add calibrated contact normals, suspension/load transfer and 6-DOF
   chassis dynamics, preserving this planar archive as the regression baseline.

No publication, commits, pushes or changes to the original MUSE archive were
made by this task. All current driving evidence is CPU-only.
