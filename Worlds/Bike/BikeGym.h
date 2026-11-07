// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekBikeWorldGymH
#define _iGeekBikeWorldGymH
#include "BikeWorld.h"
#include "../../Gym.h"
#if _Seam >= _iGeekBikeWorldCore
namespace iGeek {
/** Actual iGeek Gym interface, with 1..GymEnvMax independent bike states.
 * Categorical actions select pilot speed commands; this is NOT an end-to-end
 * learned steering policy. ABikeEnv::StepControl exposes raw actuators instead.
 * Terminal rewards/flags survive the immediate seeded auto-reset; the returned
 * observation is the next episode's initial observation, as PPO expects.
 */
class ABikeGym : public Gym
{
 public:
  ABikeGym(TileMap& map, TVehicleRoute& route,
           TBikeState& initial, ISC count = 4, IUD seed = 1);
  ~ABikeGym() override;
  ABikeGym(ABikeGym&) = delete;
  ABikeGym& operator=(ABikeGym&) = delete;
  void ResetBatch() override;
  void StepBatch() override;
  /** @pre i in [0,EnvironmentCount()). */
  ABikeEnv& Bike(ISC i) { return *bikes_[i]; }
  ISC LastStatus(ISC i) {
    return i >= 0 && i < env_count_ ? last_status_[i] : BikeInvalid;
  }
 private:
  ABikeEnv* bikes_[GymEnvMax] = {};
  FPC observation_storage_[GymEnvMax * ABikeEnv::ObservationLength] = {};
  FPC reward_storage_[GymEnvMax] = {}, terminal_storage_[GymEnvMax] = {};
  FPC mask_storage_[GymEnvMax * BikeActionCount] = {};
  ISC action_storage_[GymEnvMax] = {}, last_status_[GymEnvMax] = {};
  IUD episodes_[GymEnvMax] = {}, seed_;
  void CopyObservation(ISC i);
};
} //< namespace iGeek
#endif
#endif
