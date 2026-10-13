// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_CARWORLD_PILOT_H
#define CRABS_CARWORLD_PILOT_H
#include "CarPhysics.h"
#include "../BikeWorld/VehicleRoute.h"
#if SEAM >= CRABS_CARWORLD
namespace _ {
/** Classical reference driver. Ideal state/map, no neural policy or state edits. */
class ACarPilot
{
 public:
  void Reset() { target_speed_ = 0; guide_ = {}; }
  TCarControl Control(const TileMap& map, const TVehicleRoute& route,
      const TCarState& car, const TCarParams& params, FPD cruise_speed);
  FPD TargetSpeed() const { return target_speed_; }
 private:
  FPD target_speed_ = 0;
  TVehicleGuide guide_;
};
}  //< namespace _
#endif
#endif
