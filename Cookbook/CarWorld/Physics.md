# CarWorld vehicle physics

[CarWorld chapter](./README.md) · [API](./Api.md) ·
[Control](./ControlAndPlanning.md) · [Course references](./CourseGuide.md)

The specialization builds from kinematics to forces, moments, tires and
actuators. CarWorld follows that distinction: the plant integrates forces;
the pilot requests bounded actuators. The plant does not snap onto a route.
The current model is reduced-order planar dynamics, not a calibrated full
vehicle, a complete course port, or a hardware safety argument.

## Frames, signs and units

- World x/y are metres in a fixed plane. Heading psi is radians from +x;
  positive yaw turns toward +y. The body origin is the centre of mass.
- Body u = forward velocity, v = lateral velocity, r = yaw rate. Positive
  lateral is left when heading is zero. Steering delta is the wheel angle
  target, not steering-wheel rotations or degrees.
- Lengths: metres; mass: kg; forces: N; power: W; inertia: kg*m^2;
  angular rate: rad/s; acceleration: m/s^2; time: simulated seconds.
- A terrain slope is dz/dx or dz/dy, not an angle in radians.

The implementation's world velocity is:

```
x_dot = u*cos(psi) - v*sin(psi)
y_dot = u*sin(psi) + v*cos(psi)
psi_dot = r
```

Do not copy coordinate-matrix labels or apparent x/y swaps from informal
notes without reconciling their frame convention. A signed-turn test is a
better check than assuming the notation matches another simulator.

## Kinematic reference versus the actual plant

For a no-slip car referenced to the rear axle, the familiar course model is
`psi_dot = speed*tan(delta)/wheelbase`, with velocity along body heading.
At the centre of mass, a kinematic formulation introduces
`beta = atan(lr*tan(delta)/wheelbase)` and resolves velocity along psi+beta.
A steering-rate input adds delta to the state rather than teleporting it.

These are useful low-acceleration reference models and potential test
oracles in their declared regime. They do not model finite tire grip,
split-surface braking, lateral momentum, yaw inertia or combined tire forces.
CarWorld instead integrates `[x,y,psi,u,v,r,delta]`; forward/lateral speeds and
yaw are dynamic state variables. It samples four tire contacts rather than
collapsing both tires on an axle into one bicycle contact.

## Default parameters

Source of truth: [CarPhysics.h](../../CarWorld/CarPhysics.h). These are
simulation assumptions, not parameters identified from a real car.

| Parameter | Default | Meaning |
|---|---:|---|
| mass | 1400kg | Rigid vehicle body |
| front_arm / rear_arm | 1.15m / 1.55m | CG to axles; wheelbase is their sum |
| track | 1.60m | Left/right contact separation |
| yaw_inertia | 2500kg*m^2 | Planar rotational inertia |
| front_stiffness / rear_stiffness | 80000N/rad each | Axle cornering stiffness; half per tire |
| drag_area | 0.65m^2 | Lumped drag coefficient times frontal area |
| air_density | 1.225kg/m^3 | Aerodynamic assumption |
| drive_force / drive_power | 7000N / 60000W | Rear-drive force/power limits |
| brake_force / brake_bias | 16000N / 0.65 | Total brake-force scale / front fraction |
| steer_limit | 0.55rad | Symmetric wheel-angle limit |
| steer_rate / steer_tau | 0.8rad/s / 0.15s | Rate limit / first-order steering lag |
| body_radius | 2.65m | Half-width of conservative world-aligned collision square |

## Four independent contacts

Contact order is front-left, front-right, rear-left, rear-right. In body
coordinates each contact is at `(front_arm or -rear_arm, +/-track/2)`.
Rotate and translate these positions into the map to sample material.
Each 5ms physics substep resamples the four positions.

For contact i at body `(xi,yi)` with wheel angle di:

```
local_forward = u - r*yi
local_side    = v + r*xi
u_tire = local_forward*cos(di) + local_side*sin(di)
v_tire = local_side*cos(di) - local_forward*sin(di)
```

Front tires use the same delta (parallel-steer approximation); rear tires
use zero steer. This is not Ackermann steering geometry.

Mean slopes sx/sy define `g_normal = g/sqrt(1+sx*sx+sy*sy)`. Static normal
load on EACH front tire is `mass*g_normal*rear_arm/(2*wheelbase)`;
each rear tire uses front_arm in the numerator. Dynamic longitudinal/lateral
load transfer and suspension travel are absent.

## Tire forces, drive and braking

The lateral law is linear cornering stiffness applied to a regularized slip
angle, then saturated jointly with longitudinal force:

