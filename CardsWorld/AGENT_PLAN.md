# AGENT_PLAN — iGeekCardsWorld AI Gym (CrabsTK/ASCIICrabs, PufferLib-informed)

Status: PLAN (not yet executed). Owner: low-level engineer. Date: 2026-10-07.
Goal: turn the working `BlackjackEnv` into a real, trainable RL gym on the
ASCIICrabs no-stdlib C++23 stack, learning the training-loop shape from
`~/3P/PufferLib` (PPO + MuSE-transform) — implemented in C/C++, not Python.

## 0. Why this plan exists
- `iGeekCardsWorld` now builds, runs, and passes its seam unit tests
  (`CARDSWORLD_CORE`), and `BlackjackEnv` exposes `Reset/Observe/Step/Done/
  Credit/RunPolicy`. But there is no *learner*: `RunPolicy` takes a hand-written
  C function pointer, not a neural policy. To "train agents to play card games"
  we need a policy network + a PPO update loop.
- `~/3P/PufferLib` is a heavily modified **C/CUDA** PufferLib fork (no Python
  `pufferlib` package; `src/algo.cu` is a fused PPO kernel + MuSE-transform
  decoder, `src/pufferenv.h` is the env contract). It is the reference
  *architecture*, but it cannot be dropped in as-is: it is CUDA, C99, uses
  `raylib`/`cJSON`/`glad`, and a `Float`/`Dict`/`obs_t` data model that does not
  match our `CHA/ISC/BOL` + `TStack/ALoom` no-stdlib system. So the plan is to
  **port the algorithm's shape**, not copy the tree.

## 1. What PufferLib actually is (grounded, from the local fork)
Env contract — `src/pufferenv.h`:
- `typedef struct Agent { obs_t* observations; float* actions; float* rewards;
  float* terminals; unsigned char* action_mask; int policy; } Agent;`
- Backend hooks: `puf_init(Env*, Dict*)`, `puf_reset(Env*)`, `puf_step(Env*)`,
  `puf_render(Env*)`, `puf_close(Env*)`, `puf_log(Log*, Dict*)`;
  optional `puf_set_bot_policy(Env*, int)` for scripted opponents.
- `bf16` = `uint16_t` with `f32_to_bf16`/`bf16_to_f32` helpers (half-precision
  activations).
- Trainer — `src/algo.cu`: fused PPO kernel `PPOKernelArgs` with
  `grad_logits`, `grad_logstd` (continuous actions), `grad_values_pred`,
  `logits`, `actions`, `advantages`, `values_pred`, `action_mask`, `ent_coef`;
  value-clip (`mb_values` = "frozen rollout V (vf-clip)"); MuSE-transform
  decoder (`decoder_backward`, `grad_logits`/`grad_logstd`/`grad_value`);
  `NUM_ATNS` = number of action heads, `PPO_THREADS = 256`, `PPO_MAX_HEAD_A`.

Takeaways we copy: (a) a flat **vectorized** env with obs/actions/rewards/
terminals/action-mask arrays; (b) PPO with clipped policy loss + value-clip +
entropy bonus; (c) a transformer (MuSE-style) policy+value head; (d) bf16/
low-precision activations for throughput.

## 2. Target architecture (ASCIICrabs, C++23, no stdlib, console)
Everything in `namespace _` / `namespace CardsWorld`, Chimera+ naming,
seam-gated headers, single-TU build. New seams after `IGEEK_BLACKJACKENV (37)`:

| Seam | Value | Contents |
|---|---|---|
| `IGEEK_TENSOR` | 42 | `TTensor` (row-major float/isc buffer) + matmul/softmax/layer-norm |
| `IGEEK_POLICY` | 43 | `TTransformer` MuSE-style policy+value net (forward + backward) |
| `IGEEK_PPO` | 44 | `TPPO` update loop: rollout buffer, advantages (GAE), clipped loss |
| `IGEEK_GYM` | 45 | `TGym` (vectorized env batch) + `BlackjackGym` adapter |
| `CARDSWORLD_TRAIN` | 46 | test seam: overfit toy, reward-goes-up check |
| (existing) `CARDSWORLD_CORE` 40 / `CARDSWORLD_RELEASE` 41 | | unit tests / demo |

Layering (each depends only on the rows above it):
```
Card/CardStack/Deck/Hand/Player/Dealer/Blackjack   (done)
        |
BlackjackEnv  (done — single-agent step/observe/done)
        |
TGym / BlackjackGym  (vectorized: N parallel tables, obs/actions/rewards/terminals/mask)
        |
TTransformer policy  (MuSE-style: token embed -> L transformer blocks -> action
                      logits + value head; bf16 activations)
        |
TPPO  (rollout collect -> GAE -> K epochs of clipped PPO+value+entropy)
        |
CARDSWORLD_TRAIN seam  (verify learning happens on a toy target)
```

## 3. Component specs

