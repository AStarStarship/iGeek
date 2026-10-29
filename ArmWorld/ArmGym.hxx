// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_ARMWORLD_GYM_HXX
#define CRABS_ARMWORLD_GYM_HXX
#include "ArmGym.h"
#include "ArmWorld.hxx"
#include "../Gym.hxx"
#if SEAM >= CRABS_ARMWORLD
namespace iGeek {
AArmGym::AArmGym(const TArmParams& params, ISC count, IUD seed)
    : Gym("armworld"), seed_(seed) {
  env_count_ = count < 1 ? 1 : count > GymEnvMax ? GymEnvMax : count;
  obs_len_ = AArmEnv::ObsLength;
  action_count_ = ArmActionCount;
  observations_ = obs_storage_; actions_ = action_storage_;
  rewards_ = reward_storage_; terminals_ = terminal_storage_;
  action_mask_ = mask_storage_;
  for (ISC i = 0; i < env_count_; ++i) {
    arms_[i] = new AArmEnv(params, seed_ + IUD(i) * 0x9E37u);
    envs_[i] = NILP;
  }
  CopyStr(manifest_.name, ManifestNameLen, "armworld");
  CopyStr(manifest_.version, ManifestVersionLen, "1.0");
  manifest_.kind = ManifestKindEnv;
  manifest_.domain = ManifestDomainRobotics;
  CopyStr(manifest_.description, ManifestDescriptionLen,
          "2-link arm reacher with joint torque control");
  CopyStr(manifest_.modality, ManifestModalityLen, "vector");
  manifest_.reward_low = FPC(-1.0);
  manifest_.reward_high = FPC(1.0);
  manifest_.reward_higher_is_better = true;
  manifest_.determinism = ManifestDeterminismSeeded;
  manifest_.obs_count = obs_len_;
  manifest_.action_count = action_count_;
  manifest_.max_episode_steps = AArmEnv::MaxFrames;
  manifest_.env_count_max = GymEnvMax;
  CopyStr(manifest_.world_name, ManifestWorldLen, "ArmWorld");
  manifest_.seam_number = IGEEK_ARMWORLD_CORE;
  manifest_.manifest_version = ISC(1);
  ResetBatch();
}
AArmGym::~AArmGym() {
  for (ISC i = 0; i < env_count_; ++i) delete arms_[i];
}
void AArmGym::CopyObservation(ISC i) {
  for (ISC j = 0; j < obs_len_; ++j)
    observations_[i * obs_len_ + j] = arms_[i]->Obs()[j];
  for (ISC a = 0; a < action_count_; ++a)
    action_mask_[i * action_count_ + a] = !arms_[i]->Done() || a == ArmHold ? 1 : 0;
}
void AArmGym::ResetBatch() {
  for (ISC i = 0; i < env_count_; ++i) {
    episodes_[i] = 0;
    arms_[i]->ResetSeed(seed_ + IUD(i) * 0x9E37u);
    last_status_[i] = arms_[i]->Status();
    actions_[i] = ArmHold;
    rewards_[i] = 0; terminals_[i] = arms_[i]->Done() ? 1 : 0;
    CopyObservation(i);
  }
}
void AArmGym::StepBatch() {
  for (ISC i = 0; i < env_count_; ++i) {
    ISC action = actions_[i];
    rewards_[i] = arms_[i]->Step(action);
    terminals_[i] = arms_[i]->Done() ? 1 : 0;
    last_status_[i] = arms_[i]->Status();
    if (arms_[i]->Done()) {
      ++episodes_[i];
      arms_[i]->ResetSeed(seed_ + IUD(i) * 0x9E37u + episodes_[i] * 0x10001u);
    }
    CopyObservation(i);
  }
}
} //< namespace iGeek
#endif
#endif