```
Fy = -0.5 * axle_stiffness * atan2(v_tire, max(u_tire, 2m/s))
rear drive per tire = 0.5 * throttle * min(drive_force, drive_power/max(u,1m/s))
brake per tire = -0.5 * brake * brake_force * axle_bias * tanh(u_tire/0.05m/s)
```

Here axle_bias is brake_bias at the front and 1-brake_bias at the rear.
Throttle and brake are clamped to [0,1] in the plant. Front wheels have no
positive drive force in this model. Steering is angle/rate limited:

```
delta_dot = clamp((clamp(target,-limit,+limit)-delta)/tau, -rate, +rate)
```

For a tire with normal load N and local friction mu, if the requested force
magnitude exceeds mu*N, scale BOTH Fx and Fy by `mu*N/hypot(Fx,Fy)`.
Braking consumes lateral capacity; tires cannot independently supply their
maximum braking and maximum cornering forces at once.

The diagnostic utilization/tire-demand value is the PRE-clipping demand
ratio. Values above one indicate a saturated request, not proof that the
applied forces violated the friction circle. This matters when interpreting
`peak_tire_demand` in CSV output.

The model does not integrate wheel angular velocities or longitudinal slip
ratio. Force saturation is not ABS, ESC, traction-control logic or a fitted
Pacejka/Magic Formula tire. Camber, tire temperature, wear and pressure are
not simulated. The 2m/s slip chart and tanh near-zero braking/rolling terms
are explicit numerical/low-speed approximations.

## Force balance and integration

Rotate tire Fx/Fy back to the body frame, sum forces, and accumulate yaw
moment `sum(xi*Fy_body - yi*Fx_body)`. Aerodynamic drag is
`0.5*rho*drag_area*u*abs(u)`. Rolling resistance sums each contact's
`N*rolling*tanh(u/0.2m/s)`. Terrain gravity projects into body axes:

```
g_forward = g_normal*(sx*cos(psi) + sy*sin(psi))
g_side    = g_normal*(-sx*sin(psi) + sy*cos(psi))
ax = (sum(Fx_body) - drag - rolling)/mass - g_forward
ay = sum(Fy_body)/mass - g_side
u_dot = ax + v*r
v_dot = ay - u*r
r_dot = yaw_moment/yaw_inertia
```

A control frame holds its input over four FP64 RK4 substeps of 0.005s,
for 0.02 simulated seconds per frame. `TCarStep` accepts finite dt in
(0,0.01]; passing a whole 0.02s frame to that primitive is rejected.
The environment, not the primitive, performs the four substeps and collision
queries. Speed is clamped forward-only; there is no reverse gear.

Every prospective next state is checked before the primitive commits it.
Environment-level failure also changes status/reward; do not confuse that
with mutating a valid physics state through an invalid command.

## Terrain and collision are different data

[VehicleMap.hxx](../../BikeWorld/VehicleMap.hxx) reuses TileWorld occupancy.
A non-solid ice tile changes friction without becoming a wall. A solid tile
with an unknown material still collides; unknown material defaults to asphalt.

| Material | Friction mu | Rolling coefficient | Slope sx |
|---|---:|---:|---:|
| Empty/asphalt | 0.85 | 0.006 | 0 |
| Gravel | 0.45 | 0.025 | 0 |
| Ice | 0.10 | 0.004 | 0 |
| Grass | 0.40 | 0.05 | 0 |
| Uphill | 0.85 | 0.006 | +0.05 |
| Downhill | 0.85 | 0.006 | -0.05 |

Collision uses a swept world-aligned square against expanded tile AABBs and
solid map boundaries. This is conservative, not a rotating exact chassis
or multibody impact solver. First contact truncates position and terminates
the episode; the vehicle is not silently teleported back onto a route.
Grades exert planar forces: there is no 3D height mesh, pitch, heave or roll
state in the car model.

## Engineering limits and validation

The course motivates richer powertrain inertia, torque maps, wheel slip,
load transfer and 3D body/suspension models. Those are future extensions,
not hidden in these parameters. Add measured calibration data and independent
reference comparisons before using this as a physical vehicle model.

Current tests include zero-resistance coasting, known longitudinal acceleration,
signed turning, dry/ice braking, split-grip yaw, friction-circle clipping,
RK4 half-step convergence, invalid-state rejection and no-tunneling collision.
See [Engineering](./Engineering.md) for actual test commands and the difference
between numerical verification and physical validation.

## License

Copyright [AStarship](https://astarship.net). [Repository license](../../LICENSE.md).
