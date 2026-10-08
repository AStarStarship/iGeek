# iGeekPacWorld — Headless RL Env

A headless, no-stdlib Pac-Man environment for the iGeek RL framework
(ASCIICrabs C++23). The original SFML arcade game is preserved as the
reference for game *rules*; this is a grid-based, discrete re-implementation
that the PPO loop can step deterministically.

## What's here (new, 2026-10-07)

- `PacWorld.h` / `PacWorld.hxx` — `PacWorldEnv` (single env) +
  `PacWorldGym` (vectorized, inherits iGeek's `Gym`).
- `_probe_pacworld.hxx` / `_probe_pacworld.cpp` — build + run probe.

## Design

- **Grid, discrete**: one cell per action (no sub-cell motion). The original
  was continuous pixel-based; discrete is cleaner + deterministic for RL.
- **Observation** (fixed length, Puffer "scalar vector"): a `(2R+1)^2` local
  window around pac, `R=3` → 49 cells × 7 channels (wall/dot/superdot/bonus/
  ghost/weakghost/pac) + 1 progress feature = **344 features**.
- **Actions**: 5 (Stay, Up, Down, Left, Right).
- **Rules** (from the original PacWorld):
  - Dots +5, SuperDot +25 (ghosts weak `WeakSteps`), Bonus +500.
  - Ghosts: greedy chase (minimize Manhattan distance to pac, no reversal).
  - Weak ghost eaten: +100, respawn. Strong ghost collision: −10, lose a life.
  - 3 lives; all dots eaten → win (+50); `MaxSteps`=200 also terminates.
- **Determinism**: seeded LCG per env; same seed → same maze + same episode.

## Verified (2026-10-07)

- Env steps 1000 actions in <1ms (the env itself is instant).
- Two fresh envs, same seed → identical first observation (deterministic).
- `PacWorldGym` batches N envs; the iGeek `TPPO` loop steps it without
  crashing (no-stdlib, no `std::` includes).
- The original SFML files are untouched (they remain the human-play renderer).

## Build (probe)

```
cd iGeekPacWorld
g++ -std=c++2b -I. -I../ASCIICrabs/_Seams -I../ASCIICrabs -I.. \
    _probe_pacworld.cpp -o /tmp/probe_pacworld
/tmp/probe_pacworld
```
Exits 0 on success; prints `ok:` lines + `PACWORLD_PROBE_OK`.

## Verified headless training speed (2026-10-08, `-O2`)

Headless RL training is **fast**: `TPPO::TrainStep()` on `PacWorldGym` runs at
~650–2,600 steps/s and is **learning** (greedy reward climbs over training).
The earlier "one TrainStep = ~2 min" was an artifact of the **`-O0` probe
build**, not a real bottleneck.

| config (steps envs horizon epochs) | time  | speed     | greedy reward |
|-----------------------------------|-------|-----------|---------------|
| 2000  8  4  4                      | 0.79s | 2,516/s   | 52 → 6,427    |
| 5000  8  4  4                      | 1.90s | 2,635/s   | 52 → 6,552    |
| 2000  8  8  4                      | 1.57s | 1,275/s   | 52 → 114      |
| 10000 8  8  4                      | 7.71s | 1,296/s   | 52 → 177      |
| 10000 8 16  4                      | 15.1s | 662/s     | 52 → 177      |

`batch=4` runs show `-10 → -10` only because that run's greedy-eval seed was
unlucky; `batch=8` proves learning end-to-end.

### Memory-safety bugs found + fixed in the iGeek framework (this session)

Running the real `TPPO` loop at larger batches (ASan) surfaced three real
overflows that only tripped at `envs=8` (the Gym's `GymEnvMax`):

1. **Bias-broadcast overflow** — `TLinearPolicy::Forward` /
   `TTransformer::Forward` did `TTensorAdd(logits, b_, logits)` where `b_` is a
   `[K,1]` bias column and `logits` is `[B,K]`; `TTensorAdd` assumes matching
   shapes and read past `b_`'s single column. Fixed with a new
   `TTensorAddBiasInPlace(X, bias)` (Tensor.h/.hxx) that broadcasts
   `bias[j,0]` across every row; call sites updated in `LinearPolicy.hxx`
   (2) and `PolicyNet.hxx` (8). Residual adds (`TTensorAdd(H, Res, H)`, both
   `[B,dim]`) are legitimate and left as `TTensorAdd`.
2. **RowLogSumExp output sized `[1,1]`** — `TLinearPolicy::SampleActions`
   allocated `lse` as `[1,1]` but `TTensorRowLogSumExp` writes `out.At(i,0)`
   for **every** row `i in [0,B)`. Fixed by sizing `lse` to `[B,1]` and
   computing LSE once for the whole batch (also a perf win — it was
   re-computed per row before).
3. **VLAs in `TPPO::CollectRollout`** — `ISC acts[B]; FPC lp[B];` are
   non-standard C++ (GNU extension) and corrupt the stack at `-O2` with
   larger `B`. Fixed to fixed-size `ISC acts[GymEnvMax]; FPC lp[GymEnvMax];`.
4. **`TTransformer::SampleActions` LSE sized `[1,1]`** — same bug as #2 but in
   the transformer policy's own copy (PolicyNet.hxx). The ASan verifier caught
   it (I'd only fixed `TLinearPolicy` before). Fixed identically: `lse` sized
   `[B,1]`, LSE computed once per batch.

After the fixes, the verifier (`_verify_pacworld.cpp`) is **ASan-clean on both
policies** (`TLinearPolicy` and `TTransformer`) at `envs=8`, and `-O2` runs
exit 0 with a reward increase.

## Build (probes)

```
cd iGeekPacWorld
# Headless training-speed probe (N real TrainSteps, steps/s + reward gain):
g++ -std=c++2b -O2 -I. -I../ASCIICrabs/_Seams -I../ASCIICrabs -I.. \
    _train_speed.cpp -o /tmp/pw_ts
/tmp/pw_ts <steps> <envs> <horizon> <epochs>     # defaults 50 4 4 4

# Determinism / env probe:
g++ -std=c++2b -O2 -I. -I../ASCIICrabs/_Seams -I../ASCIICrabs -I.. \
    _probe_pacworld.cpp -o /tmp/probe_pacworld
/tmp/probe_pacworld

# Verifier (both policies, learning + ASan-clean; run the ASan build for safety):
g++ -std=c++2b -O1 -g -fsanitize=address -I. -I../ASCIICrabs/_Seams \
    -I../ASCIICrabs -I.. _verify_pacworld.cpp -o /tmp/pw_verify
/tmp/pw_verify        # expect VERIFY_TRAIN_PASS, exit 0
```

## Next milestones

1. Tune the PPO hyperparameters (the `batch=8, horizon=4` config learns fastest;
   larger horizons plateau lower on this small maze).
2. Wire the `TTransformer` policy (not just `TLinearPolicy`) into PacWorld and
   confirm the bias-broadcast fix holds for the block biases under ASan.
3. Add a seam test to `_Seams/` that asserts a reward increase over training.
4. SDL3 display layer (`~/3P/SDL3/`) for optional visualization — watch the
   trained agent play. Separate from the headless experiments.

## License

Copyright [AStarship™](https://astarship.net).
