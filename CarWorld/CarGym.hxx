// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekCarWorldCarGymHXX
#define _iGeekCarWorldCarGymHXX
#include <_Config.h>
#include "CarGym.h"
#include "CarWorld2D.hxx"
#include "../Gym.hxx"
#if _Seam >= _iGeekCarWorldCore
namespace iGeek {
using namespace ::_;

ACarGym::ACarGym(TileMap& map, TVehicleRoute& route,
    TCarState& initial, ISC count, IUD seed, ISC mode)
    : Gym("carworld"), seed_(seed), Mode(mode == CarActuatorActions ? mode : CarDriverActions) {
  env_count_ = count < 1 ? 1 : count > GymEnvMax ? GymEnvMax : count;
  obs_len_ = ACarEnv::ObsLength;
  action_count_ = Mode == CarActuatorActions ? ISC(CarRawActionCount) : ISC(CarActionCount);
  observations_ = obs_storage_; actions_ = action_storage_;
  rewards_ = reward_storage_; terminals_ = terminal_storage_; action_mask_ = mask_storage_;
  for (ISC i = 0; i < env_count_; ++i) {
    cars_[i] = new ACarEnv(map, route, initial, seed_ + IUD(i) * 0x9E37u);
    envs_[i] = _NilP;
  }
  TSPrint(manifest_.name, ISW(ManifestNameLen), "carworld");
  TSPrint(manifest_.version, ISW(ManifestVersionLen), "1.0");
  manifest_.kind = ManifestKindEnv;
  manifest_.domain = ManifestDomainRobotics;
  TSPrint(manifest_.description, ISW(ManifestDescriptionLen),
          "2D car driving with classical driver and MPC");
  TSPrint(manifest_.modality, ISW(ManifestModalityLen), "vector");
  manifest_.reward_low = FPC(-1.0);
  manifest_.reward_high = FPC(1.0);
  manifest_.reward_higher_is_better = true;
  manifest_.determinism = ManifestDeterminismSeeded;
  manifest_.obs_count = obs_len_;
  manifest_.action_count = action_count_;
  manifest_.max_episode_steps = ACarEnv::MaxFrames;
  manifest_.environment_count_max = GymEnvMax;
  TSPrint(manifest_.world_name, ISW(ManifestWorldLen), "CarWorld");
  manifest_.seam_number = _iGeekCarWorldCore;
  manifest_.manifest_version = ISC(1);
  ResetBatch();
}
ACarGym::~ACarGym() {
  for (ISC i = 0; i < env_count_; ++i) delete cars_[i];
}
void ACarGym::CopyObservation(ISC i) {
  for (ISC j = 0; j < obs_len_; ++j)
    observations_[i * obs_len_ + j] = cars_[i]->Obs()[j];
  for (ISC a = 0; a < action_count_; ++a)
    action_mask_[i * action_count_ + a] = !cars_[i]->Done() ||
        a == (Mode == CarActuatorActions ? 7 : CarStop) ? 1 : 0;
}
void ACarGym::ResetBatch() {
  for (ISC i = 0; i < env_count_; ++i) {
    episodes_[i] = 0;
    cars_[i]->ResetSeed(seed_ + IUD(i) * 0x9E37u);
    last_status_[i] = cars_[i]->Status();
    actions_[i] = Mode == CarActuatorActions ? 1 : CarCruise;
    rewards_[i] = 0; terminals_[i] = cars_[i]->Done() ? 1 : 0;
    CopyObservation(i);
  }
}
void ACarGym::StepBatch() {
  for (ISC i = 0; i < env_count_; ++i) {
    ISC action = actions_[i];
    if (Mode == CarActuatorActions && action >= 0 && action < CarRawActionCount) {
      TCarControl control;
      control.steer = FPD(action % 3 - 1) * 0.18;
      if (action / 3 == 0) control.throttle = 0.35;
      if (action / 3 == 2) control.brake = 0.6;
      rewards_[i] = cars_[i]->StepControl(control);
    } else {
      rewards_[i] = cars_[i]->Step(Mode == CarActuatorActions ? -1 : action);
    }
    terminals_[i] = cars_[i]->Done() ? 1 : 0;
    last_status_[i] = cars_[i]->Status();
    if (cars_[i]->Done()) {
      ++episodes_[i];
      cars_[i]->ResetSeed(seed_ + IUD(i) * 0x9E37u + episodes_[i] * 0x10001u);
    }
    CopyObservation(i);
  }
}

}       //< namespace iGeek
#endif  //< _Seam >= _iGeekCarWorldCore
#endif  //< _iGeekCarWorldCarGymHXX
