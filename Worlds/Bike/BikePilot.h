// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekBikeWorldPilotH
#define _iGeekBikeWorldPilotH
#include "BikePhysics.h"
#include "VehicleRoute.h"
#if _Seam >= _iGeekBikeWorldCore
namespace iGeek {
/** Classical baseline controller, not a neural or LLM policy. Does not modify
 * vehicle state. Roll stabilization acts only through bounded steering.
 * Global-map routing and ideal contact state are explicit simulator inputs.
 */
class ABikePilot
{
 public:
  void Reset() { roll_target_ = 0; guide_ = {}; target_speed_ = 0; }
  TBikeControl Control(TileMap& map, TVehicleRoute& route,
      TBikeState& bike, TBikeParams& params,
      FPD cruise_speed, FPD dt);
  TVehicleGuide& Guidance() { return guide_; }
  FPD TargetSpeed() { return target_speed_; }
 private:
  FPD roll_target_ = 0, target_speed_ = 0;
  TVehicleGuide guide_;
};
} //< namespace iGeek
#endif
#endif
