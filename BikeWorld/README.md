# BikeWorld

Status: **tested headless** — standalone headless build/test entry point
passes. Not validated as a learned policy; the autopilot is classical
(speed commands), not end-to-end learned steering.

Native headless bicycle simulator and classical autopilot, built on iGeek and
TileWorld. No LLM inference, Python runtime, downloaded assets, SFML, CUDA,
external physics engine, or C++ standard-library containers are required by
the simulator. It uses Crabs types, compiler `new[]`/`delete[]`, and C math/I/O.
The normal GCC C++ runtime is still linked. Python is optional CLI test tooling.

## Build and run

From the iGeek repository:

```sh
cmake -S BikeWorld -B BikeWorld/build -DCMAKE_BUILD_TYPE=Release
cmake --build BikeWorld/build -j 4
ctest --test-dir BikeWorld/build --output-on-failure
BikeWorld/build/bikeworld --scenario all --episodes 100 --seed 7
```

Release uses `-O2`. Run one instrumented obstacle ride:

```sh
BikeWorld/build/bikeworld --scenario obstacle --seed 7 \
  --csv BikeWorld/build/my_obstacle_metrics.csv \
  --trace BikeWorld/build/my_obstacle_trace.csv
```

Output files are exclusive-created; existing evidence is never overwritten.
A trace requires one scenario and one episode. `--help` lists the flags.
Exit codes: 0 = every ride reached its goal, 1 = a failed ride, 2 = CLI/I/O error.
`--unassisted` disables the pilot, without changing the physics; falls then
return exit 1. It is a negative control, not a successful autopilot run.

Sanitizer build:

```sh
cmake -S BikeWorld -B BikeWorld/build/sanitized \
  -DCMAKE_BUILD_TYPE=Debug -DBIKEWORLD_SANITIZE=ON
cmake --build BikeWorld/build/sanitized -j 4
ASAN_OPTIONS=detect_leaks=1 ctest --test-dir BikeWorld/build/sanitized \
  --output-on-failure
```

## Tiles and coordinates

The supplied fixtures use TileWorld's existing `TileMap`: 120 columns by
64 rows, one metre per tile. The rider's continuous FP64 pose is in metres and
radians; the physics does not teleport between cells. Positive yaw/roll turn
and lean toward positive world y from a heading of zero.

A tile has two independent meanings:

| Material | Friction coefficient | Rolling coefficient | Grade |
|---|---:|---:|---|
| Empty/asphalt | 0.85 | 0.006 | 0 |
| Gravel | 0.45 | 0.025 | 0 |
| Ice | 0.10 | 0.004 | 0 |
| Grass | 0.40 | 0.050 | 0 |
| Uphill | 0.85 | 0.006 | dz/dx = +0.05 |
| Downhill | 0.85 | 0.006 | dz/dx = -0.05 |

These are configurable modeling assumptions, not measured material constants.
Material labels set tire contact properties; only `Tile::GetCollision()` makes
a cell solid. Unknown material types default to asphalt. World boundaries are
solid. Front and rear contacts sample their own cells each physics step.
Grades are force fields here, not a rendered 3D height mesh.

`TVehicleSweep` performs continuous slab intersections against expanded tile
AABBs. A square centre envelope of radius 0.8 m conservatively covers the
bike/rider; corner contacts are conservative, not a rotating chassis shape.
Collisions halt and terminate at first contact, rather than tunneling through
or silently snapping the bicycle back to a valid route. Three ideal range
rays supply observation features; braking uses the inflated envelope instead.

## Physics and control

`BikePhysics.h/.hxx` implement a forward-only dynamic single-track model:

- rider+bike mass, front/rear axle arms, CG height, yaw and roll inertia;
- finite cornering stiffness with axle Coulomb friction-circle saturation;
- pedal-force and power limits, braking, aerodynamic drag, rolling resistance,
  and longitudinal/lateral gravity components on grades;
- lateral velocity, yaw rate, unstable gravity-driven roll, and roll damping;
- steering angle, rate, and first-order actuator limits.

