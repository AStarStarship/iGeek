# CarWorld: native headless 2D

Copyright AStarship <https://astarship.net>.

This is an opt-in, continuous, force-based car simulator and classical reference
controller. It reuses iGeek, TileWorld, and ASCIICrabs. The older maze/Qt dashboard
files are preserved; this build does not depend on them. No LLM, downloaded
assets, CUDA, external physics engine, SFML, Python runtime, or C++ standard
containers are required to run the simulator. C math/I/O and compiler
`new[]`/`delete[]` are used; the normal GCC C++ runtime is still linked.
Python is optional test/verification tooling.

## Build and execute

From `/home/astarcale/AStarStarship/iGeek`:

```sh
cmake -S CarWorld -B CarWorld/build -DCMAKE_BUILD_TYPE=Release
cmake --build CarWorld/build -j 4
ctest --test-dir CarWorld/build --output-on-failure
CarWorld/build/carworld --scenario all --episodes 100 --seed 7
```

Executable: `/home/astarcale/AStarStarship/iGeek/CarWorld/build/carworld`.
Use `--help` for the complete CLI. Examples, using fresh output paths:

```sh
CarWorld/build/carworld --scenario obstacle --seed 7 \
  --csv CarWorld/build/my_obstacle.csv --trace CarWorld/build/my_obstacle_trace.csv
CarWorld/build/carworld --scenario straight --barrier \
  --csv CarWorld/build/my_emergency.csv
CarWorld/build/carworld --scenario obstacle --open-loop \
  --csv CarWorld/build/my_open_loop.csv
```

The last two are negative controls and return 1: blocked and collision,
respectively. Exit codes are 0 = every drive reached its goal, 1 = at least one
non-goal episode, 2 = CLI/I/O error. CSV and trace files are exclusive-created;
existing evidence is not overwritten. Trace requires one scenario and episode.
`--action` selects cruise/slow/fast/stop/crawl (8/5/12/0/2 m/s requested speeds).
Safety limits can reduce those speeds. `--open-loop` bypasses the driver with
fixed 0.35 throttle and zero steering; it is not a random or neural policy.
`--barrier` inserts a full-height wall at x=45 m after the original route is
planned. It tests obstruction sensing/braking, not automatic replanning.

Sanitizers and the reproducible evidence runner:

```sh
cmake -S CarWorld -B CarWorld/build/sanitized \
  -DCMAKE_BUILD_TYPE=Debug -DCARWORLD_SANITIZE=ON
cmake --build CarWorld/build/sanitized -j 4
ASAN_OPTIONS=detect_leaks=1 ctest --test-dir CarWorld/build/sanitized \
  --output-on-failure
ASAN_OPTIONS=detect_leaks=1 python3 CarWorld/verify_headless.py \
  --episodes 100 --out CarWorld/build/my_new_verification
```

The archive directory must be new. The runner freezes project-owned compiler
inputs, binaries, per-drive metrics, traces, checksums, CTest logs and three
no-CSV timing samples. It checks exact seeds/counts, byte-identical CSV replay,
finite values, frame/substep reconciliation and obstacle swept clearance.
The runner is verification tooling, not an environment or physics fork.

## Tiles and shared core

Nine fixed 120 x 64 m fixtures use the existing TileWorld `TileMap`, with one
metre tiles. Pose/velocity/dynamics are continuous FP64, not cell hopping.
Terrain properties reuse `BikeWorld/VehicleMap.hxx`:

| Material | Friction | Rolling resistance | Grade dz/dx |
|---|---:|---:|---:|
| Empty/asphalt | 0.85 | 0.006 | 0 |
| Gravel | 0.45 | 0.025 | 0 |
| Ice | 0.10 | 0.004 | 0 |
| Grass | 0.40 | 0.050 | 0 |
| Uphill/downhill | 0.85 | 0.006 | +0.05 / -0.05 |

These are modeling assumptions, not fitted material constants. Material and
solidity are independent: only the existing collision flag blocks motion.
Unknown material labels default to asphalt but still collide when solid.
The four tire locations sample their own terrain cells each physics substep.
A grade is a planar gravity force field, not a rendered 3D elevation mesh.

`BikeWorld/VehicleRoute.h/.hxx` provide the same fixed-workspace, deterministic
four-neighbor A* and line-of-sight pruning. No second occupancy grid/planner is
introduced. Limits are 8192 cells and 256 route points. Fixtures use 6 m planning
clearance for the body envelope and pure-pursuit corner cutting; arbitrary
narrow roads and curvature feasibility are not guaranteed. Planner allocation
is outside the simulation tick and routes are not recomputed each tick.

`TVehicleSweep` continuously intersects motion segments with expanded tile
AABBs. The default 2.65 m heading-independent square envelope covers the declared
4.5 x 1.8 m chassis around the asymmetric CG. This is intentionally conservative,
not an exact oriented rectangle/contact manifold. Collision halts and terminates
at first contact; there is no teleport back to a route or hidden crash reset.
Three ideal point ranges feed observations; braking uses the inflated body sweep.

