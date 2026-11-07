// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekArmWorldHXX
#define _iGeekArmWorldHXX
#include "ArmWorld.h"
#include "ArmPhysics.hxx"
#include "../../Gym.h"
#include "../../CrabsTK/Math/Blas.h"
#if _Seam >= _iGeekArmWorldCore
namespace iGeek {
using namespace ::_;
using namespace ::CTK;

const CHA* TArmStatusName(ISC status) {
  const CHA* names[] = {"running", "reach", "timeout", "invalid"};
  return status >= ArmRunning && status <= ArmInvalid ? names[status] : "?";
}
/* Deterministic splitmix64-style hash for target drawing. */
static IUD TArmHash(IUD x) {
  x ^= x >> 12; x = x * 0x9E3779B97F4A7C15ull; x ^= x >> 25;
  x = x * 0xC2B2AE3D27D4EB4Full; x ^= x >> 27;
  return x;
}
AArmEnv::AArmEnv(ArmParams& params, IUD seed)
    : Environment("armworld", 2), params_(params), seed_(seed) {
  /* The arm env IS the Environment, so it owns the Room and the manifest that
     a joining agent reads. The Gym only batches it. */
  TSPrint(manifest_.name, ISW(ManifestNameLen), "armworld");
  TSPrint(manifest_.version, ISW(ManifestVersionLen), "1.0");
  manifest_.kind = ManifestKindEnv;
  manifest_.domain = ManifestDomainRobotics;
  TSPrint(manifest_.description, ISW(ManifestDescriptionLen),
          "2-link arm reacher with joint torque control");
  TSPrint(manifest_.modality, ISW(ManifestModalityLen), "vector");
  manifest_.reward_low = FPC(-1.0);
  manifest_.reward_high = FPC(1.0);
  manifest_.reward_higher_is_better = true;
  manifest_.determinism = ManifestDeterminismSeeded;
  manifest_.observation_count = ISC(ObservationLength);
  manifest_.action_count = ISC(ArmActionCount);
  manifest_.max_episode_steps = ISC(MaxFrames);
  manifest_.environment_count_max = ISC(GymEnvMax);
  TSPrint(manifest_.world_name, ISW(ManifestWorldLen), "ArmWorld");
  manifest_.seam_number = _iGeekArmWorldCore;
  manifest_.manifest_version = ISC(1);
  Reset();
}
void AArmEnv::DrawTarget() {
  IUD h = TArmHash(seed_ + 0x5851F42D4C957F2Dull);
  // Angle in [25 deg, 65 deg], radius in [0.30, 0.32] m — the reachable
  // annulus for j∈[0,π/2] with good coverage (verified 2026-10-20).
  FPD angle = 0.4363323129985824 + FPD(h % 41) / 40 * 0.6981317007977318;  // rad (25..65 deg)
  FPD radius = 0.30 + FPD((h >> 16) % 21) / 20 * 0.02;
  target_x_ = radius * Cosine(angle);
  target_y_ = radius * Sine(angle);
}
void AArmEnv::Reset() {
  arm_ = initial_;
  metrics_ = {};
  status_ = ArmRunning;
  if (!ArmParamsValid(params_) || !ArmStateValid(initial_)) {
    status_ = ArmInvalid;
    Observe();
    return;
  }
  DrawTarget();
  tip_error_ = hypot(arm_.tip_x - target_x_, arm_.tip_y - target_y_);
  prev_error_ = tip_error_;
  Observe();
}
void AArmEnv::Observe() {
  for (ISC i = 0; i < ObservationLength; ++i) observation_[i] = 0;
  if (status_ == ArmInvalid) return;
  observation_[0] = FPC(target_x_ / 0.5);
  observation_[1] = FPC(target_y_ / 0.5);
  observation_[2] = FPC(arm_.j1 / 1.5707963267948966);
  observation_[3] = FPC(arm_.j2 / 1.5707963267948966);
  observation_[4] = FPC(arm_.w1 / 5);
  observation_[5] = FPC(arm_.w2 / 5);
}
FPC AArmEnv::Step(ISC action) {
  if (Done()) return 0;
  if (action < ArmHold || action >= ArmActionCount) {
    status_ = ArmInvalid;
    Observe();
    return -10;
  }
  ArmControl control;
  FPD tau = 0.7 * params_.max_torque;
  switch (action) {
    case ArmHold: control.tau_1 = 0; control.tau_2 = 0; break;
    case ArmInc1: control.tau_1 = tau; break;
    case ArmDec1: control.tau_1 = -tau; break;
    case ArmInc2: control.tau_2 = tau; break;
    case ArmDec2: control.tau_2 = -tau; break;
    case ArmInc12: control.tau_1 = tau; control.tau_2 = tau; break;
    case ArmDec12: control.tau_1 = -tau; control.tau_2 = -tau; break;
    case ArmCross: control.tau_1 = tau; control.tau_2 = -tau; break;
  }
  return StepControl(control);
}
FPC AArmEnv::StepControl(ArmControl& control) {
  if (Done()) return 0;
  if (!ArmParamsValid(params_) || !iGeek::IsFinite(control.tau_1) ||
      !iGeek::IsFinite(control.tau_2)) {
    status_ = ArmInvalid;
    Observe();
    return -10;
  }
  FPD old_tip_x = arm_.tip_x, old_tip_y = arm_.tip_y;
  if (!ArmStep(arm_, params_, control)) {
    status_ = ArmInvalid;
    Observe();
    return -10;
  }
  ++metrics_.frames;
  metrics_.physics_steps += 4;
  FPD tip_speed = hypot(arm_.tip_x - old_tip_x, arm_.tip_y - old_tip_y) /
      params_.dt;
  metrics_.max_tip_speed = tip_speed > metrics_.max_tip_speed
      ? tip_speed : metrics_.max_tip_speed;
  tip_error_ = hypot(arm_.tip_x - target_x_, arm_.tip_y - target_y_);
  metrics_.reach_error_squared += tip_error_ * tip_error_;
  FPD util = Absolute(control.tau_1) + Absolute(control.tau_2);
  util /= 2 * params_.max_torque;
  metrics_.max_torque_util = util > metrics_.max_torque_util ? util : metrics_.max_torque_util;
  FPD reward;
  if (tip_error_ <= GoalRadius) {
    status_ = ArmReach;
    reward = 10;
  } else if (tip_error_ >= prev_error_) {
    reward = -1;
  } else {
    reward = 0;
  }
  prev_error_ = tip_error_;
  if (!Done() && metrics_.frames >= MaxFrames) status_ = ArmTimeout;
  Observe();
  return FPC(reward);
}
}       //< namespace iGeek
#endif  //< _Seam >= _iGeekArmWorldCore
#endif  //< _iGeekArmWorldHXX
