# [iGeek Cookbook](../README.md): CarWorld

CarWorld is the Interactive Gym Environment and Educational Kit's native,
headless 2D driving environment. It combines a four-contact force-based car,
TileWorld terrain/collision queries, route planning, a classical reference
autopilot, Gym/PPO interfaces, and a Chinese Room application binding.

This chapter connects the implementation to the University of Toronto
Self-Driving Cars Specialization. It is an engineering guide, not a claim
that CarWorld implements every course component or is safe for real vehicles.

## Read in this order

1. [Build and run](#build-and-run).
2. [API and Chinese Room commands](./Api.md): state ownership, typed Floor
   schema, manual/autopilot takeover, replies, Gym observations and actions.
3. [Vehicle physics](./Physics.md): frames, units, equations, contact forces,
   actuation, numerical integration, defaults and model limits.
4. [Control and planning](./ControlAndPlanning.md): feedforward/feedback,
   pure pursuit, stopping envelopes, grid routing and missing advanced control.
5. [Engineering and verification](./Engineering.md): architecture boundary,
   reproducibility, test coverage, failure modes, telemetry and safety scope.
6. [Specialization learning map](./CourseGuide.md): all four course domains,
   their relationship to CarWorld, references and future exercises.

## Build and run

Run commands from the iGeek repository root. Keep ASCIICrabs and CrabsTK as
sibling repositories. The native build uses C++23 and CMake 3.16 or newer;
the current verified compiler is GCC 14.2.0 on Linux.

```sh
cmake -S CarWorld -B CarWorld/build -DCMAKE_BUILD_TYPE=Release
cmake --build CarWorld/build -j4
ctest --test-dir CarWorld/build --output-on-failure
CarWorld/build/carworld --scenario all --episodes 1 --seed 7
CarWorld/build/carworld_room_tests
```

The classical-driver run should finish with `TOTAL goals=9/9`. The native
Room test should print `CAR_ROOM takeover_routes=9/9` and `CAR_ROOM failures=0`.
Wall-clock timing varies with the host; it is not a fixed acceptance value.

To run sanitizer checks:

```sh
cmake -S CarWorld -B CarWorld/build/sanitized \
  -DCMAKE_BUILD_TYPE=Debug -DCARWORLD_SANITIZE=ON
cmake --build CarWorld/build/sanitized -j4
ASAN_OPTIONS=detect_leaks=1 ctest --test-dir CarWorld/build/sanitized \
  --output-on-failure
```

CTest currently registers physics/routes, PPO interfaces, Room binding, CLI,
and evidence-validation checks. Python-dependent checks require a Python
interpreter; Python is test tooling, not a simulation dependency.

## Executables and current scope

| Executable | Purpose |
|---|---|
| `carworld` | Seeded scenarios, classical speed actions, CSV metrics and traces |
| `carworld_tests` | Native physics, routes, rejection cases and Gym contracts |
| `carworld_ppo_smoke` | Finite PPO updates for driver and raw-actuator action spaces |
| `carworld_room_tests` | Typed Room dispatch, schema rejection and manual/autopilot takeover |

The Room adapter is exercised by its own test executable. The existing
`carworld` CLI still calls the environment directly; it has no keyboard,
gamepad, Room RPC or interactive video-game frontend yet. The old Qt/maze
files are not the native build and are not proof of a working dashboard.

The implementation avoids C++ standard containers in the simulation path,
but the existing CLI uses C math/POSIX timing/C file I/O, and ASCIICrabs COut
currently uses iostream internally. Do not interpret this as a freestanding,
standard-runtime-free executable. No GPU or MKL backend is needed for the
current scalar vehicle physics.

## Source map

- [CarPhysics.h](../../CarWorld/CarPhysics.h) and
  [CarPhysics.hxx](../../CarWorld/CarPhysics.hxx): plant parameters/state and RK4.
- [CarPilot.hxx](../../CarWorld/CarPilot.hxx): classical control and preview.
- [CarWorld2D.h](../../CarWorld/CarWorld2D.h) and
  [CarWorld2D.hxx](../../CarWorld/CarWorld2D.hxx): environment and observations.
- [CarGym.h](../../CarWorld/CarGym.h): batched Gym interface.
- [CarRoom.h](../../CarWorld/CarRoom.h) and
  [CarRoom.hxx](../../CarWorld/CarRoom.hxx): native Chinese Room binding.
- [CarScenarios.hxx](../../CarWorld/CarScenarios.hxx): nine deterministic layouts.
- [VehicleMap.hxx](../../BikeWorld/VehicleMap.hxx) and
  [VehicleRoute.hxx](../../BikeWorld/VehicleRoute.hxx): shared Bike/Car terrain,
  swept collision, guidance and planning.
- [Existing detailed headless notes](../../CarWorld/Headless.md): earlier
  model/evidence documentation; historical measurements are not new benchmarks.

## License

Copyright [AStarship](https://astarship.net). See the repository
[LICENSE.md](../../LICENSE.md), AStartup Strong Source-available License.
Course materials and third-party references retain their own rights and terms.