## Planar physics

`CarPhysics.h/.hxx` implement Newton-Euler rigid-body motion with four contact
forces, rear-wheel drive, finite tire cornering stiffness and axle brake bias.
The front wheels share one steering angle (parallel-steer approximation).
Each wheel's position is `(a,+/-track/2)` or `(-b,+/-track/2)` in body coordinates.
Its velocity is `(u-r*y, v+r*x)` and is rotated into its steering frame.

With `L=a+b`, normal acceleration `gn=g/sqrt(1+sx^2+sy^2)` and static wheel loads:

    N_front_wheel = m*gn*b/(2*L)
    N_rear_wheel  = m*gn*a/(2*L)
    Fy_wheel = -(C_axle/2)*atan2(v_tire, max(u_tire, 2 m/s))

Drive is force/power limited. Brakes oppose longitudinal tire motion, smoothly
regularized with `tanh(u_tire/0.05)`. Each combined `(Fx,Fy)` is projected onto
its own Coulomb circle of radius `mu*N`. Wheel forces rotate back into the body
frame; yaw moment includes both axle arms and left/right track offsets.

    xdot = u*cos(heading) - v*sin(heading)
    ydot = u*sin(heading) + v*cos(heading)
    heading_dot = r
    udot = Fx_total/m - resistance/m - gravity_forward + v*r
    vdot = Fy_total/m - gravity_lateral - u*r
    rdot = sum(x_wheel*Fy_body - y_wheel*Fx_body)/Iz

Defaults: mass 1400 kg, wheelbase 2.70 m, track 1.60 m, yaw inertia 2500 kg m^2,
axle stiffnesses 80000 N/rad, 7000 N / 60000 W drive limits, 16000 N brake demand,
65% front brake bias. Steering has 0.55 rad angle, 0.8 rad/s rate and 0.15 s
first-order lag limits. There is no path follower in the plant.

RK4 runs at 0.005 simulated seconds (200 Hz), four substeps per 0.02 s control
frame (50 Hz), without wall-time sleeps. Forces use sampled material constant
within each RK4 substep. Forward-only travel clamps negative longitudinal speed;
there is no reverse gear. Low-speed slip and brake regularizations are declared
approximations, not a full static-contact or wheel-spin model.

Inputs must be finite, with positive physical scales and valid material ranges.
Malformed inputs return false without mutating the plant. Engineering bounds
also reject enormous finite coordinates before indexing: positions <=1e9 m,
velocities/rates <=1e4 in their SI units, steering <=1.3 rad, friction <=2,
rolling coefficient <=1 and grades in [-1,1]. These are numerical domain guards,
not safe vehicle operating limits. Valid out-of-range actuator commands are
clamped. Validity does not establish timestep stability for arbitrary parameters.

The `tire_demand`/`peak_tire_demand` fields report requested force divided by the
friction limit BEFORE saturation; values above one are expected. Actual wheel
forces are clipped to the circle. Lateral-acceleration telemetry is physical
body lateral acceleration, not just the body-coordinate velocity derivative.

## Reference driver and iGeek interfaces

`ACarPilot` uses pure pursuit with speed-dependent lookahead, unsaturated-model
understeer compensation, proportional speed feedback and resistance feed-forward.
It previews route corners so low-grip braking starts before a turn, caps speed
using friction/curvature, tapers to the goal, and brakes for obstructions.
The driver has ideal pose, map, route and contact material, including a 40 m
ideal material preview. The nominal 2 m obstacle braking gap is a request;
actual measured gap is smaller and must not be described as a guaranteed margin.
The reduced driver is not certified for real roads or moving obstacles.

- `ACarEnv::Step()` accepts the five speed commands through the classical driver.
- `ACarEnv::StepControl()` accepts raw steer/throttle/brake and bypasses it.
- `Obs()` exposes 22 normalized FP32 features: body velocities, yaw/steer state,
  heading, local goal, route curvature/error/remaining length, four tire friction
  coefficients, two grade components, three ranges and episode progress.
- `Reset()` exactly replays the stored seed; `ResetSeed()` changes it. Seeds
  perturb heading by up to 0.025 rad and world y by up to 0.15 m; they do not
  randomize the terrain, car parameters or sensor errors.
- `ACarGym` derives from native iGeek `Gym`. It preallocates flat buffers for
  1..8 independent cars and preserves terminal reward/status through seeded
  auto-reset. Observations then identify the next episode, as native PPO expects.
- Default Gym actions select classical-driver speeds. `CarActuatorActions` uses
  nine raw categorical actions: steering {-0.18,0,+0.18} crossed with throttle
  0.35 / coast / brake 0.6. This mode does not call the driver. Continuous raw
  actuators remain available through `StepControl()`.
- `carworld_ppo_smoke` exercises existing `TLinearPolicy` and `TPPO` in BOTH modes:
  eight cars, horizon 64, two epochs, ten updates. Finite statistics establish
  interface compatibility only, not learned full-route competence. Existing
  `TPPO::CollectRollout()` resets the Gym at each rollout; this 1.28 s horizon
  does not train complete drives. That training issue is preserved, not silently
  changed in this simulator task.

