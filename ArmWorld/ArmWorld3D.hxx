// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekArmWorldHXX3D
#define _iGeekArmWorldHXX3D
#include "ArmWorld.h"
#include "ArmPhysics3D.h"
#include "../../CrabsTK/Math/Blas.h"
#include "../CrabsTK/Math/Blas.h"
#if _Seam >= _iGeekArmWorldCore
namespace iGeek {
/* 3D extension of AArmEnv: adds j3 (pitch) and spherical target workspace.
   The 2D core stays; StepControl uses TArmControl3D with tau_3. */
struct TArmControl3D {
  FPD tau_1 = 0, tau_2 = 0, tau_3 = 0;
};

struct TArmState3D {
  FPD j1 = 0, j2 = 0, j3 = 0;     // Absolute joint angles, rad
  FPD w1 = 0, w2 = 0, w3 = 0;     // Angular velocities, rad/s
  FPD time = 0;
  FPD tip_x = 0, tip_y = 0, tip_z = 0;  // End-effector position, m
  FPD reach = 0;             // SquareRoot(tip_x^2 + tip_y^2 + tip_z^2), m
};

struct TArmMetrics3D {
  FPD max_tip_speed = 0, max_reach_error = 0, max_torque_util = 0;
  FPD reach_error_squared = 0;
  IUD frames = 0, physics_steps = 0;
};

enum { ArmRunning3D, ArmReach3D, ArmTimeout3D, ArmInvalid3D };

const CHA* TArmStatusName3D(ISC status) {
  const CHA* names[] = {"running", "reach", "timeout", "invalid"};
  return status >= ArmRunning3D && status <= ArmInvalid3D ? names[status] : "?";
}

static IUD TArmHash3D(IUD x) {
  x ^= x >> 12; x = x * 0x9E3779B97F4A7C15ull; x ^= x >> 25;
  x = x * 0xC2B2AE3D27D4EB4Full; x ^= x >> 27;
  return x;
}

class AArmEnv3D {
 public:
  enum { ObsLength = 9, MaxFrames = 2000 };
  static constexpr FPC GoalRadius = 0.02f;
  AArmEnv3D(TArmParams3D& params, IUD seed = 7);
  void Reset();
  void ResetSeed(IUD seed) { seed_ = seed; Reset(); }
  BOL Done() { return status_ != ArmRunning3D; }
  ISC Status() { return status_; }
  /* Categorical action interface (ArmHold3D..ArmCross3D). */
  FPC Step(ISC action);
  /* Raw actuator interface for learned continuous policies (3D phase). */
  FPC StepControl(TArmControl3D& control);
  void Observe();
  const FPC* Obs() { return obs_; }
  TArmState3D& State() { return arm_; }
  TArmMetrics3D& Metrics() { return metrics_; }
  TArmParams3D& Params() { return params_; }
  FPD TargetX() { return target_x_; }
  FPD TargetY() { return target_y_; }
  FPD TargetZ() { return target_z_; }
  FPD TipError() { return tip_error_; }
 private:
  TArmParams3D params_;
  TArmState3D initial_, arm_;
  TArmMetrics3D metrics_;
  FPC obs_[ObsLength] = {};
  IUD seed_;
  ISC status_ = ArmRunning3D;
  FPD target_x_ = 0, target_y_ = 0, target_z_ = 0;
  FPD prev_error_ = 0, tip_error_ = 0;
  /* Draw the target for the stored seed. */
  void DrawTarget();
};

} //< namespace iGeek
#endif
#endif