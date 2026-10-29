// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_BIKEWORLD_GYM_H
#define CRABS_BIKEWORLD_GYM_H
#include "BikeWorld.h"
#include "../Gym.h"
#if SEAM >= CRABS_BIKEWORLD
namespace iGeek {
/** Actual iGeek Gym interface, with 1..GymEnvMax independent bike states.
 * Categorical actions select pilot speed commands; this is NOT an end-to-end
 * learned steering policy. ABikeEnv::StepControl exposes raw actuators instead.
 * Terminal rewards/flags survive the immediate seeded auto-reset; the returned
 * observation is the next episode's initial observation, as TPPO expects.
 */
class ABikeGym : public Gym
{
 public:
  ABikeGym(const TileMap& map, const TVehicleRoute& route,
           const TBikeState& initial, ISC count = 4, IUD seed = 1);
  ~ABikeGym() override;
  ABikeGym(const ABikeGym&) = delete;
  ABikeGym& operator=(const ABikeGym&) = delete;
  void ResetBatch() override;
  void StepBatch() override;
  /** @pre i in [0,EnvCount()). */
  ABikeEnv& Bike(ISC i) { return *bikes_[i]; }
  ISC LastStatus(ISC i) const {
    return i >= 0 && i < env_count_ ? last_status_[i] : BikeInvalid;
  }
 private:
  ABikeEnv* bikes_[GymEnvMax] = {};
  FPC obs_storage_[GymEnvMax * ABikeEnv::ObsLength] = {};
  FPC reward_storage_[GymEnvMax] = {}, terminal_storage_[GymEnvMax] = {};
  FPC mask_storage_[GymEnvMax * BikeActionCount] = {};
  ISC action_storage_[GymEnvMax] = {}, last_status_[GymEnvMax] = {};
  IUD episodes_[GymEnvMax] = {}, seed_;
  void CopyObservation(ISC i);
};
} //< namespace iGeek
#endif
#endif
