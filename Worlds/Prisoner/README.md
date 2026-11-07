# PrisonerWorld

Status: **simulation-only** — repeated Prisoner's Dilemma with
hand-coded strategies (Tit-for-Tat, Grim Trigger, Random, etc.).
Not a Gym subclass; no PPO loop. Strategies are analytical, not learned.

Repeated Prisoner's Dilemma simulation for iGeek.

## Building

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 4
```

## Running

```sh
PrisonerWorld/build/PrisonerWorld --episodes 100 --payoffs 5 3 1 0
```

## Controls

- `--episodes N`: Number of episodes to run (default: 100)
- `--payoffs T R P S`: Payoff matrix values (default: 5 3 1 0, where T>R>P>S)
- `--seed S`: Random seed (default: 7)

## Design

Two agents simultaneously choose Cooperate (0) or Defect (1) each round.
Payoffs:
- Both Cooperate: both get R
- Both Defect: both get P
- I Cooperate, you Defect: I get S (sucker), you get T (temptation)
- I Defect, you Cooperate: I get T, you get S

The `ComputeReward` function in `PrisonerWorld.h` compares achieved vs desired
goals and returns the scalar FPC reward per agent per step.