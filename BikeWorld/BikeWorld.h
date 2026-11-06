// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekBikeWorldH
#define _iGeekBikeWorldH
#include "BikePilot.h"
#if _Seam >= _iGeekBikeWorldCore
namespace iGeek {
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
  ABikeEnv(TileMap& map, TVehicleRoute& route,
           TBikeState& initial, IUD seed = 1);
  void Reset();
  void ResetSeed(IUD seed) { seed_ = seed; Reset(); }
  BOL Done() { return status_ != BikeRunning; }
  ISC Status() { return status_; }
  FPC Step(ISC action = BikeCruise);
  FPC StepControl(TBikeControl& control);
  void Observe();
  const FPC* Obs() { return obs_; }
  TBikeState& State() { return bike_; }
  TBikeMetrics& Metrics() { return metrics_; }
  TBikeParams& Params() { return params_; }
  FPD GoalDistance();
 private:
  TileMap& map_;
  TVehicleRoute& route_;
  TBikeState initial_, bike_;
  TBikeParams params_;
  ABikePilot pilot_;
  TBikeMetrics metrics_;
  FPC obs_[ObsLength] = {};
  IUD seed_;
  ISC status_ = BikeRunning;
};
} //< namespace iGeek
#endif
#endif
