// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekArmWorldH
#define _iGeekArmWorldH
#include "ArmPhysics.h"
#if _Seam >= _iGeekArmWorldCore
namespace iGeek {
enum { ArmRunning, ArmReach, ArmTimeout, ArmInvalid };
const CHA* TArmStatusName(ISC status);
/*Discrete actions (torque command per joint):
0 hold, 1 +j1, 2 -j1, 3 +j2, 4 -j2, 5 +j1+j2, 6 -j1-j2, 7 +j1-j2.
Torque magnitude: 0.7 * max_torque per joint. */
enum { ArmHold, ArmInc1, ArmDec1, ArmInc2, ArmDec2,
       ArmInc12, ArmDec12, ArmCross, ArmActionCount = 8 };
struct TArmMetrics
{
  FPD max_tip_speed = 0, max_reach_error = 0, max_torque_util = 0;
  FPD reach_error_squared = 0;
  IUD frames = 0, physics_steps = 0;
};
/* Two-link planar reacher environment. Deterministic per seed:
Reset draws the target from the seed; Step advances physics.
Target is inside the joint-bounded quarter workspace (r in [0.25, 0.38],
angle in [15 deg, 75 deg]) so the task is always solvable.  */
class AArmEnv
{
 public:
  enum { ObsLength = 6, MaxFrames = 2000 };
  static constexpr FPC GoalRadius = 0.02f;
  AArmEnv(TArmParams& params, IUD seed = 7);
  void Reset();
  void ResetSeed(IUD seed) { seed_ = seed; Reset(); }
  BOL Done() { return status_ != ArmRunning; }
  ISC Status() { return status_; }
  /*Categorical action interface (ArmHold..ArmCross). */
  FPC Step(ISC action);
  /*Raw actuator interface for learned continuous policies (3D phase). */
  FPC StepControl(TArmControl& control);
  void Observe();
  const FPC* Obs() { return obs_; }
  TArmState& State() { return arm_; }
  TArmMetrics& Metrics() { return metrics_; }
  TArmParams& Params() { return params_; }
  FPD TargetX() { return target_x_; }
  FPD TargetY() { return target_y_; }
  FPD TipError() { return tip_error_; }
 private:
  TArmParams params_;
  TArmState initial_, arm_;
  TArmMetrics metrics_;
  FPC obs_[ObsLength] = {};
  IUD seed_;
  ISC status_ = ArmRunning;
  FPD target_x_ = 0, target_y_ = 0;
  FPD prev_error_ = 0, tip_error_ = 0;
  /*Draw the target for the stored seed. */
  void DrawTarget();
};
} //< namespace iGeek
#endif
#endif
