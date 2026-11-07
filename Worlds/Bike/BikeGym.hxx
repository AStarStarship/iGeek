// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekBikeWorldGymHXX
#define _iGeekBikeWorldGymHXX
#include <_Config.h>
#include "BikeGym.h"
#include "BikeWorld.hxx"
#include "../../Gym.hxx"
#if _Seam >= _iGeekBikeWorldCore
namespace iGeek {
ABikeGym::ABikeGym(TileMap& map, TVehicleRoute& route,
    TBikeState& initial, ISC count, IUD seed)
    : Gym("bikeworld"), seed_(seed) {
  env_count_ = count < 1 ? 1 : count > GymEnvMax ? GymEnvMax : count;
  observation_len_ = ABikeEnv::ObservationLength; action_count_ = BikeActionCount;
  observations_ = observation_storage_; actions_ = action_storage_;
  rewards_ = reward_storage_; terminals_ = terminal_storage_;
  action_mask_ = mask_storage_;
  for (ISC i = 0; i < env_count_; ++i) {
    bikes_[i] = new ABikeEnv(map, route, initial, seed_ + IUD(i) * 0x9E37u);
    envs_[i] = bikes_[i];   //< the bikes ARE the Environments.
  }
  ResetBatch();
}
ABikeGym::~ABikeGym() {
  for (ISC i = 0; i < env_count_; ++i) delete bikes_[i];
}
void ABikeGym::CopyObservation(ISC i) {
  for (ISC j = 0; j < observation_len_; ++j)
    observations_[i * observation_len_ + j] = bikes_[i]->Observation()[j];
  for (ISC a = 0; a < action_count_; ++a)
    action_mask_[i * action_count_ + a] =
        !bikes_[i]->Done() || a == BikeStop ? 1 : 0;
}
void ABikeGym::ResetBatch() {
  for (ISC i = 0; i < env_count_; ++i) {
    episodes_[i] = 0; last_status_[i] = BikeRunning;
    bikes_[i]->ResetSeed(seed_ + IUD(i) * 0x9E37u);
    actions_[i] = BikeCruise; rewards_[i] = 0;
    terminals_[i] = bikes_[i]->Done() ? 1 : 0;
    CopyObservation(i);
  }
}
void ABikeGym::StepBatch() {
  for (ISC i = 0; i < env_count_; ++i) {
    rewards_[i] = bikes_[i]->Step(actions_[i]);
    terminals_[i] = bikes_[i]->Done() ? 1 : 0;
    last_status_[i] = bikes_[i]->Status();
    if (bikes_[i]->Done()) {
      ++episodes_[i];
      bikes_[i]->ResetSeed(seed_ + IUD(i) * 0x9E37u + episodes_[i] * 0x10001u);
    }
    CopyObservation(i);
  }
}
} //< namespace iGeek
#endif
#endif
