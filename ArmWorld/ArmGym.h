// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_ARMWORLD_GYM_H
#define CRABS_ARMWORLD_GYM_H
#include "ArmWorld.h"
#include "../Gym.h"
#if SEAM >= CRABS_ARMWORLD
namespace iGeek {
/* Real native Gym buffers for the 2-link reacher. Categorical actions (8) drive joint torques. Terminal rewards/flags survive the seeded auto-reset; the returned observation is the next episode's. */
class AArmGym : public Gym {
 public:
  AArmGym(const TArmParams& params, ISC count = 4, IUD seed = 7);
  ~AArmGym() override;
  AArmGym(const AArmGym&) = delete;
  AArmGym& operator=(const AArmGym&) = delete;
  void ResetBatch() override;
  void StepBatch() override;
  AArmEnv& Arm(ISC i) { return *arms_[i]; }  //< @pre i in [0,EnvCount()).
  ISC ActionCount() const { return action_count_; }
  ISC LastStatus(ISC i) const {
    return i >= 0 && i < env_count_ ? last_status_[i] : ArmInvalid;
  }
 private:
  AArmEnv* arms_[GymEnvMax] = {};
  FPC obs_storage_[GymEnvMax * AArmEnv::ObsLength] = {};
  FPC reward_storage_[GymEnvMax] = {}, terminal_storage_[GymEnvMax] = {};
  FPC mask_storage_[GymEnvMax * ArmActionCount] = {};
  ISC action_storage_[GymEnvMax] = {}, last_status_[GymEnvMax] = {};
  IUD episodes_[GymEnvMax] = {}, seed_;
  void CopyObservation(ISC i);
};
} //< namespace iGeek
#endif
#endif
