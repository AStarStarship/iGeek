# Self-Driving Cars Specialization: CarWorld learning map

[CarWorld chapter](./README.md) · [Physics](./Physics.md) ·
[Control and planning](./ControlAndPlanning.md) · [Engineering](./Engineering.md)

This guide relates CarWorld to the University of Toronto's
[Self-Driving Cars Specialization](https://www.coursera.org/specializations/self-driving-cars).
The local reference is the `Self-Driving-Cars-Specialization` checkout.
It contains third-party notes and learner notebooks, not a verified iGeek
implementation or an authoritative automotive safety specification.

This is original explanatory documentation and a course-to-code map, not a
copy of lectures, textbooks, images, assignment answers or licensed course
assets. Consult their original rights/terms before redistributing material.
The current kit does not require CARLA, notebook grader packages, Python
scientific libraries or downloaded course assets at runtime.

## Course 1: requirements, architecture, safety, models and control

| Module | Engineering lesson | CarWorld relationship |
|---|---|---|
| 1. Requirements for autonomy | Define tasks, operating conditions, responsibility and fallback | Describe the current fixed-map, ideal-state, forward-only simulator domain; do not imply a road-deployment autonomy level |
| 2. Hardware/software architecture | Sensors, perception, localization, planning, control, actuation and interfaces are separate | iGeek owns dynamics/control; abstract Room is ASCIICrabs; concrete OS/hardware adapters are CrabsTK |
| 3. Safety assurance | Hazards, FTA/FMEA, intended functionality, testing, human fallback and recording | Negative controls, explicit failure statuses and reproducible evidence; no hardware safety certification |
| 4. Vehicle dynamic modeling | Coordinate frames, kinematic bicycle, Newton-Euler dynamics, resistance, powertrain, steering and tire models | Four-contact planar force model with finite grip and bounded steering; full derivation and differences in Physics.md |
| 5. Longitudinal control | P/PI/PID design, response metrics, feedforward and feedback, actuator nonlinearities | Current resistance-feedforward + proportional speed feedback; no integral/derivative controller or engine-map lookup |
| 6. Lateral control | Heading/cross-track error, pure pursuit, Stanley and MPC | Current pure-pursuit-style curvature and understeer correction; Stanley/MPC are not implemented |
| 7. Putting it together | Compose models/controllers, evaluate path and speed tracking | Headless scenarios, raw plant, Gym and native Room takeover tests; no CARLA dependency |

### Physics exercises and their scope

The reference kinematic notebook evolves centre-of-mass position, heading,
steering and geometric sideslip from speed and steering-rate inputs. Its
circle/figure-eight exercises are useful for frame/sign/rate-limit checks.
They are not substitutes for a four-tire force model or bicycle roll dynamics.

The longitudinal notebook connects throttle, engine torque, effective gear
ratio, engine/wheel angular velocity, load forces and slip-dependent traction.
CarWorld currently uses bounded force/power actuation instead; wheel speed,
engine inertia, gearbox selection and slip-ratio dynamics are absent. Do not
claim those mechanisms merely because the simulation accelerates uphill.

Learner solutions must be reviewed before reuse. For example, a one-sided
steering-rate check is not a symmetric physical actuator bound, resetting pose
on an invalid input is not input rejection, and a positive saturated tire force
for both slip signs is not a general braking model. CarWorld's contracts and
independent tests, not notebook completion, determine accepted behaviour.

Suggested independent extensions, each requiring tests BEFORE implementation:

- A kinematic reference oracle with explicit rear-axle versus CG convention.
- Constant-radius and figure-eight tracking with bounded steer/rate and
  signed error, compared only in the reference model's valid regime.
- A PI/PID speed-controller alternative with anti-windup and declared
  derivative filtering; report overshoot, settling and disturbance rejection.
- A Stanley alternative with consistent signed front-axle error and low-speed
  softening; compare to pure pursuit on matched seeds/constraints.
- Identified tire/powertrain/suspension models, with measured parameter data
  and unit/sign/energy checks rather than tuning only until goals pass.

## Course 2: state estimation and localization

The course covers least-squares estimation, linear/nonlinear Kalman filters,
GNSS/INS, LIDAR/scan matching and a combined state estimator. Its project
uses an error-state extended Kalman filter with CARLA-derived data.

| Topic | What an implementation needs | Current CarWorld status |
|---|---|---|
| Least squares | Explicit measurement model, residuals, weighting and identifiable parameters | Not implemented as a vehicle estimator/calibrator |
| Linear/nonlinear filtering | State transition, Jacobians or sigma-point contract, process/measurement covariance | No Kalman/EKF/UKF state estimator |
| GNSS/INS | Sensor/body/world transforms, timestamps, biases, gravity and sampling rates | No simulated GPS/IMU stream; ideal pose/velocity is directly available |
| LIDAR/ICP | Scan geometry, correspondences, registration and uncertainty | Ideal collision ranges are not scan matching or an ICP pipeline |
| Fusion/integrity | Synchronization, latency/outliers/dropout, initialization and uncertainty validation | Future explicit sensor/estimator boundary |

When adding this layer, keep simulator ground truth distinct from controller
state estimates. Seed sensor noise separately, retain timestamps/covariance,
and test estimation error/consistency and fault response rather than only
route success. A perfect-state controller is not evidence for localization.

## Course 3: visual perception

The curriculum includes 3D vision/calibration, feature detection/description/
matching, neural-network foundations, object detection, semantic segmentation
and an integrated drivable-region/object-perception project.

| Topic | Engineering output | Current CarWorld status |
|---|---|---|
| Camera geometry/calibration | Declared intrinsics/extrinsics, projection, depth and frame conventions | No camera model or image renderer |
| Features and visual odometry | Trackable correspondences, motion estimate, degeneracy/outlier handling | Not implemented |
| Neural perception | Trained model, labeled data/splits, declared preprocessing and inference contract | PPO smoke is not a perception model |
| Detection/tracking | Objects with identity, geometry, confidence and timestamp | No dynamic detected-object list |
| Semantic/drivable-region estimation | Surface labels with uncertainty, distinct from collision truth | Tile material/solidity are simulator truth, not predicted segmentation |

A future renderer is an observation producer, not proof of perception.
Hardware camera/GPU integration belongs behind CrabsTK adapters; image-based
policies and world semantics remain explicit application components. Do not
mix ground-truth map access into a claimed camera-only controller.

## Course 4: motion planning

| Module | Engineering lesson | CarWorld relationship |
|---|---|---|
| 1. Planning problem | Mission, behaviour, local trajectory, constraints and objective | Current fixture route plus separate pilot; not a full hierarchy |
| 2. Mapping for planning | Occupancy and efficient collision checks | Reuses TileMap and continuous swept-body checks; map is known exactly |
| 3. Mission planning | Graph search, Dijkstra/A*, route representation | Deterministic grid A* and pruning, not a road/lane mission graph |
| 4. Dynamic interactions | Predict other agents and reason about changing clearance | No moving traffic prediction or lead-car following |
| 5. Behaviour planning | Select manoeuvres using context/rules/state machines | Speed-action selection and terminal statuses are not an intersection/traffic FSM |
| 6. Reactive local planning | Generate collision-free, dynamically feasible candidates and speed profiles | Obstruction braking and heuristic speed envelope; no general local trajectory optimizer |
| 7. Smooth local planning | Smooth paths/velocity profiles, integrate avoidance/following/intersections | Polyline pruning does not guarantee curvature/rate/jerk continuity; future tested extension |

Proposed next exercises: demonstrate an A* route that is geometrically clear
but steering-infeasible; introduce a curvature-constrained local planner;
add time-indexed moving obstacles; add lead-vehicle spacing and an explicit
behaviour machine. Define collision, comfort, progress and rule metrics
before optimizing. Keep negative/no-path scenarios in the evaluation.

## Reading references and source traceability

Public notes mirror used by the local course materials:
[Self-Driving Cars Specialization notes](https://github.com/afondiel/Self-Driving-Cars-Specialization-Coursera).
The upstream notes may change independently of the local checkout. The
paths below identify the local material behind this chapter, not bundled
files inside iGeek:

- `README.md`: complete four-course curriculum map.
- `Course1-Introduction-to-Self-Driving-Cars/course1-w3-notes.md`: safety,
  hazard analysis, analytical versus empirical assurance.
- `Course1-Introduction-to-Self-Driving-Cars/course1-w4-notes.md`: frames,
  kinematics, longitudinal/lateral dynamics, actuators, slip and tire models.
- `Course1-Introduction-to-Self-Driving-Cars/course1-w5-notes.md`: longitudinal
  P/PI/PID and feedforward/feedback design.
- `Course1-Introduction-to-Self-Driving-Cars/course1-w6-notes.md`: geometric
  tracking, pure pursuit, Stanley and MPC.
- `Course1-Introduction-to-Self-Driving-Cars/resources/w4/Lab/`:
  `Kinematic_Bicycle_Model_w4.ipynb` and `Longitudinal_Vehicle_Model_w4.ipynb`;
  source cells, not saved plots/grader results, inform the exercise discussion.
- Courses 2, 3 and 4 `readme.md` and the root curriculum table establish the
  broader learning map. Their detailed algorithms have NOT all been ported,
  and this chapter is not a replacement for the full readings/assignments.

The course cites Rajamani's *Vehicle Dynamics and Control* and Snider's
*Automatic Steering Methods for Autonomous Automobile Path Tracking* as
further background. Use original sources to resolve conflicting notation or
informal notes; do not propagate typos as requirements. References and a
successful simulator run are not experimental validation of a physical car.

## License

Copyright [AStarship](https://astarship.net). [Repository license](../../LICENSE.md).
Third-party course materials retain their own licenses and terms.
