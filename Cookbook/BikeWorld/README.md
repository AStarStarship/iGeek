# BikeWorld

A 2D bike autopilot iGeek gym environment.

A continuous planar bicycle plant with a 4-tire friction model, driven
headless by a classical reference driver. It is the fastest end-to-end
example in the Cookbook: build it, run it, and read the evidence.

## Build and run

```Bash
cmake -S BikeWorld -B BikeWorld/build -DCMAKE_BUILD_TYPE=Release
cmake --build BikeWorld/build -j
./BikeWorld/build/bikeworld
```

Expected output (end of run): a goal tally (`TOTAL goals=8/8`), frame and
physics-step counts, and the simulated/wall time split — the wall time is
the headless speed evidence (254 simulated seconds in ~0.06 s wall).

Exit code 0 = the driver completed all goals.

## Layout

- Plant model — state, inputs, the 4-tire contact model.
- Driver — the classical (non-learning) reference controller.
- Gym wiring — the env exposed to the RL core (PPO smoke).
- `_Seams/` — the seam test units (the build's exit code is the test result).

## License

Copyright [AStarship™](https://astarship.net).
