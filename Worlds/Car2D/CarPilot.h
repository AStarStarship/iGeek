// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekCarWorldPilotH
#define _iGeekCarWorldPilotH
#include "CarPhysics.h"
#include "../../Worlds/Bike/VehicleRoute.h"
#if _Seam >= _iGeekCarWorldCore
namespace iGeek {
/** Classical reference driver. Ideal state/map, no neural policy or state edits. */
class ACarPilot
{
 public:
  void Reset() { target_speed_ = 0; guide_ = {}; }
  TCarControl Control(TileMap& map, TVehicleRoute& route,
      TCarState& car, TCarParams& params, FPD cruise_speed);
  FPD TargetSpeed() { return target_speed_; }
 private:
  FPD target_speed_ = 0;
  TVehicleGuide guide_;
};
} //< namespace iGeek
#endif
#endif