The roll torque is `m*h*(g_normal*sin(roll) - ay*cos(roll))`.
There is no hidden high-speed balance correction in the plant. The pilot
stabilizes roll only by countersteering through the same bounded actuator
available to a raw policy. Invalid/nonfinite inputs fail without advancing the
physics state. A roll of 0.70 rad terminates as a fall.

A control frame is 0.02 simulated seconds (50 Hz), containing four 0.005 s
FP64 RK4 physics steps (200 Hz). These rates are simulation time, not wall-time
waits. There is no sleeping or rendering in the loop.

`ABikePilot` combines pure-pursuit route guidance, smoothed target lean,
roll/roll-rate feedback with front-tire force inversion, speed feedback with
resistance feed-forward, friction/curvature speed limits, and obstacle braking.
The nominal cruise speed is 3.5 m/s. The requested braking gap is 1.5 m beyond
the conservative body envelope; actual gap is measured, not asserted equal to
the request.

Launch and halt use explicit rider support: `foot_down` is effective only below
0.45 m/s and below 0.35 rad lean. While supported, roll and roll rate are reset
to zero. This is a declared low-speed rider/stand approximation, not learned
track-standing. At speed, the plant falls when steering cannot stabilize it.

`AVehiclePlanner` uses deterministic four-neighbour A* on TileWorld, checks
clearance and segment sweeps, then removes unnecessary waypoints with
line-of-sight tests. Workspace is fixed: at most 8192 map cells and 256 output
waypoints; invalid/unreachable/over-capacity inputs fail with an empty route.
The fixtures plan with 5 m clearance to accommodate pure-pursuit corner
cutting. This is not a curvature-constrained planner: narrow corridors and
sharp turns are not guaranteed feasible. Planning is not repeated each tick.
A newly introduced obstruction makes the pilot stop; automatic replanning
around moving obstacles is not implemented.

## iGeek interfaces and CarWorld reuse

- `ABikeEnv::Reset()` replays its stored seed; `ResetSeed()` changes it.
  Seeds perturb initial roll and heading, not the terrain layout.
- `ABikeEnv::Step()` applies a speed command through the classical pilot.
  Actions are cruise, slow, fast, stop, and crawl (3.5, 2.5, 4.5, 0, 1.5 m/s).
- `ABikeEnv::StepControl()` accepts raw steering/pedals/brake/foot support,
  bypassing the pilot entirely. Use this seam for future learned balance.
- `ABikeEnv::Obs()` exposes 21 normalized FP32 features: body velocities,
  yaw/roll/steering state, heading, local goal, route error/curvature,
  contact friction/grade, three ranges, support, and episode progress.
- `ABikeGym` really derives from iGeek `Gym`, with 1..8 independent states,
  flat observation/action/reward/terminal/mask buffers, and seeded terminal
  auto-reset. Terminal reward/flag are retained; the next initial observation
  is returned. Map and route ownership remain with the caller.
- `bikeworld_ppo_smoke` exercises existing `TLinearPolicy` and `TPPO` with
  eight bikes, horizon 32, two epochs, and ten updates. It verifies finite
  statistics and interface compatibility only. It does NOT train or establish
  a learned bicycle autopilot. Existing `TPPO::CollectRollout()` resets the gym
  for every rollout; a short horizon does not cover a complete ride.

`VehicleMap.hxx` supplies terrain, swept contact, and ranges;
`VehicleRoute.h/.hxx` supply clearance-aware planning and route guidance.
These are the shared CarWorld foundations. Reuse them and the Gym/trace
contracts, then replace bike roll dynamics/control with an appropriate car
plant and footprint. CarWorld's four-wheel dynamics and driver are not built
by this change.

## Measured verification

CPU x86-64, GCC 13.3, `-O2`; seeds 7..106, 100 rides per fixed fixture:

