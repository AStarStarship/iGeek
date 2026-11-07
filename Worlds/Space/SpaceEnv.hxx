// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekSpaceEnvHXX
#define _iGeekSpaceEnvHXX
#include <_Config.h>
#if _Seam >= _iGeekSpaceWorldCore
#include "SpaceEnv.h"
#include "SpaceCaps.h"
#include "SpaceRcs.h"
#include "../../Gym.h"

namespace iGeek {

/* No <cmath> here (no-stdlib): scalar absolute value. */
static FPD AbsFPD_(FPD v) { return v < 0.0 ? -v : v; }

SpaceEnv::SpaceEnv(IUD seed)
    : Environment("spaceworld", 2), steps_(0), done_(false),
      seed_(seed ? seed : IUD(7))
{
  /* The space env IS the Environment: it owns the Room and the manifest a
     joining agent reads. */
  TSPrint(manifest_.name, ISW(ManifestNameLen), "spaceworld");
  TSPrint(manifest_.version, ISW(ManifestVersionLen), "1.0");
  manifest_.kind = ManifestKindEnv;
  manifest_.domain = ManifestDomainRobotics;
  TSPrint(manifest_.description, ISW(ManifestDescriptionLen),
          "Spacecraft 6-DoF attitude hold with RCS pulses");
  TSPrint(manifest_.modality, ISW(ManifestModalityLen), "vector");
  manifest_.reward_low = FPC(-1.0);
  manifest_.reward_high = FPC(1.0);
  manifest_.reward_higher_is_better = true;
  manifest_.determinism = ManifestDeterminismSeeded;
  manifest_.observation_count = ISC(ObservationLength);
  manifest_.action_count = ISC(ActionCount);
  manifest_.max_episode_steps = ISC(MaxSteps);
  manifest_.environment_count_max = ISC(GymEnvMax);
  TSPrint(manifest_.world_name, ISW(ManifestWorldLen), "SpaceWorld");
  manifest_.seam_number = _iGeekSpaceWorldCore;
  manifest_.manifest_version = ISC(1);

  params_.mass = 1000.0;
  TSpaceInertia(params_);
  if (!(params_.Ixx > 0.0)) params_.Ixx = 200.0;
  if (!(params_.Iyy > 0.0)) params_.Iyy = 200.0;
  if (!(params_.Izz > 0.0)) params_.Izz = 200.0;
  Reset();
}

void SpaceEnv::Reset() {
  state_ = TSpaceState();          //< identity attitude, at rest, at origin
  control_ = TSpaceControl();
  /* Seeded initial tumble so episodes differ but stay reproducible. */
  state_.wx = FPD(ISC((seed_ + 1u) % 7u) - 3) * 0.1;
  state_.wy = FPD(ISC((seed_ + 3u) % 5u) - 2) * 0.1;
  state_.wz = FPD(ISC((seed_ + 5u) % 9u) - 4) * 0.1;
  steps_ = 0;
  done_ = false;
  Observe();
}

FPC SpaceEnv::Step(ISC action) {
  ISC a = action;
  if (a < 0) a = 0;
  if (a >= ISC(ActionCount)) a = ISC(ActionCount) - 1;
  control_.fx = control_.fy = control_.fz = 0.0;
  control_.tx = control_.ty = control_.tz = 0.0;
  if (a >= 1 && a <= 6) {
    const ISC idx = a - 1;
    const FPD s = (idx % 2 == 0) ? 1.0 : -1.0;
    const ISC axis = idx / 2;
    if (axis == 0) control_.tx = s * 5.0;
    else if (axis == 1) control_.ty = s * 5.0;
    else control_.tz = s * 5.0;
  } else if (a >= 7 && a <= 12) {
    const ISC idx = a - 7;
    const FPD s = (idx % 2 == 0) ? 1.0 : -1.0;
    const ISC axis = idx / 2;
    if (axis == 0) control_.fx = s * 50.0;
    else if (axis == 1) control_.fy = s * 50.0;
    else control_.fz = s * 50.0;
  }
  TSpaceStep(state_, params_, control_);
  ++steps_;

  const FPD w_rate = AbsFPD_(state_.wx) + AbsFPD_(state_.wy) + AbsFPD_(state_.wz);
  const FPD v_rate = AbsFPD_(state_.vx) + AbsFPD_(state_.vy) + AbsFPD_(state_.vz);
  FPC reward = FPC(-0.01f) - FPC(w_rate * 0.2) - FPC(v_rate * 0.002);
  if (w_rate < 0.05 && v_rate < 0.10) {
    done_ = true;
    reward = FPC(1.0f);          //< at rest: attitude held
  }
  Observe();
  return reward;
}

void SpaceEnv::Observe() {
  observation_[0] = FPC(steps_) / FPC(MaxSteps);
  observation_[1] = FPC(state_.x / 50.0);
  observation_[2] = FPC(state_.y / 50.0);
  observation_[3] = FPC(state_.z / 50.0);
  observation_[4] = FPC(state_.vx / 10.0);
  observation_[5] = FPC(state_.vy / 10.0);
  observation_[6] = FPC(state_.vz / 10.0);
  observation_[7] = FPC(state_.wx / 5.0);
  observation_[8] = FPC(state_.wy / 5.0);
  observation_[9] = FPC(state_.wz / 5.0);
  observation_[10] = FPC(state_.qx);
  observation_[11] = FPC(state_.qy);
  observation_[12] = FPC(state_.qz);
}

}  //< namespace iGeek
#endif  //< _Seam >= _iGeekSpaceWorldCore
#endif  //< _iGeekSpaceEnvHXX
