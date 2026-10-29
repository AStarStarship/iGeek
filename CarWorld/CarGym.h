// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_CARWORLD_GYM_H
#define CRABS_CARWORLD_GYM_H
#include "CarWorld2D.h"
#include "../Gym.h"
#if SEAM >= CRABS_CARWORLD
namespace iGeek {
enum { CarDriverActions, CarActuatorActions, CarRawActionCount = 9 };
/** Real native Gym buffers. Default five actions select classical-driver speed.
 * Optional nine raw actions bypass the driver: three steer targets crossed
 * with throttle/coast/brake. Both preserve terminal rewards through auto-reset.
 */
class ACarGym : public Gym
{
 public:
  ACarGym(const TileMap& map, const TVehicleRoute& route,
          const TCarState& initial, ISC count = 4, IUD seed = 7,
          ISC mode = CarDriverActions);
  ~ACarGym() override;
  ACarGym(const ACarGym&) = delete;
  ACarGym& operator=(const ACarGym&) = delete;
  void ResetBatch() override;
  void StepBatch() override;
  ACarEnv& Car(ISC i) { return *cars_[i]; }  //< @pre i in [0,EnvCount()).
  ISC ActionCount() const { return action_count_; }
  ISC LastStatus(ISC i) const {
    return i >= 0 && i < env_count_ ? last_status_[i] : CarInvalid;
  }
 private:
  ACarEnv* cars_[GymEnvMax] = {};
  FPC obs_storage_[GymEnvMax * ACarEnv::ObsLength] = {};
  FPC reward_storage_[GymEnvMax] = {}, terminal_storage_[GymEnvMax] = {};
  FPC mask_storage_[GymEnvMax * CarRawActionCount] = {};
  ISC action_storage_[GymEnvMax] = {}, last_status_[GymEnvMax] = {};
  IUD episodes_[GymEnvMax] = {}, seed_;
  const ISC Mode;
  void CopyObservation(ISC i);
};
} //< namespace iGeek
#endif
#endif
