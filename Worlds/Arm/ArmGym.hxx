// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekArmWorldGymHXX
#define _iGeekArmWorldGymHXX
#include <_Config.h>
#include "ArmGym.h"
#include "ArmWorld.hxx"
#include "../../Gym.hxx"
#if _Seam >= _iGeekArmWorldCore
namespace iGeek {
using namespace ::_;
using namespace ::CTK;

AArmGym::AArmGym(ArmParams& params, ISC count, IUD seed)
    : Gym("armworld"), seed_(seed) {
  env_count_ = count < 1 ? 1 : count > GymEnvMax ? GymEnvMax : count;
  observation_len_ = AArmEnv::ObservationLength;
  action_count_ = ArmActionCount;
  observations_ = observation_storage_; actions_ = action_storage_;
  rewards_ = reward_storage_; terminals_ = terminal_storage_;
  action_mask_ = mask_storage_;
  for (ISC i = 0; i < env_count_; ++i) {
    arms_[i] = new AArmEnv(params, seed_ + IUD(i) * 0x9E37u);
    envs_[i] = arms_[i];   //< the arms ARE the Environments this Gym batches.
  }
  ResetBatch();
}
AArmGym::~AArmGym() {
  for (ISC i = 0; i < env_count_; ++i) delete arms_[i];
}
void AArmGym::CopyObservation(ISC i) {
  for (ISC j = 0; j < observation_len_; ++j)
    observations_[i * observation_len_ + j] = arms_[i]->Observation()[j];
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
