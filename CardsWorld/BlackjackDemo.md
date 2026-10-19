# Headless contiguous-batch Blackjack RL demo

## Scope and choice

CardsWorld already builds against the current Crabs single-TU package and has
real Blackjack rules. MazeWorld is still the UTF-16/Script2/Windows legacy tree
(no modern headless implementation); CarWorld's entrypoint includes the missing
legacy toolkit server. Neither was ported for this small demo.

This is **one-decision Blackjack**, a contextual-bandit slice of the existing
BlackjackEnv, not a complete multi-hit Blackjack learner. Start from a fresh,
uniformly shuffled 52-card deck, reject rounds already decided by a natural,
choose hit or stand once, and force stand after a surviving hit. The existing
engine handles aces, dealer draws, busts, comparisons, and settlement. Rewards
are the unmodified engine values: win +2, lose/bust -2, push 0. Conditioning
away naturals means the +3 payout does not occur here.

TBlackjackDemoGym derives from existing Gym and owns fixed contiguous observation,
action, reward and terminal arrays for **8 environments**. Observations are 30
one-hot features: player total 4..21, dealer up-card ace..10, and hard/soft hand.
No dealer hole-card, future draw, or reward labels enter the observation. It
wraps eight existing BlackjackEnv objects; their card pointers and engine
allocations are unchanged. This is not a fully serialized, pointer-free Crabs
blob. Existing TPPO also still allocates scratch tensors during rollouts.

The adapter supplies the missing Gym-compatible seeded/autoreset interface:
existing BlackjackGym is standalone, not a Gym subclass, has no seeded reset,
and leaves completed tables done. No existing engine files were modified.
The seeded Fisher-Yates shuffle uses rejection sampling over a local LCG instead
of the legacy half-normal random helper. Each next deck is independent of the
chosen action, allowing common-deal comparisons across evaluation policies.

## Compile and run

```sh
cd iGeek/CardsWorld   # from the AStarship workspace root
cmake -S . -B build/rl_demo/cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/rl_demo/cmake --target blackjack_demo blackjack_demo_tests -j2
build/rl_demo/cmake/blackjack_demo_tests
mkdir -p build/blackjack_demo
build/rl_demo/cmake/blackjack_demo > \
  build/blackjack_demo/learning_curve.csv
```

Direct compiler equivalent:

```sh
g++ -std=c++23 -O2 -I_Seams -I. -I../../ASCIICrabs \
  blackjack_demo.cpp -o build/rl_demo/blackjack_demo
build/rl_demo/blackjack_demo > build/rl_demo/results.csv
```

Sanitizer gate (both seam tests and the complete training/evaluation executable):

```sh
for source in demo_tests blackjack_demo; do
  g++ -std=c++23 -O1 -g -fsanitize=address,undefined \
    -fno-omit-frame-pointer -fno-sanitize-recover=all \
    -I_Seams -I. -I../../ASCIICrabs \
    "$source.cpp" -o "build/rl_demo/${source}_san"
done
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 \
  build/rl_demo/demo_tests_san
ASAN_OPTIONS=detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1 \
  build/rl_demo/blackjack_demo_san > build/rl_demo/results_san.csv
cmp build/rl_demo/results.csv build/rl_demo/results_san.csv
```

## Frozen protocol and actual results

Three policy seeds 777/778/779; training deck seeds 12345 + policy seed.
TLinearPolicy is built explicitly with its seed (avoids its default lazy-build
seed). Learning rate 0.1, horizon 32, **one update epoch**, 1000 TrainSteps,
256000 training episodes per run. All transitions are terminal. Training is
existing TPPO::TrainStep and TLinearPolicy::SgdStep, not a replacement trainer.

