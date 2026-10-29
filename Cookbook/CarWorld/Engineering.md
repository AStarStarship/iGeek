# CarWorld engineering and verification

[CarWorld chapter](./README.md) · [API](./Api.md) ·
[Physics](./Physics.md) · [Course map](./CourseGuide.md)

## System architecture and dependency direction

```
Host input / replay / future frontend
           |
           v
validated application command -> ACarRoom -> ACarEnv -> TCarStep
                                    |          |
                                    |          +-> TileMap / shared VehicleMap
                                    +-> copied reply / telemetry

ASCIICrabs: abstract Room, List, schema primitives, Door/Slot/interpreter
CrabsTK: concrete Linux process/IPC/file/clock/input/rendering/BLAS services
iGeek: vehicle dynamics, observations, policy/control and application reducer
```

Do not put a Linux shared-memory client or renderer in the abstract Room.
Do not make CrabsTK depend on CarWorld to implement a reusable transport.
The current native binding is a working object bridge, not a completed Slot
scheduler or portable RPC system. The human-supervised CrabsTK session has
the concrete adapter backlog in its AGENT_PLAN.md.

Contiguous memory means well-defined arrays and bounded records, not a
promise that all Walls share one allocation or that native C++ padding is a
wire format. The Floor owns its tuples; environment, map and route lifetimes
are explicit. Queued consumers need independent snapshots or a documented
publication mechanism, not retained pointers into mutable storage.

## Test the correct entry points

Run from the iGeek root:

```sh
cmake -S CarWorld -B CarWorld/build -DCMAKE_BUILD_TYPE=Release
cmake --build CarWorld/build -j4
ctest --test-dir CarWorld/build --output-on-failure
CarWorld/build/carworld_room_tests
```

Current CTest targets:

| Test | What is checked | What is not established |
|---|---|---|
| `carworld_physics_routes` | Analytic coasting/drive checks, turning, dry/ice/split grip, RK4 convergence, route/goal/obstruction/collision, invalid input and Gym semantics | Calibrated physical-car accuracy or arbitrary-road feasibility |
| `carworld_ppo_interfaces` | Finite policy/value/entropy/KL/statistics in both action modes | Learned route completion or sample efficiency |
| `carworld_room_binding` | Query/tick/reset, schema/type/version/offset/control rejection, native/direct-call agreement, nine takeover routes | Remote transport, scheduling, auth or thread safety |
| `carworld_cli` | CLI validation, deterministic replay, scenario/trace and I/O behaviours | General scenario distribution coverage |
| `carworld_evidence_validation` | Checks the evidence validator's numerical/counting expectations | A new full benchmark campaign |

The last two require Python at configuration time. Inspect CTest's registered
list and actual failures rather than assuming every environment runs five
checks. The direct native executables return nonzero when their checks fail.
A printed assertion without a failing process exit is not a CI gate.

Recent native integration verification passed five registered checks in
Release and five in the sanitizer build. The Room executable printed nine
successful takeover routes. Those are bounded verification results, not a
claim that every iGeek world, every ASCIICrabs seam or IPC is now correct.

## Sanitizers and numerical contracts

```sh
cmake -S CarWorld -B CarWorld/build/sanitized \
  -DCMAKE_BUILD_TYPE=Debug -DCARWORLD_SANITIZE=ON
cmake --build CarWorld/build/sanitized -j4
ASAN_OPTIONS=detect_leaks=1 ctest --test-dir CarWorld/build/sanitized \
  --output-on-failure
```

Use AddressSanitizer, UBSan and float-cast-overflow checks. Test finite and
range-valid values BEFORE array indexing or numeric casts. Invalid primitive
inputs must leave physics unchanged. Test capacity exhaustion before counting
a List append as successful, and preserve FP32/FP64 bit representations when
storing floating tuples rather than numerically casting them to integers.

RK4 half-step agreement is a numerical convergence check within the model,
not validation of tire parameters. Test the raw plant independently of the
pilot so a controller cannot conceal a defective force law. Preserve
open-loop collision and obstruction-stop negative controls.

## Reproducible runs and telemetry

### Known upstream diagnostic-printer issue

While validating this chapter's example, adding
`StdOut() << room.Reply().frames` produced a UBSan alignment error in
`ASCIICrabs/Puff.hpp::SPrintCharPair` (line 110 in the inspected revision).
The fallback stores through an IUB pointer that may point at an odd byte
address. The example reached CarGoal before the numeric print failed;
Release printed 724 frames, but that does not make the store defined.