### 3.1 `TTensor` (seam `IGEEK_TENSOR`)
- No `std`; a row-major `FPC*` (or `ISC*`) buffer with shape `{r, c}`.
  POD struct `TTensor` (Chimera+: T prefix, `data_`, `rows_`, `cols_`,
  `dtype_`); free fns `TTensorAlloc`, `TTensorFree`, `TTensorMatMul`,
  `TTensorSoftmax`, `TTensorLayerNorm`, `TTensorRelu`, `TTensorAdd`.
- Precision: store `FPC`; optionally a `bf16` (`IUB`) packed path later for
  throughput (mirrors PufferLib's `bf16`). Start `FPC` for correctness.
- **Determinism requirement (PufferLib values this):** all RNG through
  `_::Random` with a **clamped** index where it indexes (we already hit the
  half-normal `TRandom` bug); expose a seed setter so rollouts are reproducible.

### 3.2 `TTransformer` policy (seam `IGEEK_POLICY`)
- MuSE-style, small: token embedding (obs tokens -> dim D), L = 2–4 transformer
  blocks (MHSA + FFN + layernorm, residual), then two heads:
  - **action head**: logits over the discrete action set (blackjack = 2:
    hit/stand; keep `NUM_ATNS` general for combo games later).
  - **value head**: scalar V(s) for the GAE baseline.
- Forward: obs -> `logits[B, A]` + `values[B]` + (optionally) `logstd[B, A]`
  if we ever go continuous.
- Backward: analytic PPO gradients -> `grad_logits`, `grad_value` (the exact
  quantities PufferLib's fused kernel produces) -> backprop through the blocks.
- This is the biggest piece. Plan it as its own milestone with its own seam
  unit tests (forward shape checks, a known-input gradient check against a
  hand-computed case).

### 3.3 `TPPO` (seam `IGEEK_PPO`)
- Rollout buffer (PufferLib-shaped): `actions`, `rewards`, `terminals`,
  `logits` (old policy), `values`, `action_mask`, `obs` — fixed-size
  `TTensor`s, preallocated at init (PufferLib style: preallocate at init, free
  at close).
- **GAE** advantage estimate: `delta = r + gamma*V(s')*(1-done) - V(s)`,
  `adv_t = sum lambda^k delta`.
- Update: K epochs, minibatches; loss =
  `-min(ratio*adv, clip(ratio,1-eps,1+eps)*adv)  +  vf_coef*value_loss  -
  ent_coef*entropy`, with `ratio = exp(new_logp - old_logp)` and PufferLib's
  **value-clip** (`vf-clip`) on the value loss.
- Output: updated `TTransformer` weights + a `TPPOStats` (policy_loss,
  value_loss, entropy, approx_kl, explained_var) for logging.

### 3.4 `TGym` + `BlackjackGym` (seam `IGEEK_GYM`)
- `TGym` = vectorized env batch (the `Env*`/`Agent*` analog): holds `N`
  parallel game states and the flat arrays `observations[B*obs_len]`,
  `actions[B]`, `rewards[B]`, `terminals[B]`, `action_mask[B*A]`.
  Methods: `GymInit(N)`, `GymReset()`, `GymStep(actions[B])`, `GymClose()`,
  `GymLog()`. This is the direct analog of PufferLib's `puf_*` contract.
- `BlackjackGym` adapts the existing `Blackjack`/`BlackjackEnv` to `TGym`:
  - **Observation encoding** (critical design decision — see §4): the current
    `Observe()` returns a *text* percept (`"P:15 D:5 H:1 R:0\nKs 5d"`). For a
    neural policy we need a **numeric token/vector** obs. Encode per-round:
    player hand value (0..21 -> token/float), dealer up-card value (0..10),
    hidden-card count, round-over flag, and (optionally) each card as a
    (rank, suit) token pair. Keep it fixed-length `obs_len`.
  - Reuse the existing engine; do **not** rewrite blackjack rules here.

### 3.5 Migrate the stale iGeek `Env`/`Gym`/`Multiverse` (optional, later)
`~/AStarStarship/iGeek/Env.h` is still `class Env : public Room` with `STA*`
(dead type) — the same breakage we fixed in the card game. **Do not block the
gym on this.** Build `TGym` standalone in iGeekCardsWorld first (like
`BlackjackEnv` is standalone). Only later, if we want the shared framework,
port `Env/Gym/EnvGoal/EnvReward` to `TRoom<...>` + `CHA*` and have
`BlackjackGym` conform to it. Track as a separate work item, not a dependency.

## 4. Open decisions (need the Captain's call before Milestone 2+)
1. **Action space**: start **discrete** (hit/stand, A=2) — matches the current
   engine and is the tractable start. PufferLib's `NUM_ATNS` head is general, so
   we keep the door open to combo/continuous actions later. *Recommendation:
   discrete first.*
2. **Observation encoding**: (a) fixed scalar vector (hand value, up-card,
   hidden count, done) — simplest, works now; vs (b) per-card token sequence
   fed to the transformer — more faithful to a MuSE-transform policy but more
   work. *Recommendation: (a) for Milestone 1–2, add (b) when we want
   sequence-aware play.*
3. **Precision**: `FPC` everywhere for correctness first; add the `bf16` packed
   path later if throughput matters (we're console, not chasing SPS yet).
4. **Scope of "dunk on him with MuSE Transform"**: are we implementing a *real*
   MuSE (multi-head, gated) transformer, or a small standard transformer with
   the same forward/gradient structure? A real MuSE-port is a large lift.
   *Recommendation: standard small transformer first (proves the PPO loop
   learns), then swap in MuSE-block internals as an optimization.*
5. **CPU only** (this VM has no GPU — do not assume CUDA). The PufferLib CUDA
   kernels are reference-only; we write CPU FPC kernels. Confirm we are not
   expecting GPU training on this box.

## 5. Milestones (each ends in a green seam + committed)
- **M0 — done.** `BlackjackEnv` builds/runs; `CARDSWORLD_CORE` unit tests pass;
  `_Seams/` restructured; AGENTS.md documents the seam test API.
- **M1 — `TTensor` + seam tests.** Implement the POD tensor ops; add
  `02.Tensor.hxx` unit tests (matmul/softmax/layernorm vs hand-computed values).
  Gate: `CARDSWORLD_CORE`-adjacent seam green.
- **M2 — `BlackjackGym` (vectorized) + numeric obs.** Add `TGym`/`BlackjackGym`,
  encode obs numerically, add seam tests (N=8 batch steps, reward/terminal
  invariants, obs fixed-length).
- **M3 — `TTransformer` forward.** Small policy net; seam test: fixed input ->
  expected logits/value shapes and a deterministic output (seeded).
- **M4 — `TTransformer` backward + gradcheck.** Analytic grads; seam test:
  numeric gradcheck (finite-diff vs analytic) on a tiny net — this is the
  load-bearing correctness gate before any training.
- **M5 — `TPPO` loop.** GAE + clipped PPO+value+entropy on the rollout buffer;
  seam test: on a **toy** env (known optimum) reward increases over N updates,
  and approx_kl stays bounded.
- **M6 — end-to-end train.** `CARDSWORLD_TRAIN` seam: train the policy on
  `BlackjackGym` for K steps, assert the learned policy's win rate > the
  hit-to-17 baseline (or reward strictly increases). Wire into `_Main.cpp`
  release seam as a `--train` mode.
- **M7 — (optional) real MuSE blocks + bf16 + iGeek framework conformance.**

## 6. Testing (use the ASCIICrabs seam API — NOT CTest)
Per AGENTS.md: tests are seam-gated `.hxx` units in `_Seams/`, using the
`CORE_CHECK/CORE_EQ/CORE_PTR` fail-tracking pattern (so a failure sets a
non-zero exit code). New units:
- `02.Tensor.hxx` (seam `IGEEK_TENSOR`)
- `03.Gym.hxx` (seam `IGEEK_GYM`)
- `04.Policy.hxx` (seam `IGEEK_POLICY`) — forward determinism + gradcheck
- `05.PPO.hxx` (seam `IGEEK_PPO`) — toy-env reward-goes-up
- `06.Train.hxx` (seam `CARDSWORLD_TRAIN`) — end-to-end learning check
Aggregate them in `_Tests.hxx` (extend `TTestTree<CWTest::Core, ...>`).

## 7. Risks / honest unknowns
- **The transformer backward pass is the highest-risk piece.** A subtle grad
  bug will look like "training doesn't converge." Mitigated by the M4
  finite-difference gradcheck gate — do not skip it.
- **Card obs are sparse/short** — a transformer may be overkill for
  hit/stand; the scalar-vector obs (M2) may learn faster. The MuSE-transform
  "dunk" is most justified once obs are per-card sequences.
- **Single-deck blackjack is nearly intractable to beat with basic policy** —
  the baseline is already near-optimal, so "reward goes up" may be a small
  delta. The toy-env check (M5) isolates *whether the learner works* from
  *whether blackjack is a hard target*.
- **Determinism/RNG**: we already bit the `TRandom` half-normal bug; all
  training RNG must be seeded + clamped or rollouts won't reproduce.
- **No GPU on this VM.** All M1–M6 are CPU FPC. If GPU training is the actual
  goal, that's a separate hardware/VM task — say so and I'll re-plan the
  kernels for CUDA.
- **Do not vendor the PufferLib tree.** It is CUDA/C99/raylib/cJSON and
  incompatible with the no-stdlib ASCIICrabs system; only its *algorithm shape*
  is reusable. Copying `src/`/`vendor/` in would break the single-TU no-stdlib
  build.

## 8. Immediate next step (after Captain approves §4 decisions)
Start M1: scaffold `TTensor.h/.hxx` + `_Seams/02.Tensor.hxx` + the seam wiring in
`_Seams.h`/`_Config.h`/`_Tests.hxx`, land it green, commit. Then M2.
