// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_CARWORLD_2D_H
#define CRABS_CARWORLD_2D_H
#include "CarPilot.h"
#if SEAM >= CRABS_CARWORLD
namespace iGeek {
enum { CarRunning, CarGoal, CarCollision, CarTimeout, CarInvalid, CarBlocked, CarStopped };
enum { CarCruise, CarSlow, CarFast, CarStop, CarCrawl, CarActionCount };
const CHA* TCarStatusName(ISC status);
struct TCarMetrics
{
  FPD max_speed = 0, max_cross_track = 0, max_lateral_accel = 0;
  FPD max_utilization = 0, cross_track_squared = 0;
  IUD frames = 0, physics_steps = 0;
};
/** Continuous, native, deterministic environment. Map/route must outlive it.
 * Step chooses classical-driver speed. StepControl bypasses that driver.
 * Reset replays the stored seed. No allocation, network, rendering or sleeping
 * inside the tick; a frame is four 0.005 s RK4 substeps.
 */
class ACarEnv
{
 public:
  enum { ObsLength = 22, MaxFrames = 5000 };
  ACarEnv(const TileMap& map, const TVehicleRoute& route,
          const TCarState& initial, IUD seed = 7);
  void Reset();
  void ResetSeed(IUD seed) { seed_ = seed; Reset(); }
  BOL Done() const { return status_ != CarRunning; }
  ISC Status() const { return status_; }
  FPC Step(ISC action = CarCruise);
  FPC StepControl(const TCarControl& control);
  void Observe();
  const FPC* Obs() const { return obs_; }
  const TCarState& State() const { return car_; }
  const TCarMetrics& Metrics() const { return metrics_; }
  TCarParams& Params() { return params_; }
  FPD GoalDistance() const;
 private:
  const TileMap& map_;
  const TVehicleRoute& route_;
  TCarState initial_, car_;
  TCarParams params_;
  ACarPilot pilot_;
  TCarMetrics metrics_;
  FPC obs_[ObsLength] = {};
  IUD seed_;
  ISC status_ = CarRunning;
  void Peaks();
};
} //< namespace iGeek
#endif
#endif
