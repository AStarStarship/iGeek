# [iGeek Cookbook](../)

## CarWorld

CarWorld is an environment for building, measuring, and learning
self-driving car AI agents.

A continuous 2D car plant with four friction-limited tire contacts, route
planning, and a classical driver as the reference baseline, plus the
Gym/PPO wiring for the learning path.

## Build and run

```Bash
cmake -S CarWorld -B CarWorld/build -DCMAKE_BUILD_TYPE=Release
cmake --build CarWorld/build -j
./CarWorld/build/carworld
```

Expected output (end of run): a goal tally (`TOTAL goals=9/9`), frame and
physics-step counts, and the simulated/wall time split.

Exit code 0 = the driver completed all goals.

## Layout

- Plant model — state, inputs, 4-tire friction contacts.
- Driver — the classical reference controller.
- Route planning — the planned-path generation the driver follows.
- Gym wiring — the env exposed to the RL core (PPO smoke).
- `_Seams/` — the seam test units (the build's exit code is the test result).

## License

Copyright [AStarship™](https://astarship.net).