## Measured results

Linux x86-64, virtual Intel Xeon (Cascade Lake), GCC 13.3, release `-O2`, one
simulation thread. Seeds 7..106, 100 drives per fixed fixture:

| Fixture | Goals | Mean simulated seconds | Peak cross-track m |
|---|---:|---:|---:|
| Straight | 100/100 | 14.460 | 0.953 |
| Curves | 100/100 | 18.143 | 1.288 |
| Obstacle detour | 100/100 | 18.884 | 1.563 |
| Gravel | 100/100 | 14.460 | 0.953 |
| Ice | 100/100 | 16.560 | 0.952 |
| Uphill | 100/100 | 14.460 | 0.953 |
| Downhill | 100/100 | 14.460 | 0.954 |
| Launch from rest | 100/100 | 15.540 | 0.955 |
| Ice plus curves | 100/100 | 35.970 | 1.010 |

900/900 goal stops, zero non-goal drives in this counted sample. Worst final
Euclidean goal error: 0.955374581 m. Every final planar speed was below 0.1 m/s
(worst 0.099998740 m/s). Peak cross-track includes the initial pose and is sampled
at physics endpoints (200 Hz); it includes distance beyond a polyline endpoint.
RMS cross-track and saved traces are sampled at 50 Hz. Do not describe the
roughly metre-scale errors as lane-centering or precision parking.

814685 control frames, 3258740 physics substeps and 16293.700 simulated seconds
per 900-drive benchmark. Three no-CSV wall-time samples were 11.020459,
10.832979 and 10.846583 s. Median throughput was 75109.8 control frames/s,
1502.2x simulated real time. Scope includes fixture construction, planning,
control, physics and observations; excludes process startup, planner allocation,
final output flush and CSV/trace logging. This is simulator throughput, not
PPO training throughput or a V100/5060 Ti benchmark.

Additional exercised behavior:

- Full braking from 10 m/s for 2 s: asphalt ends at approximately 0 m/s after
  5.946335 m, versus ice at 7.875362 m/s after 17.871762 m. These distances cover
  the two-second interval; the ice result is NOT a complete stopping distance.
- Left wheels on ice with right wheels on asphalt produce -0.357210 rad heading
  change during the same unsteered brake test: split-grip yaw is not suppressed.
- RK4 half-step comparison over 0.5 s gives y residual 5.7178956e-11 m.
- Unexpected-wall CLI fixture stops blocked with 0.346770641 m remaining beyond
  the conservative envelope. The unit fixture begins faster and retains
  0.345953 m. Neither result is a physical safety certification.
- Fixed-throttle open-loop obstacle control collides; 300 m/s collision test
  terminates at first contact without tunneling.
- Release and ASan/UBSan/float-cast-overflow builds pass all four CTest entries;
  the frozen source archive also rebuilds and passes all four from scratch.
- Existing BikeWorld's three tests and PacWorld's linear/transformer PPO probe
  still pass. The legacy PacWorld compile emits its existing SEAM redefinition
  warning. This is affected-world regression evidence, not workspace-wide green.

Evidence archive:
`/home/astarcale/AStarStarship/iGeek/CarWorld/build/verified_900_02/`.
Its `verification_summary.json` records 95 hashed artifacts and 57 frozen
project source inputs. `drives.csv` contains the exact 900 metrics rows;
`replay.csv` is byte-identical. Obstacle and ice-curve traces have 946 and 1799
rows, including the initial state. CTest/physics/PPO logs, emergency/open-loop
controls, frozen `bin/` and rebuildable `code/` are retained. The earlier `_01`
archive is incomplete: its timing validator was too strict for rates printed
to one decimal. A regression test fixes that report-only defect; `_02` is the
verified result. The existing archive cannot be overwritten by the runner.

## Limits and 3D extension

This is a reduced-order planar plant, not a calibrated full vehicle dynamics
package or safe hardware autopilot. Static wheel loads omit load transfer,
suspension, roll/pitch/heave, Ackermann geometry, tire relaxation, wheel spin,
ABS/traction control, powertrain shifts, reverse motion and impact dynamics.
There is no traffic, pedestrian model, sensor noise/latency or online replanning.
Those features need separate acceptance tests and measured calibration data.

For 3D, retain the map/route/Gym/evidence contracts, add a declared elevation and
contact-normal representation, a 6-DOF chassis/orientation, suspension/load
transfer and per-wheel contact state. Compare the new model with this archived
planar baseline before claiming a gain. A future learned driver must be tested
against the classical baseline on held-out layouts, not credited with this
controller's successful drives. Laplacian/MUSE policy integration and GPU
training are not implemented by this change.

All implementation/test files in this task are additive under `CarWorld/`.
No legacy files, BikeWorld, ASCIICrabs or CrabsTK were changed for this car task.
No subagents, commits, pushes, remote writes or NAS corpus changes were used.