Validation seed 700001: 8192 deals every 100 updates. Final test seed 1000001:
100000 deals for each before/after greedy policy, uniform-random policy, and
hit-below-17 heuristic. The fixed training length is not selected from validation
or test scores; all three seeds are reported, not just the best seed. Random
actions use a separate RNG so they do not perturb the deck stream. Evaluation
calls Forward only and never changes weights or policy sampling state. The
same initial deck sequence is replayed for every comparison. Seeds are held
out, not a split of observation types; card hands naturally recur.

Actual final test mean settlement reward (higher is better):

| Policy seed | Before | After | Random | Hit-below-17 |
|---|---:|---:|---:|---:|
| 777 | -0.33336 | -0.21848 | -0.55446 | -0.19580 |
| 778 | -0.45612 | -0.38830 | -0.55446 | -0.19580 |
| 779 | -0.39328 | -0.34066 | -0.55446 | -0.19580 |

Across these three runs: before -0.39425333, after -0.31581333, improvement
+0.07844. All improve and beat random, but **none beats the heuristic**.
That heuristic is not an oracle and is restricted to the same single decision.
CSV includes the ordinary episode sample standard error (about 0.006 per final
score); it is not a paired-comparison confidence interval or uncertainty across
training seeds. Seed 777's validation reward rises from -0.38818 at update 100
to -0.20532 at update 1000; the full curves include the weaker other runs.

The complete three-seed executable took **1.76 s** in the Release CMake build,
and **13.71 s** with ASan+UBSan, including evaluation. The release, direct -O2,
and sanitizer CSV outputs were byte-identical (42 data rows).

Artifacts:
- `build/blackjack_demo/learning_curve.csv`
- `build/blackjack_demo/learning_curve.svg`
- Executable: `/home/astarcale/AStarStarship/iGeek/CardsWorld/build/rl_demo/cmake/blackjack_demo`
- Test/sanitizer logs and binaries: `/home/astarcale/AStarStarship/iGeek/CardsWorld/build/rl_demo/`

CSV SHA256: `7a4a92071f3841202b48625219edec65958299dc61d1a5243ce2b999317cd603`.

## Tests and limitations

New behavior followed RED -> GREEN vertically. Recorded failing seam returns:
`seeded_playable_observation_missing`, `one_decision_settlement_missing`,
`heldout_evaluation_missing`, `training_improvement_missing`. Each was observed
with exit 1 before its implementation, then passed. The driver first emitted
nothing and failed the 42-row/kind acceptance check before CSV wiring. Logs are
`build/rl_demo/tdd_red_*.log` / `tdd_green_*.log` (driver red output is CSV).
The seam returns non-nil on failure, not print-only A_ASSERT. Tests exercise
seed replay/distinction, eight-row settlement versus direct existing Env calls,
autoreset replay over 64 steps, evaluation moments/replay, and held-out learning.
New tests and complete demo both pass strict ASan+UBSan with leak detection.

The original CardsWorld full seam suite passes at -O2 (compile `_Seams/_Main.cpp`
with `-DSEAM_N=41`). **Its strict UBSan run fails** in the pre-existing
`ASCIICrabs/Puff.hpp:110` misaligned IUB store while printing the float gradcheck
result. This is outside the demo path; it was not suppressed or patched. Existing
package compiler warnings remain. No shared engine or Crabs files were edited.

Algorithm inspection also found existing TPPO limitations: UpdatePolicy uses
stored rather than recomputed logits; TLinearPolicy::Backward ignores old_logp,
clipping and entropy; return targets are reconstructed from normalized
advantages; multi-step GAE reads the next row's terminal rather than the current
transition. Thus this is an **existing TPPO-path policy-gradient demo**, not a
validation of a complete clipped PPO implementation. One epoch and all-terminal
transitions avoid repeated stale-logit updates and multi-step terminal leakage,
but do not establish correct value targets or full PPO. Fixing those requires
separate engine TDD, and was not indispensable to this bounded demo.

No transformer backprop, full Blackjack strategy, physics, token geometry,
consciousness by this experiment alone. No commits
or pushes were made; pre-existing untracked PrisonerWorld work was preserved.
