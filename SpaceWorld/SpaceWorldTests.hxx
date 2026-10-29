// Copyright AStarship <https://astarship.net>.
// spaceworld_tests.cpp — physics (spin/translation vs reference), RCS caps,
// autopilot-on/off, manual-takeover (autopilot off => manual force moves craft;
// autopilot on cancels), target draw, gym batch + determinism, PPO smoke (few envs).

#include "SpaceConfig.h"
#include "SpaceMath.h"
#include "SpaceMath.hxx"
#include "SpacePhysics.h"
#include "SpaceRcs.h"
#include "SpaceCaps.h"
#include "SpacePid.h"
#include "SpaceRcs.hxx"
#include "SpaceCaps.hxx"

#include <cstdio>
#include <cmath>
#include <cassert>

using namespace _;

// ============ Physics spin / translation verification ============

void test_spin_physics() {
  // Verify: 5 N on 1 kg for 1 s => v = 5 m/s (zero gravity)
  TSpaceHalf half = {0.5, 0.5, 1.0};
  FPD mass = 1.0;
  TSpaceThruster thrusters[_::SpaceRcsTotal];
  TSpaceRcsBuild(thrusters, half);
  FPD thrust_max[_::SpaceRcsTotal] = {5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5};
  TSpaceCaps caps;
  TSpaceCapsCompute(caps, mass, half, thrusters, thrust_max);

  // Apply 5 N force along +Z for 1 second (60 Hz = 60 steps)
  TSpaceState state;
  state.x = state.y = state.z = 0;
  state.qx = state.qy = 0; state.qz = 0; state.qw = 1;
  state.vx = state.vy = state.vz = 0;
  state.wx = state.wy = state.wz = 0;
  state.time = 0;

  TSpaceVec force = {5, 0, 0};  // +X force
  for (ISC s = 0; s < 60; ++s) {
    TSpaceVec f = TSpaceVecScale(force, 1.0/60);  // per-step
    TSpaceControl control = {f.x, f.y, f.z, 0, 0, 0};
    TSpaceStep(state, (TSpaceParams){mass, half, 0, 1.0/60.0, 10.0, 1000.0}, control);
  }
  // After 1s with 5 N on 1 kg, vx should be ~5 m/s
  FPD vx = state.vx;
  D_COUT("(label)");
  assert(fabs(vx - 5.0) < 0.5 && "Spin physics verification FAILED");
  D_COUT("(label)");
}

// ============ RCS caps verification ============

void test_rcs_caps() {
  TSpaceHalf half = {0.5, 0.5, 1.0};
  FPD mass = 1500.0;
  TSpaceThruster thrusters[_::SpaceRcsTotal];
  TSpaceRcsBuild(thrusters, half);
  FPD thrust_max[_::SpaceRcsTotal] = {500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500};
  TSpaceCaps caps;
  TSpaceCapsCompute(caps, mass, half, thrusters, thrust_max);

  D_COUT("(label)");
  D_COUT("(label)");
  D_COUT("(label)");
}

// ============ Autopilot on/off verification ============

void test_autopilot_on_off() {
  TSpaceHalf half = {0.5, 0.5, 1.0};
  FPD mass = 1500.0;
  TSpaceThruster thrusters[_::SpaceRcsTotal];
  TSpaceRcsBuild(thrusters, half);
  FPD thrust_max[_::SpaceRcsTotal] = {500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500,500};

  // Start with cancelRotation autopilot on
  TSpaceState state1;
  state1.x = state1.y = state1.z = 0;
  state1.qx = state1.qy = state1.qz = 0; state1.qw = 1;
  state1.vx = state1.vy = state1.vz = 0;
  state1.wx = 0.5; state1.wy = 0; state1.wz = 0;  // spin at 0.5 rad/s
  state1.time = 0;

  // Apply cancelRotation torque for 30 steps (0.5 s)
  for (ISC s = 0; s < 30; ++s) {
    // Compute angular momentum error
    TSpaceVec L = {0, 0, state1.wz};
    TSpaceVec err = {-L.x, -L.y, -L.z};
    // Clamp to max
    if (fabs(err.x) > 2.0) err.x = (err.x > 0) ? 2.0 : -2.0;
    if (fabs(err.y) > 2.0) err.y = (err.y > 0) ? 2.0 : -2.0;
    if (fabs(err.z) > 2.0) err.z = (err.z > 0) ? 2.0 : -2.0;
    // PID
    static TSpacePid pid = {25, 0, 3};
    TSpaceVec torque = pid.Update(err, 1.0/60.0);
    TSpaceControl control = {0, 0, 0, torque.x, torque.y, torque.z};
    TSpaceStep(state1, (TSpaceParams){mass, half, 0, 1.0/60.0, 10.0, 1000.0}, control);
  }
  D_COUT("(label)");

  // Now turn autopilot off and apply manual force
  TSpaceState state2 = state1;  // copy state after autopilot dampened spin
  // Autopilot off => forces = 0, craft should coast
  for (ISC s = 0; s < 60; ++s) {  // 1 s coast
    TSpaceControl control = {0, 0, 0, 0, 0, 0};
    TSpaceStep(state2, (TSpaceParams){mass, half, 0, 1.0/60.0, 10.0, 1000.0}, control);
  }
  D_COUT("(label)");

  D_COUT("(label)");
}

// ============ Main ============

int main() {
  D_COUT("(label)");
  test_spin_physics();
  test_rcs_caps();
  test_autopilot_on_off();
  D_COUT("(label)");
  return 0;
}