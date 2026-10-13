// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_BIKEWORLD_H
#define CRABS_BIKEWORLD_H
#include "BikePilot.h"
#if SEAM >= CRABS_BIKEWORLD
namespace _ {
enum {
  BikeRunning, BikeGoal, BikeCollision, BikeFallen, BikeTimeout,
  BikeInvalid, BikeBlocked, BikeStopped
};
enum { BikeCruise, BikeSlow, BikeFast, BikeStop, BikeCrawl, BikeActionCount };
const CHA* TBikeStatusName(ISC status);
struct TBikeMetrics
{
  FPD max_roll = 0, max_speed = 0, max_cross_track = 0;
  FPD cross_track_squared = 0;
  IUD frames = 0, physics_steps = 0;
};
/** Native continuous bicycle environment. Map and route must outlive this
 * environment and must not be mutated concurrently with stepping.
 * Reset uses the stored seed exactly; it does not silently advance an RNG.
 * Step is the classical pilot's speed-action interface. StepControl is the
 * unassisted actuator interface for future learned policies/physics tests.
 */
class ABikeEnv
{
 public:
  enum { ObsLength = 21, MaxFrames = 6000 };
  ABikeEnv(const TileMap& map, const TVehicleRoute& route,
           const TBikeState& initial, IUD seed = 1);
  void Reset();
  void ResetSeed(IUD seed) { seed_ = seed; Reset(); }
  BOL Done() const { return status_ != BikeRunning; }
  ISC Status() const { return status_; }
  FPC Step(ISC action = BikeCruise);
  FPC StepControl(const TBikeControl& control);
  void Observe();
  const FPC* Obs() const { return obs_; }
  const TBikeState& State() const { return bike_; }
  const TBikeMetrics& Metrics() const { return metrics_; }
  TBikeParams& Params() { return params_; }
  FPD GoalDistance() const;
 private:
  const TileMap& map_;
  const TVehicleRoute& route_;
  TBikeState initial_, bike_;
  TBikeParams params_;
  ABikePilot pilot_;
  TBikeMetrics metrics_;
  FPC obs_[ObsLength] = {};
  IUD seed_;
  ISC status_ = BikeRunning;
};
}  //< namespace _
#endif
#endif
