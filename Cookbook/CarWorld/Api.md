# CarWorld API and Chinese Room commands

[CarWorld chapter](./README.md) · [Physics](./Physics.md) ·
[Control and planning](./ControlAndPlanning.md)

All native C++ types below live in `namespace _`. Use the declaration `.h`
files for contracts and one translation unit that includes the required
`.hxx` implementations. Do not compile implementation headers into several
translation units and assume all legacy ASCIICrabs definitions are inline.

## Plant, environment and driver

| Interface | Contract |
|---|---|
| `TCarState` | FP64 pose, body velocities, yaw, steering, time, distance and diagnostic acceleration/tire demand |
| `TCarParams` | Vehicle and actuator parameters; inspect [defaults](./Physics.md#default-parameters) |
| `TCarControl` | Steering angle target in radians, throttle/brake fractions |
| `TCarContacts` | Four independently sampled `TVehicleSurface` records |
| `TCarStep(state, params, control, contacts, dt)` | One physics integration step; false on invalid inputs without committing a new state |
| `ACarPilot::Control(map, route, state, params, cruise_speed)` | Classical controller produces actuators; it does not edit the plant |
| `ACarEnv(map, route, initial, seed)` | Borrowed map/route, owned simulation state and pilot |
| `ACarEnv::Step(action)` | One 20ms control frame using the classical pilot |
| `ACarEnv::StepControl(control)` | Same frame through raw actuators, bypassing the pilot |
| `Reset()` / `ResetSeed(seed)` | Replay the stored seed / change seed and reset |
| `State()` / `Metrics()` / `Obs()` | Borrowed read-only views, not retained immutable snapshots |
| `Done()` / `Status()` / `GoalDistance()` | Episode termination, reason and Euclidean goal error |

Map and route must outlive the environment and not be changed concurrently
with stepping. The Room must not outlive its environment. Callers should copy
telemetry before handing it to another thread; a const reference does not
freeze the underlying object. `Params()` is mutable: perform changes under
the same ownership rule and revalidate the resulting configuration.

After an episode terminates, further Step/StepControl calls return zero and
do not advance it. Goal means distance below 1.2m AND planar speed below
0.1m/s; reaching a waypoint at speed is not success. Other statuses are
`CarCollision`, `CarTimeout`, `CarInvalid`, `CarBlocked` and `CarStopped`.

Speed actions are `CarCruise`, `CarSlow`, `CarFast`, `CarStop`, `CarCrawl`,
requesting 8, 5, 12, 0 and 2m/s respectively. Safety envelopes may lower them.
These actions do not teach the network how to steer.

For a normal raw-control frame, reward is the reduction in projected
remaining route length, minus 0.001 and minus 0.001 times the post-frame
cross-track distance. CarGoal adds 10; collision/invalid adds a -10 penalty.
Invalid actions/control parameters rejected before integration return -10.
The classical Step path applies an additional -5 when it terminates blocked
at low speed. Reward is a learning signal, not a physical safety objective
or a calibrated probability of successful driving.

## What the Chinese Room adds

`ACarRoom` derives from `TRoom<CHA,CHC,ISC,ISC,DTB,IUC>` and binds an existing
`ACarEnv&` to native operations. The inline Room Floor is a 1024-byte ASCII
List with capacity for 16 tuple indices. This request uses six entries:

| Index | ASCII type | Meaning | Accepted values |
|---:|---|---|---|
| 0 | `_ISC` | Schema version | 1 |
| 1 | `_ISC` | Control source | `CarRoomAutopilot` or `CarRoomManual` |
| 2 | `_ISC` | Classical action | `[0, CarActionCount)` |
| 3 | `_FPD` | Steering target | Finite, absolute value at most 0.55rad |
| 4 | `_FPD` | Throttle | Finite, [0,1] |
| 5 | `_FPD` | Brake | Finite, [0,1] |

`SetCommand(TCarRoomCommand)` validates the request before replacing the
previous one. Check its boolean return: ignoring rejection and ticking would
execute the old request. Validity includes all fields even if the selected
mode does not use them. The plant applies its own parameterized actuator
limits; the Room's current steering admission limit remains fixed at 0.55rad.

`Star` validates count, types, offsets/alignment and packed Floor extent
before dereferencing its scalar values. Normal application code should use
SetCommand, not edit Floor metadata or manually reinterpret a C++ object as
a List. Corrupt Floor headers are not an alternate application API.

| Call | Result and side effect |
|---|---|
| `Star('?', NILP)` | Non-nil metadata Op; no physics advance |
| `Star('t', NILP)` | NILP on successful dispatch of one frame; otherwise non-nil error Op |
| `Star('r', NILP)` | Explicit seeded reset, default autopilot/cruise request, zero reward snapshot |
| Unknown operation | Non-nil error Op; no frame |
| Non-nil Crabs context | Rejected: interpreter/Slot argument decoding is not implemented here |

A successful dispatch is not necessarily a successful episode: inspect
`Reply().status`. The native reply contains version, mode, status, frame
count, reward and a `TCarState` snapshot. Reply is updated on tick/reset;
query/rejected requests leave the prior snapshot. `Reply()` itself returns
a borrowed reference that the next successful operation can overwrite.

The bridge to developer objects is explicit:

```
typed Floor fields -> validate -> virtual Operand::Star operation ID
                  -> ACarEnv::Step or StepControl -> native reply snapshot
```

No universal reflection/object factory is implied. An application owns its
C++ environment and supplies the reducer. Future Linux transport belongs in
CrabsTK, and must serialize fields with explicit version/length/byte order.
No live pointer, function pointer, vtable, descriptor or native padding goes
onto the wire. This is presently native dispatch, NOT a working remote Slot
protocol, database integration or keyboard/gamepad frontend.

## Complete native manual/autopilot example

Save this block as `$TMPDIR/CarWorldRoomExample.cpp`. Build from the iGeek
root using the command below. Construction/planning happen before ticks.

```cpp
// Copyright AStarship <https://astarship.net>.
#include "BikeWorld/TileRuntime.hxx"
#include <_Package.hxx>
#include "CarWorld/CarScenarios.hxx"
#include "CarWorld/CarRoom.hxx"

ISN main() {
  using namespace _;
  TileMap map(1.0f, 120, 64);
  AVehiclePlanner* planner = new AVehiclePlanner;
  TVehicleRoute route;
  TCarState initial;
  BOL planned = TCarScenarioBuild(map, CarStraight, *planner, route, initial);
  delete planner;
  if (!planned) return 1;

  ACarEnv env(map, route, initial, 7);
  ACarRoom room(env);
  TCarRoomCommand command;
  while (!env.Done()) {
    IUD frame = env.Metrics().frames;
    command.mode = frame >= 20 && frame < 25 ? CarRoomManual : CarRoomAutopilot;
    command.control = {0, 0, 0.2};
    if (!room.SetCommand(command) || room.Star('t', NILP) != NILP) return 2;
  }
  StdOut() << "CarWorld Room status=" << TCarStatusName(room.Reply().status)
           << '\n';
  return room.Reply().status == CarGoal ? 0 : 1;
}
```

```sh
g++ -std=c++2b -O2 -I. -IBikeWorld -I../ASCIICrabs \
  -I../ASCIICrabs/_Seams "$TMPDIR/CarWorldRoomExample.cpp" \
  -o "$TMPDIR/CarWorldRoomExample"
"$TMPDIR/CarWorldRoomExample"
```

The five manual braking frames are an example of source switching, not a
validated emergency fallback or a graphical driving demo. Time and pose
continue across both transitions; only an explicit reset starts over.
The example returns zero only on CarGoal. ASCIICrabs StdOut currently writes
its diagnostic through stderr, despite its name; capture both streams when
checking its printed status.
This example deliberately prints no numeric fields: the current upstream
integer formatter has an unaligned-store UBSan failure. See the known issue
in [Engineering](./Engineering.md); it is not a vehicle-physics failure.

## Gym and observation schema

`ACarGym(map, route, initial, count, seed, mode)` derives from root `Gym`.
Count is clamped to 1..GymEnvMax (currently 8). It exposes contiguous buffers:
observations `[B,22]`, actions/rewards/terminals `[B]`, action masks `[B,A]`.
The Gym owns these arrays; its map/route remain borrowed.

| Observation indices | Features and normalization |
|---|---|
| 0,1 | Forward/lateral body speed divided by 12m/s |
| 2,3 | Yaw rate / 2rad/s; steering / 0.55rad |
| 4,5 | Sine/cosine of heading |
| 6,7,8 | Goal in body frame / (120m,64m); goal distance / 120m |
| 9,10,11 | Guidance curvature times 10; cross-track / 8m; remaining length / 160m |
| 12..15 | Contact friction at front-left, front-right, rear-left, rear-right |
| 16,17 | Mean terrain slopes dz/dx, dz/dy |
| 18..20 | Ideal ranges at heading, heading+0.6rad, heading-0.6rad, divided by 40m |
| 21 | Frame count / MaxFrames (5000) |

Observation buffers and rewards use FPC (FP32); physics/state/control use
FPD (FP64). Normalization is not a guarantee that every feature is in [0,1].
Ideal ranges are geometric collision queries, not camera/LIDAR simulations.

`CarDriverActions` uses the five speed actions. `CarActuatorActions` uses
nine categories: `action % 3` selects steering -0.18/0/+0.18rad;
`action / 3` selects throttle 0.35, coast, or brake 0.6. Invalid actions
terminate with CarInvalid rather than indexing past a buffer.

`StepBatch` preserves terminal reward/flag and LastStatus while immediately
resetting that environment to the next seeded episode. Returned observation
is therefore the NEXT initial observation on a terminal step. ResetBatch
replays the batch's seed sequence. Do not pair a terminal reward with the
reset observation as though they belonged to the same state transition.

The PPO smoke checks finite updates in both modes, not learned driving.
Read the current `TPPO::CollectRollout`: it resets each rollout, so short
horizons do not establish complete-route learning.

## License

Copyright [AStarship](https://astarship.net). [Repository license](../../LICENSE.md).
