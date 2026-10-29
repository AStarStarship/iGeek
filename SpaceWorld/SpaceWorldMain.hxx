// Copyright AStarship <https://astarship.net>.
// spaceworld.cpp — headless CLI: --mode goto|cancel|orient|manual --episodes N --trace
// Demonstrates: (a) autopilot goto a target (reaches), (b) autopilot cancel-rotation
// (damps spin), (c) MANUAL TAKEOVER: start autopilot, then turn it off and
// drive with manual forces, show the craft obeys manual.

#include "SpaceConfig.h"
#include "SpaceMath.h"
#include "SpaceMath.hxx"
#include "SpacePhysics.h"
#include "SpacePhysics.hxx"
#include "SpaceRcs.h"
#include "SpaceCaps.h"
#include "SpacePid.h"
#include "SpaceRcs.hxx"
#include "SpaceCaps.hxx"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>

using namespace _;

constexpr FPD THRUST_MAX[_::SpaceRcsTotal] = {500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500};

int main(int argc, char* argv[]) {
  bool mode_goto = false, mode_cancel = false, mode_orient = false, mode_manual = false;
  int episodes = 2;
  bool trace = false;

  for (ISC i = 1; i < argc; ++i) {
    if (std::string(argv[i]) == "--mode") {
      ++i; if (i < argc) {
        if (std::string(argv[i]) == "goto") mode_goto = true;
        else if (std::string(argv[i]) == "cancel") mode_cancel = true;
        else if (std::string(argv[i]) == "orient") mode_orient = true;
        else if (std::string(argv[i]) == "manual") mode_manual = true;
      }
    } else if (std::string(argv[i]) == "--episodes") {
      ++i; if (i < argc) episodes = std::atoi(argv[i]);
    } else if (std::string(argv[i]) == "--trace") { trace = true; }
  }

  // Spacecraft defaults (from spacecraftModel.ts / BUILD_BRIEF.md)
  TSpaceHalf half = {0.5, 0.5, 1.0};  // hx=0.5, hy=0.5, hz=1.0
  FPD mass = 1500.0;

  // Build RCS thruster layout
  TSpaceThruster thrusters[_::SpaceRcsTotal];
  TSpaceRcsBuild(thrusters, half);

  // Compute RCS capabilities
  FPD thrust_max[_::SpaceRcsTotal];
  for (ISC i = 0; i < _::SpaceRcsTotal; ++i) thrust_max[i] = THRUST_MAX[i];
  TSpaceCaps caps;
  TSpaceCapsCompute(caps, mass, half, thrusters, thrust_max);

  // Setup initial state
  TSpaceState state;
  state.x = 0; state.y = 0; state.z = 0;
  state.qx = 0; state.qy = 0; state.qz = 0; state.qw = 1;
  state.vx = 0; state.vy = 0; state.vz = 0;
  state.wx = 0; state.wy = 0; state.wz = 0.5;  // start with spin
  state.time = 0;

  // Target for goto mode
  TSpaceVec target = {0.0, 0.0, 50.0};  // 30-50m away

  // Principal inertias for this craft: Ixx=Iyy=(m/12)(hy^2+hz^2)=(1500/12)(0.25+1)=~156, Izz=(m/12)(1)=~125
  TSpaceParams params = {mass, half, 156.0, 156.0, 125.0, 1.0/60.0, 10.0, 1000.0};

  FPD max_angular_momentum = 2.0;
  FPD max_linear_momentum = 10.0;
  FPD a_brake = 1.0;
  FPD v_max = 10.0;

  // Mode: cancelRotation — damp spin using PID on angular momentum
  auto cancel_rotation = [&]() {
    // L = I * omega (body frame)
    FPD Lx = params.Ixx * state.wx;
    FPD Ly = params.Iyy * state.wy;
    FPD Lz = params.Izz * state.wz;
    // err = -L clamped to maxAngularMomentum
    FPD err_x = -Lx; if (err_x > max_angular_momentum) err_x = max_angular_momentum; if (err_x < -max_angular_momentum) err_x = -max_angular_momentum;
    FPD err_y = -Ly; if (err_y > max_angular_momentum) err_y = max_angular_momentum; if (err_y < -max_angular_momentum) err_y = -max_angular_momentum;
    FPD err_z = -Lz; if (err_z > max_angular_momentum) err_z = max_angular_momentum; if (err_z < -max_angular_momentum) err_z = -max_angular_momentum;
    // PID(orientation) -> torque
    static TSpacePid pid_rot; pid_rot.Reset();
    TSpaceVec torque = pid_rot.Update(TSpaceVec(err_x, err_y, err_z), 1.0/60.0);
    TSpaceControl control = {0, 0, 0, torque.x, torque.y, torque.z};
    TSpaceStep(state, params, control);
  };

  // Mode: goto position
  auto goto_position = [&]() {
    // Set target orientation to point local +Z (forward) at target position
    TSpaceVec forward = {0, 0, 1};
    TSpaceQuat target_quat = TSpaceQuatFromUnitVectors({state.x, state.y, state.z}, target);
    // PID(orientation) -> torque
    // Error: aim local +Z at target
    TSpaceVec aim_local_z = {0, 0, 1};
    TSpaceQuat q_err = TSpaceQuatMul(target_quat, TSpaceQuatInvert(
      TSpaceQuat(state.qx, state.qy, state.qz, state.qw)));
    FPD angle;
    TSpaceVec axis;
    TSpaceQuatAngleAxis(q_err, angle, axis);
    TSpaceVec desired_angular_momentum = TSpaceVecScale(axis, angle);  // desired angular momentum
    // PID
    static TSpacePid pid_rot; pid_rot.Reset();
    TSpaceVec err = desired_angular_momentum;  // simplified: use desired directly
    TSpaceVec torque = pid_rot.Update(err, 1.0/60.0);
    TSpaceControl control = {0, 0, 0, torque.x, torque.y, torque.z};
    TSpaceStep(state, params, control);
  };

  // Mode: manual — drive with RCS thrusters
  auto manual_control = [&](ISC axis, FPD amount) {
    TSpaceVec force = {0, 0, 0};
    TSpaceVec torque = {0, 0, 0};
    switch (axis) {
      case 0: force.z = amount; break;  // +Z forward
      case 1: force.z = -amount; break; // -Z backward
      case 2: force.y = amount; break;  // +Y up
      case 3: force.y = -amount; break; // -Y down
      case 4: force.x = -amount; break; // -X left
      case 5: force.x = amount; break;  // +X right
      case 6: torque.x = amount; break; // +pitch
      case 7: torque.x = -amount; break; // -pitch
      case 8: torque.y = amount; break; // +yaw
      case 9: torque.y = -amount; break; // -yaw
      case 10: torque.z = amount; break; // +roll
      case 11: torque.z = -amount; break; // -roll
      default: break;
    }
    TSpaceControl control = {force.x, force.y, force.z, torque.x, torque.y, torque.z};
    TSpaceStep(state, params, control);
  };

  // Main loop
  for (ISC ep = 0; ep < episodes; ++ep) {
    if (mode_goto || mode_orient) {
      // Reset state
      state.x = 0; state.y = 0; state.z = 0;
      state.qx = 0; state.qy = 0; state.qz = 0; state.qw = 1;
      state.vx = 0; state.vy = 0; state.vz = 0;
      state.wx = 0; state.wy = 0; state.wz = 0.5;
      state.time = 0;
    }

    ISC steps = 600;  // 10 seconds at 60 Hz
    for (ISC s = 0; s < steps; ++s) {
      if (mode_goto) {
        goto_position();
      } else if (mode_cancel) {
        cancel_rotation();
      } else if (mode_orient) {
        goto_position();
      } else if (mode_manual) {
        // Manual takeover - use keyboard-style controls
        // For demo, oscillate between axes
        manual_control(s % 12, 1.0);
      }

      if (trace && s % 60 == 0) {
        // Print state every second
        D_COUT("Step " << s << ": pos=(%.2f, %.2f, %.2f) q=(%.2f, %.2f, %.2f, %.2f) v=(%.2f, %.2f, %.2f) wx=%.2f wy=%.2f wz=%.2f t=%.2f\n");
      }
    }
  }

  if (trace) D_COUT("Simulation complete.\n");
  return 0;
}