| Fixture | Goals | Mean sim seconds | Peak absolute roll degrees | Max cross-track metres |
|---|---:|---:|---:|---:|
| Straight | 100/100 | 30.585 | 2.345 | 0.431 |
| Curves | 100/100 | 32.960 | 12.315 | 0.425 |
| Obstacle detour | 100/100 | 34.929 | 15.332 | 1.469 |
| Gravel strip | 100/100 | 30.585 | 2.345 | 0.431 |
| Ice strip | 100/100 | 32.160 | 2.345 | 0.430 |
| Uphill strip | 100/100 | 30.582 | 2.345 | 0.431 |
| Downhill strip | 100/100 | 30.588 | 2.345 | 0.431 |
| Launch from rest | 100/100 | 31.680 | 1.717 | 0.430 |

800/800 goals; zero failed rides. Every finish had speed below 0.1 m/s and
actual goal error below 1.2 m; the worst measured goal error was 0.458363325 m.
This samples small initial-pose perturbations on eight layouts, not arbitrary
roads, winds, sensor errors, or a broad physical parameter distribution.
Cross-track error includes endpoint overshoot. Roll peaks include the initial
pose and are sampled at the 200 Hz physics endpoints.

The same 800-ride run without CSV/trace produced 1,270,342 control frames,
5,081,368 physics steps, and 25,406.840 simulated seconds in 5.131826 wall
seconds: 247,541.9 control frames/s and 4,950.8x real time, on one thread.
This includes fixture setup and planning. It is simulator throughput, not
neural training throughput or a GPU benchmark.

Verification artifacts in the ignored build directory:

- `verified_800.csv`: exact per-ride metrics for all 800 rides;
- `verification_summary.json`: parsed counts, per-fixture statistics and trace checks;
- `obstacle_trace.csv`: 1747 rows, including initial state, for seed 7;
- `obstacle_single.csv`: its per-ride metrics;
- `unassisted.csv`: the same straight fixture without pilot, falling at 1.645 s.

Release and ASan/UBSan/float-cast-overflow builds pass all three CTest entries.
Tests also cover unchanged-state input rejection, friction-limited braking,
RK4 half-step convergence, unassisted falls, route obstruction, invalid/empty
routes, goal stopping, emergency stopping, no-tunneling collision, seeded
replay, batch terminal semantics, CLI parsing, and output overwrite refusal.
The emergency-stop fixture retains 0.570 m clearance beyond the body envelope.
A separate 0.04 rad unassisted lean test falls at 1.145 s. Braking from 5 m/s
for one second yields 0 m/s on asphalt versus 3.896 m/s on ice in this model.
Existing PacWorld linear/transformer PPO verification also still passes.

## Limits before physical deployment

This is a force-based reduced-order simulator, not a validated multibody
bicycle model or a safe hardware autopilot. Static axle loads omit longitudinal
load transfer; wheel spin/gyroscopic effects, steering-axis trail/caster,
rider articulation, suspension, pitch/heave, banked geometry, impacts,
reverse travel, traffic, and sensor noise/latency are absent. The low-speed
slip-angle chart is regularized and cannot model general standstill dynamics.
The pilot knows the ideal plant state, contact material, and planned route.

Next physical validation should identify tire/actuator parameters, compare
measured trajectories or a full bicycle benchmark, and stress combined
curves/low-friction/crosswind/load-transfer cases. These fixtures establish
working native plumbing and deterministic basic behavior, not deployment
readiness. GPU inference/training backends are not added; the CPU simulator
runs without a working NVIDIA driver, including on the planned GPU host.

## Existing core compatibility edits

- `ASCIICrabs/_ConfigHeader.h`: using-declarations expose existing global
  primitive types in `_` for TileWorld's qualified API; no new types or ABI.
- `TileWorld/ComponentHitbox.h`: its cached next-position rectangle is mutable
  so the existing const query compiles.
- `TileWorld/TileMap.h`: additive read-only `GridSize`, `TileAt`, `CellBlocked`.

The remaining changes are opt-in files in `BikeWorld/`. No CrabsTK change was
needed. Existing unrelated work is preserved; no commit or push was made.
