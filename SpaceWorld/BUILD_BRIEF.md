// Copyright AStarship <https://astarship.net>.
// ============================================================
// SpaceWorld build brief — source: ~/3P/spacecraft-autopilot/
// Handoff note for the concurrent builder. Read this before continuing.
// ============================================================
//
// GOAL: headless (no Three.js/Rapier/React) C++ clone of the spacecraft
// autopilot, as an iGeek RL gym in ./SpaceWorld/. The autopilot must be
// togglable ON/OFF, and the user (RL policy / operator) can take over with
// manual controls at any time.
//
// STYLE: follow ArmWorld exactly (same dir, /home/astar/AStarStarship/iGeek/ArmWorld).
// Chimera+ naming, namespace _, no stdlib, functional casts IUD()/ISC()/FPD(),
// #if SEAM >= CRABS_SPACEWORLD, AStarship copyright header, #pragma once + guard.
// Math via CrabsTK/Math (Blas.h: GemvR, GemmRR) where useful; scalar math is fine.
//
// VERIFIED PHYSICS (do not change): SpacePhysics.h/.hxx + SpaceMath.h/.hxx are
// DONE and verified (90-deg body-Z spin maps local +X to world +Y; 5 N on 1 kg
// gives v=5 after 1 s). SpaceMath has TSpaceVec/TSpaceQuat/TSpaceHalf/TSpaceAxis3/
// TSpaceGroups and the full vector+quaternion helper set. SpacePhysics has the
// 6-DoF TSpaceStep (world force + body torque, 2 substeps, quat integration
// qdot = 0.5 * w_world_quat * q). USE SpaceMath's TSpaceQuat* helpers instead of
// the inline quaternion code in SpacePhysics.hxx if you refactor — keep behavior
// identical.
//
// SPACECRAFT DEFAULTS (from spacecraftModel.ts):
//   body half-extents: hx=0.5, hy=0.5, hz=1.0 (width=1,height=1,depth=2)
//   mass: ~1500 kg (honeycomb panels 800 kg/m^3 x 0.02 thick + truss + ports +
//         fuel tank hydrazine 1004 kg/m^3). Use mass=1500.
//   per-thruster thrust = (mass/24) * THRUST_FACTOR(=5) ~= 312 N.
//   principal inertias: Ixx=Iyy=(m/12)(hy^2+hz^2)=(m/12)(0.25+1)=~156, Izz=(m/12)(1)=~125.
//   zero gravity, zero damping, 60 Hz physics (dt=1/60).
//   maxAngularMomentum = max(2, Ixx*0.5).
//
// RCS: 24 thrusters in 6 axis-pairs (4 each). SpaceMath TSpaceGroups already has
// the index tables matching spacecraftConfig.ts (forward/up/left/pitch/yaw/roll,
// each [positive,negative]). For a headless model you do NOT need per-thruster
// positions — model each group as producing a pure body-axis force (translation)
// or pure body-axis torque (rotation) when its 4 thrusters fire:
//   translation: forward=+Z, up=+Y, left=-X  (body frame)
//   rotation: pitch=+X, yaw=+Y, roll=+Z      (body frame)
//   group capacity (force) = 4 * thrust_per_thruster (N); torque capacity for an
//   axis = 4 * thrust * (lever arm). Use lever arm = half-extent on the opposite
//   axis (pitch~hx*? use the web's caps: angTorque = sum over group of f*arm).
//   Simpler + matches web capabilities: compute TRcsCaps once:
//     linForce[axis] = 4*thrust (N), linAccel = linForce/mass.
//     angTorque[axis] = 4*thrust*arm[axis] where arm = (pitch:hy, yaw:hz, roll:hx)
//       approx; angAccel = angTorque/I.
//   Clamp each thruster demand to [0, thrust_max]; sum group -> net.
//
// ALLOCATION (mirror ThrusterAllocator.ts + AutopilotMode.ts):
//   - Rotation: torque-domain. For axis a with pid torque t: fraction=min(1,|t|/angTorque[a]);
//     fire the 4 thrusters of the sign group at thrust*fraction (clamped per-thruster).
//   - Translation: force-domain. For axis a with force f: total=clamp(|f|,0,sumCap);
//     distribute total across the 4 sign-group thrusters by capacity share.
//   - Smoothing (EMA): rotation alpha 0.4 (cancel-rotation uses 0.0), translation 0.5.
//   - Manual (U/O/J/L/K/I) = translation keys; (W/A/D/Q/E) = rotation keys.
//     Scale: translation to saturate (thrust*24), rotation *100 (allocator clamps).
//
// AUTOPILOT (mirror Autopilot.ts + modes) — PIDController.ts vector PID:
//   p = kp*e; integral += e*dt (clamp |.|<=maxIntegral); i = ki*integral;
//   d_filtered = d_filtered*alpha + (e-lastE)/dt*(1-alpha); d = kd*d_filtered.
//   Gains (Autopilot.ts defaults):
//     orientation: kp=25, ki=0, kd=3   (angular-momentum domain, maxIntegral~0.3, alpha 0.9)
//     position:    kp=0.1, ki=0.001, kd=0.5  (maxIntegral 0.1, alpha 0.95)
//     momentum:    kp=6, ki=0.2, kd=2    (maxIntegral 0.2, alpha 0.9)
//   Five modes, each toggled independently (autopilot on if any active):
//     1. cancelRotation:  L = I*omega_local (I per axis); err = -L clamped to
//        maxAngularMomentum; PID(orientation) -> torque; hysteresis start 0.005,
//        stop 0.001 rad/s.
//     2. cancelLinearMotion: err = -m*v_local (clamp maxLinearMomentum=10);
//        PID(momentum) -> force.
//     3. orientationMatch: aim local +Z (forward) at targetOrientation; error
//        quaternion q_err = q_target * q_conj; use quaternionToAngularVelocity
//        (axis*angle) -> desired angular momentum; PID(orientation) -> torque.
//     4. pointToPosition: set targetOrientation = quat that points forward at
//        targetPosition (TSpaceQuatFromUnitVectors), then run orientationMatch.
//     5. goToPosition: braking velocity profile v_des = min(vMax, sqrt(2*aBrake*d));
//        vErr = v_des*dir_to_target - v; coast if |vErr|<max(0.1,0.1*vDes); else
//        force = m * vErr_local/responseTime (responseTime=clamp(0.3/aMin,0.1,1)),
//        clamp per axis to linForce cap, allocate.
//   Autopilot OFF (no active mode) => autopilot forces = 0.  This is the key
//   requirement: autopilot must be able to be turned off so manual takes over.
//
// ENVIRONMENT (SpaceWorld.h/.hxx) — class ASpacecraftEnv, like AArmEnv:
//   ObsLength = 14: [target x,y,z (scaled /50), pos x,y,z (/50), velocity xyz (/10),
//     angular vel xyz (/5), distance to target (/50), autopilot-on flag,
//     (pad)]. Keep <= 16.
//   Actions (categorical, manual controls — 1 axis at a time + hold):
//     0 hold, 1 +X, 2 -X, 3 +Y, 4 -Y, 5 +Z, 6 -Z, 7 pitch+, 8 pitch-, 9 yaw+,
//     10 yaw-, 11 roll+, 12 roll-  (SpaceActionCount=13).  These drive the
//     MANUAL allocator (autopilot off while the policy acts — or combine).
//   Autopilot control (NOT an RL action; programmatic, for take-over demos):
//     ASpacecraftEnv::AutopilotSetMode(isc, bol), AutopilotSetTarget(x,y,z),
//     AutopilotSetOrientation(x,y,z,w), AutopilotEnabled().  When a mode is on,
//     its forces ADD to the manual forces (like the web: manual + autopilot).
//   Reward: shaped = -distance_to_target (or -tip error); +10 on reach (radius
//     0.5 m); -1 if distance grows; timeout 2000 frames. Track metrics.
//   Step order (per web BasicWorld): physics.step(dt) then applyForces(dt) —
//     i.e. integrate pose, then compute next frame's forces. In Step(): compute
//     control (manual from action + autopilot if on) -> TSpaceStep.
//   Target: draw from seed (deterministic), within ~30 m, solvable.
//   Reset/ResetSeed like AArmEnv. StepControl(const TSpaceControl&) for raw use.
//
// GYM (SpaceGym.h/.hxx): AGymSpacecraft : public Gym, batch wrapper exactly like
// ArmGym.hxx (obs/reward/terminal/mask storage, StepBatch, seeded auto-reset).
//
// BINS:
//   spaceworld.cpp   — headless CLI: --mode goto|cancel|orient|manual --episodes N
//                      --trace. Demonstrates: (a) autopilot goto a target
//                      (reaches), (b) autopilot cancel-rotation (damps spin),
//                      (c) MANUAL TAKEOVER: start autopilot, then turn it off and
//                      drive with manual forces, show the craft obeys manual.
//   spaceworld_tests.cpp — physics (spin/translation vs reference), RCS caps,
//                      autopilot-on/off, manual-takeover (autopilot off => manual
//                      force moves craft; autopilot on cancels), target draw,
//                      gym batch + determinism, PPO smoke (few envs).
//   spaceworld_train.cpp — PPO trainer like armworld_train.cpp.
//   CMakeLists.txt    — 3 executables, includes ../CrabsTK/Math, ../../ASCIICrabs,
//                      _Seams; add_test(spaceworld_physics_reaching spaceworld_tests).
//
// !! VERIFICATION FINDING (2026-10-21, verified by compile-and-run in scratch) !!
// SpaceMath.hxx TSpaceQuatRotate is WRONG. Its code is:
//     t = Scale(Cross(qv,v), 2);  return v + Scale(t, q.w) + Cross(qv, t);
// which is v + q.w*(2 qv×v) + qv×(2 qv×v). The 2nd term is missing a factor of 2.
// A unit-rotation must be v' = v + 2*q.w*(qv×v) + 2*qv×(qv×v)  [i.e. v + 2*qw*t1 + 2*qv×t1, t1=qv×v].
// Proof (compiled): q = 90deg about X = (sin45,0,0,cos45); TSpaceQuatRotate(q,(0,1,0))
// returns (0, 2, 1)  -- length sqrt(5), NOT a unit rotation. Correct value is (0,0,1).
// FIX in SpaceMath.hxx TSpaceQuatRotate:
//     TSpaceVec t = TSpaceVecScale(TSpaceVecCross(qv, v), 2);
//     return TSpaceVecAdd(v, TSpaceVecAdd(TSpaceVecScale(t, q.w*2),
//                                         TSpaceVecScale(TSpaceVecCross(qv, t), 1)));
// (or equivalently t1=Cross(qv,v); v + Scale(t1, 2*q.w) + Scale(Cross(qv,t1), 2)).
// Until fixed, TSpaceStep orientation integration (which calls TSpaceQuatRotate to
// get w_world) integrates the quaternion wrongly -> craft does not rotate correctly.
// The qdot formula in SpacePhysics.hxx (qdot = 0.5 * w_world_quat * q) is CORRECT
// once w_world is computed correctly.
//
// BUILD (verify before done):
//   cd /home/astar/AStarStarship/iGeek
//   cmake -S SpaceWorld -B SpaceWorld/build -DCMAKE_BUILD_TYPE=Release
//   cmake --build SpaceWorld/build -j4
//   ./SpaceWorld/build/spaceworld_tests   (must pass 100%)
//   ./SpaceWorld/build/spaceworld --mode goto --episodes 2
//   Keep it fast/headless (target: thousands of frames/s). FP64 state, FPC obs.
//
// DO NOT: use stdlib containers, add dependencies, or modify ArmWorld.
// Keep CrabsTK usage consistent with ArmWorld (Blas.h include).
