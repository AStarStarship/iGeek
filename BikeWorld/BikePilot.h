// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_BIKEWORLD_PILOT_H
#define CRABS_BIKEWORLD_PILOT_H
#include "BikePhysics.h"
#include "VehicleRoute.h"
#if SEAM >= CRABS_BIKEWORLD
namespace iGeek {
/** Classical baseline controller, not a neural or LLM policy. Does not modify
 * vehicle state. Roll stabilization acts only through bounded steering.
 * Global-map routing and ideal contact state are explicit simulator inputs.
 */
class ABikePilot
{
 public:
  void Reset() { roll_target_ = 0; guide_ = {}; target_speed_ = 0; }
  TBikeControl Control(const TileMap& map, const TVehicleRoute& route,
      const TBikeState& bike, const TBikeParams& params,
      FPD cruise_speed, FPD dt);
  const TVehicleGuide& Guidance() const { return guide_; }
  FPD TargetSpeed() const { return target_speed_; }
 private:
  FPD roll_target_ = 0, target_speed_ = 0;
  TVehicleGuide guide_;
};
} //< namespace iGeek
#endif
#endif
