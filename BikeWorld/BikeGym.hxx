// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_BIKEWORLD_GYM_HXX
#define CRABS_BIKEWORLD_GYM_HXX
#include "BikeGym.h"
#include "BikeWorld.hxx"
#include "../Gym.hxx"
#if SEAM >= CRABS_BIKEWORLD
namespace iGeek {
ABikeGym::ABikeGym(const TileMap& map, const TVehicleRoute& route,
    const TBikeState& initial, ISC count, IUD seed)
    : Gym("bikeworld"), seed_(seed) {
  env_count_ = count < 1 ? 1 : count > GymEnvMax ? GymEnvMax : count;
  obs_len_ = ABikeEnv::ObsLength; action_count_ = BikeActionCount;
  observations_ = obs_storage_; actions_ = action_storage_;
  rewards_ = reward_storage_; terminals_ = terminal_storage_;
  action_mask_ = mask_storage_;
  for (ISC i = 0; i < env_count_; ++i) {
    bikes_[i] = new ABikeEnv(map, route, initial, seed_ + IUD(i) * 0x9E37u);
    envs_[i] = NILP;
  }
  CopyStr(manifest_.name, ManifestNameLen, "bikeworld");
  CopyStr(manifest_.version, ManifestVersionLen, "1.0");
  manifest_.kind = ManifestKindEnv;
  manifest_.domain = ManifestDomainRobotics;
  CopyStr(manifest_.description, ManifestDescriptionLen,
          "Bicycle riding with speed command control");
  CopyStr(manifest_.modality, ManifestModalityLen, "vector");
  manifest_.reward_low = FPC(-1.0);
  manifest_.reward_high = FPC(1.0);
  manifest_.reward_higher_is_better = true;
  manifest_.determinism = ManifestDeterminismSeeded;
  manifest_.obs_count = obs_len_;
  manifest_.action_count = action_count_;
  manifest_.max_episode_steps = ABikeEnv::MaxFrames;
  manifest_.env_count_max = GymEnvMax;
  CopyStr(manifest_.world_name, ManifestWorldLen, "BikeWorld");
  manifest_.seam_number = IGEEK_BIKEWORLD_CORE;
  manifest_.manifest_version = ISC(1);
  ResetBatch();
}
ABikeGym::~ABikeGym() {
  for (ISC i = 0; i < env_count_; ++i) delete bikes_[i];
}
void ABikeGym::CopyObservation(ISC i) {
  for (ISC j = 0; j < obs_len_; ++j)
    observations_[i * obs_len_ + j] = bikes_[i]->Obs()[j];
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