The example now prints only its string status and returns failure unless
the goal was reached. This is not a fix or waiver of the numeric-printer
defect. Keep the failing log/reproducer and give the generic formatting fix
to the ASCIICrabs owner; do not put a duplicate integer formatter in CrabsTK
or disable alignment sanitization globally. Existing CLI CSV output does
not exercise this same COut integer-print path.

Use fresh output filenames; the CLI refuses to overwrite existing evidence.
A trace requires exactly one scenario and one episode, with separate CSV
and trace paths. For example:

```sh
CarWorld/build/carworld --scenario obstacle --episodes 1 --seed 7 \
  --csv CarWorld/build/ObstacleMetrics.csv \
  --trace CarWorld/build/ObstacleTrace.csv
CarWorld/build/carworld --scenario straight --barrier
CarWorld/build/carworld --scenario obstacle --open-loop
```

The last two are negative controls and normally return 1 (blocked and
collision respectively), not zero. The CLI exit convention is 0 = all
requested drives reached the goal, 1 = at least one non-goal episode,
2 = argument/I/O error. The open-loop command is zero steer and fixed 0.35
throttle; it is not a random learned policy.

Per-drive CSV records scenario/seed/status, simulated duration, distance,
goal error, final planar speed, peak and RMS cross-track, peak speed/lateral
acceleration/tire demand, frame count and physics-step count. Trace rows
record the initial state and 50Hz control-frame endpoints. Peak metrics also
sample initial state and physics-substep endpoints; they are not continuous
analytic maxima. Collision can terminate partway through a frame's substeps.

Count output rows and reconcile requested seeds, scenarios and status totals
programmatically. Compare identical seeded CSV replay byte-for-byte on the
same build/platform. Cross-compiler/architecture bitwise agreement is not
promised. Retain source revision/dirty state, compiler/configuration, commands,
exit codes and code hashes with measured results.

Measure wall duration with a monotonic clock. Report control frames, physics
substeps and simulated seconds separately. The CLI timing covers setup,
planning, control, physics, observations and any requested output; it is not
PPO training throughput or a GPU/MKL speedup measurement. Do not copy a
historical timing result into a new hardware claim.

## Safety engineering, without a deployment claim

Course 1's safety module distinguishes hazards, faults, operational limits,
fallback, human interaction, cybersecurity and data recording. Apply that
structure here without claiming a certified automotive process.

| Hazard or failure mode | Current response or evidence | Required future work |
|---|---|---|
| Nonfinite or malformed command | Room rejection before a frame; primitive input checks | Input-source timeout and explicit fallback policy |
| Unexpected solid obstruction | Ideal sweep sensing; pilot attempts bounded braking; CarBlocked/CarCollision reported | Moving agents, uncertainty and replanning |
| Low grip on a turn | Independent contact friction, saturation and pilot preview | Measured tire model, transfer/suspension and robustness limits |
| Unsafe path geometry | Collision clearance tested on supplied fixtures | Curvature/rate/jerk-constrained planning |
| Peer/input disappears | No production transport/frontend yet | Host watchdog, command age, safe state, teardown and supervision |
| Incorrect sensor state | Pilot currently uses ideal state/map | Noisy/delayed sensors, estimator, integrity monitor and fault injection |
| Race or dangling snapshot | Single-owner native contract | Tested queue/snapshot publication with explicit ownership |

A fault-tree or FMEA exercise can identify missing requirements. Do not
fabricate occurrence probabilities or call a checklist an ISO 26262/SOTIF
certification. Many repeats of a small deterministic fixture set cannot
establish road fatality risk, broad ODD safety or human-driver superiority.

## Moving toward a playable kit

The next integration should add host input, live presentation and optional
real-time pacing without changing headless simulation time. Display control
source and failure state. Make reset explicit; switching manual/autopilot
must not teleport or silently restart the car. Losing focus or a connection
must not leave an undocumented persistent throttle command.

Use CrabsTK for concrete OS input/rendering and process/thread lifecycles.
Keep drawing rate independent from physics rate, publish coherent bounded
snapshots, and retain headless regression tests without a display/GPU.
The existing native CarWorld build does not require a working Visual Studio
project; Windows/MSVC support must be generated and tested on that platform,
not asserted from a Linux build.

## License

Copyright [AStarship](https://astarship.net). [Repository license](../../LICENSE.md).
