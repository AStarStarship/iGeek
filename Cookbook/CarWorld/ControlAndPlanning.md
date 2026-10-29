# CarWorld control and planning

[CarWorld chapter](./README.md) · [Physics](./Physics.md) ·
[Engineering](./Engineering.md) · [Course map](./CourseGuide.md)

The specialization separates mission, behaviour and local planning from
feedback control and actuator dynamics. CarWorld currently implements a
smaller vertical slice: a geometric route, a classical speed/steering pilot,
a dynamic plant, and explicit failures. It does not implement a full driving
stack merely because all fixture routes finish.

## Planning on the existing world

`TileMap` owns occupancy. `AVehiclePlanner` reuses its read-only queries;
there is no second collision map. Its deterministic four-neighbour A* uses
fixed-capacity arrays and Manhattan distance on one-metre fixture cells.
Line-of-sight pruning removes intermediate points when swept clearance holds.
Workspace supports 8192 cells and 256 output points. Allocate the planner
outside the hot stack/tick; return an empty route for invalid/unreachable or
over-capacity inputs.

The nine fixtures cover straight, curves, obstacle, gravel, ice, uphill,
downhill, launch and ice_curves on a 120x64m map. Seeds perturb initial
heading/lateral position, not the fixed map layouts. Curved fixtures supply
an explicit polyline. Other fixtures plan with 6m clearance to compensate
for the conservative body and the follower's corner cutting.

A collision-free grid route is NOT necessarily a drivable trajectory:

- Polyline corners have discontinuous heading; finite steering cannot turn
  instantaneously. Pruning changes geometry but does not impose curvature.
- Geometric collision checks do not prove bounded steering rate, lateral
  acceleration, jerk, tire demand or time-dependent obstacle clearance.
- Static A* does not predict pedestrians or other cars. There is no lane
  graph, right-of-way reasoning, lead-vehicle following or traffic-rule engine.
- Adding a barrier after planning tests braking against an obstruction;
  automatic rerouting is not implemented.

## Lateral guidance and steering

`TVehicleGuidance` projects the centre onto a simple polyline, computes
unsigned cross-track distance and remaining arc length, then chooses a target
a lookahead distance along that polyline. With target displacement (dx,dy)
and a supplied guidance heading h:

```
e_target = -sin(h)*dx + cos(h)*dy
curvature = 2*e_target / max(dx*dx + dy*dy, 1m^2)
```

This is a pure-pursuit-style geometric rule. The current pilot uses
`lookahead = 4m + 0.7s*u` and a guidance heading corrected by
`atan2(lateral_speed,max(forward_speed,2m/s))`. It is a centre-referenced
implementation with a slip-direction correction, not an exact copy of a
rear-axle no-slip course exercise.

A steady-state understeer correction converts curvature to target steer:

```
L = front_arm + rear_arm
K = mass/L * (rear_arm/front_stiffness - front_arm/rear_stiffness)
steer_target = clamp(atan((L + K*u*u)*curvature), -steer_limit, +steer_limit)
```

The plant's steering lag/rate limit then acts on that target. The controller
does not write pose, yaw or velocity to force successful path tracking.

The course also teaches Stanley steering: heading error plus an arctangent
of signed cross-track error divided by speed, with low-speed softening and
optional damping/feedforward. **Stanley is not currently implemented in
ACarPilot.** Neither are constrained MPC, sliding-mode control or an online
optimizer. Add them as separately tested alternatives rather than relabeling
this baseline.

## Longitudinal control

The nominal speed action supplies cruise_speed. Before requesting drive,
the pilot builds a conservative speed envelope:

1. Clamp the nominal request to [0,12]m/s.
2. Sample current tire materials and preview ideal material along the current
   motion direction up to 40m. This is privileged simulator knowledge, not
   perception of road friction from a camera.
3. Set lateral budget `min(2.5m/s^2, 0.4*mu*g)` and stopping deceleration
   `min(2.5m/s^2, 0.55*mu*g)`.
4. Anticipate upcoming polyline corners with a lookahead-based curvature
   estimate and distance-to-corner braking envelope.
5. Cap speed by `sqrt(lateral_budget/max(abs(curvature),0.001))`.
6. Taper to a halt using `sqrt(2*stop_decel*max(remaining-0.7m,0))`.
7. Sweep the inflated body 40m in its motion direction; reduce target speed
   using available distance minus a requested 2m braking margin.

These are model-based heuristics. A requested gap is not a mathematical
guarantee under arbitrary initial speed, friction, timestep or geometry;
measure actual clearance in each test.

The force request combines resistance feedforward and proportional speed
feedback:

```
F_resist = mass*g*mean_rolling + 0.5*rho*drag_area*u*u
         + mass*g*(mean_slope_x*cos(psi) + mean_slope_y*sin(psi))
F_request = F_resist + mass*(1.8/s)*(target_speed-u)
```

Positive force becomes bounded throttle divided by available drive force;
negative force becomes bounded brake divided by the brake-force scale.
Near a requested halt the pilot holds a brake command. This is a
feedforward-plus-P baseline, **not a full PID**: no integral state,
anti-windup or derivative speed term is implemented.

The course's richer engine/gearbox/torque-map longitudinal controller and
PI/PID design remain useful extensions. Evaluate rise time, overshoot,
settling, steady-state error, noise sensitivity and actuator saturation
before claiming that a different controller improves this one.

## Manual and autopilot ownership

The raw interface is `ACarEnv::StepControl(TCarControl)`; the speed interface
is Step(action). ACarRoom selects between them using its typed request.
Both integrate the same plant. The Room test compares these dispatches to
direct calls, rejects malformed schemas, and drives all nine layouts with
manual braking followed by autopilot resumption.

There is no keyboard/gamepad event loop yet. A future host must decide when
commands become effective, what happens on focus/connection loss, and whether
it holds the last command, brakes, or requests another explicit fallback.
Do not mistake rejection of a bad message for automatic emergency braking.
Single-owner stepping is required; a UI must not race a control thread.

## Learning path and independent baselines

Use the classical driver as a reference controller, not a learned result.
The five Gym speed actions retain classical steering. The nine raw-actuator
categories bypass it, but a finite PPO loss alone does not show successful
route learning, physical robustness or safe fallback.

A useful comparison protocol keeps map, initial state/seed, sensor access,
action limits, horizon and success/failure definitions matched across:

- raw open-loop controls (negative control);
- the current classical pilot;
- any new Stanley/PID/MPC implementation;
- a learned raw-actuator policy;
- a speed-selector policy, explicitly labelled as using the classical pilot.

Record every failure, not just successful trajectories. Test parameter,
friction and input-delay perturbations separately from tiny pose changes.
The current layouts contain no moving traffic and no noisy state estimator.

## License

Copyright [AStarship](https://astarship.net). [Repository license](../../LICENSE.md).
