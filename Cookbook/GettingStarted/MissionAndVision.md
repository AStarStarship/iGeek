# [iGeek Cookbook](../)

## [Getting Started](./)

### Mission and Vision

The mission of iGeek is to be a data-driven RL AI gym for building,
measuring, and learning agents in massively parallel fuzzed environments —
and to use that gym as a research instrument for studying how internal agent
state changes under learning (the qualia measurement protocol: behavior
metrics + internal-state snapshots + linear probes).

The vision is an engine that makes that study reproducible and fast:

- **Headless and parallel.** Every world is a deterministic simulation that
  runs on the console. No GPU, no display, no network. BikeWorld simulates
  254 simulated seconds of driving in ~0.06 seconds of wall time, which is
  what makes large-scale fuzzing and long training runs practical on a
  single workstation.
- **No standard library.** The engine is C++23 on a fixed-width type system
  (CHA/ISC/IUD/BOL/FPC). No `<vector>`, no exceptions on the hot path. The
  cost is boilerplate; the payoff is a codebase that is easy to audit, easy
  to port, and honest about what it allocates.
- **One gym contract.** An `Env` produces observations and rewards; a `Gym`
  batches N envs; a `TPolicy` maps observations to action logits and values;
  a `TPPO` loop trains the policy over the batch. The same four pieces work
  for cards, cars, bikes, and mazes.
- **Measurable agents.** The agent framework ships three generic tiers
  (table / reflex / model) plus a metrics layer (behavior tallies,
  state-snapshot records, linear probe), so the question "did a
  representation form?" has a defined experimental answer.

The Cookbook is the trail: each world chapter builds one environment, runs
its driver or learner, and shows the evidence.

**[<< Previous Section: Configuration and Build System](./ConfigurationAndBuildSystem.md) | [Next Section: iGeek Cookbook >>](../)**

## License

Copyright [AStarship™](https://astarship.net).
