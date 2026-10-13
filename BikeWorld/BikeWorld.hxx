// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_BIKEWORLD_HXX
#define CRABS_BIKEWORLD_HXX
#include "BikeWorld.h"
#include "BikePilot.hxx"
#if SEAM >= CRABS_BIKEWORLD
namespace _ {
const CHA* TBikeStatusName(ISC status) {
  const CHA* names[] = {"running", "goal", "collision", "fallen", "timeout",
                       "invalid", "blocked", "stopped"};
  return status >= BikeRunning && status <= BikeStopped ? names[status] : "?";
}
ABikeEnv::ABikeEnv(const TileMap& map, const TVehicleRoute& route,
    const TBikeState& initial, IUD seed)
    : map_(map), route_(route), initial_(initial), seed_(seed) { Reset(); }
void ABikeEnv::Reset() {
  bike_ = initial_; pilot_.Reset(); metrics_ = {}; status_ = BikeRunning;
  // Seeded, small pose perturbation. No stochastic force or policy inference.
  IUD rng = seed_ + IUD(0x9E3779B97F4A7C15ull);
  rng ^= rng >> 12; rng ^= rng << 25; rng ^= rng >> 27;
  bike_.roll += (FPD(rng % 2001) / 1000.0 - 1.0) * 0.03;
  bike_.heading += (FPD((rng >> 16) % 2001) / 1000.0 - 1.0) * 0.015;
  FPD values[9]; TBikePack(bike_, values);
  for (FPD value : values) if (!isfinite(value)) {
    status_ = BikeInvalid; Observe(); return;
  }
  if (bike_.forward < 0 || !isfinite(bike_.time) || bike_.time < 0 ||
      !isfinite(bike_.distance) || bike_.distance < 0 ||
      !isfinite(bike_.lateral_accel) || !isfinite(bike_.utilization)) {
    status_ = BikeInvalid; Observe(); return;
  }
  if (route_.count < 2 || route_.count > VehicleRouteMax) {
    status_ = BikeBlocked; Observe(); return;
  }
  for (ISC i = 0; i < route_.count; ++i) {
    const TVehiclePoint& point = route_.points[i];
    if (!isfinite(point.x) || !isfinite(point.y) || point.x < 0 || point.y < 0 ||
        point.x >= map_.SizeMaxF().x || point.y >= map_.SizeMaxF().y) {
      status_ = BikeInvalid; Observe(); return;
    }
  }
  metrics_.max_roll = fabs(bike_.roll); metrics_.max_speed = bike_.forward;
  if (bike_.fallen || fabs(bike_.roll) >= params_.fall_angle) status_ = BikeFallen;
  FPD contact;
  if (TVehicleSweep(map_, bike_.x, bike_.y, bike_.x, bike_.y, 0.8, contact))
    status_ = BikeCollision;
  Observe();
}
FPD ABikeEnv::GoalDistance() const {
  if (route_.count < 2 || route_.count > VehicleRouteMax) return 1e100;
  const TVehiclePoint& goal = route_.points[route_.count - 1];
  return hypot(goal.x - bike_.x, goal.y - bike_.y);
}
FPC ABikeEnv::Step(ISC action) {
  if (Done()) return 0;
  if (action < 0 || action >= BikeActionCount) {
    status_ = BikeInvalid; Observe(); return -10;
  }
  FPD speeds[BikeActionCount] = {3.5, 2.5, 4.5, 0, 1.5};
  TBikeControl control = pilot_.Control(map_, route_, bike_, params_,
                                       speeds[action], 0.02);
  FPC reward = StepControl(control);
  if (!Done() && bike_.forward < 0.1 && bike_.supported && metrics_.frames > 50) {
    if (action == BikeStop) status_ = BikeStopped;
    else if (pilot_.TargetSpeed() < 0.1) status_ = BikeBlocked;
    if (Done()) Observe();
  }
  return reward;
}
FPC ABikeEnv::StepControl(const TBikeControl& control) {
  if (Done()) return 0;
  TVehicleGuide before = TVehicleGuidance(route_, bike_.x, bike_.y,
                                         bike_.heading, 5);
  for (ISC substep = 0; substep < 4; ++substep) {
    TBikeState old = bike_;
    FPD ch = cos(bike_.heading), sh = sin(bike_.heading);
    TVehicleSurface front = TVehicleSample(map_, bike_.x + params_.front_arm * ch,
                                                bike_.y + params_.front_arm * sh);
    TVehicleSurface rear = TVehicleSample(map_, bike_.x - params_.rear_arm * ch,
                                               bike_.y - params_.rear_arm * sh);
    if (!TBikeStep(bike_, params_, control, front, rear, 0.005)) {
      status_ = BikeInvalid; break;
    }
    ++metrics_.physics_steps;
    metrics_.max_roll = TBikeMax(metrics_.max_roll, fabs(bike_.roll));
    metrics_.max_speed = TBikeMax(metrics_.max_speed, bike_.forward);
    FPD contact;
    if (TVehicleSweep(map_, old.x, old.y, bike_.x, bike_.y, 0.8, contact)) {
      FPD fraction = TBikeMax(0, contact - 1e-8);
      bike_.x = old.x + fraction * (bike_.x - old.x);
      bike_.y = old.y + fraction * (bike_.y - old.y);
      bike_.distance = old.distance + hypot(bike_.x - old.x, bike_.y - old.y);
      bike_.forward = 0; bike_.lateral = 0; bike_.yaw_rate = 0;
      status_ = BikeCollision; break;
    }
    if (bike_.fallen) { status_ = BikeFallen; break; }
  }
  ++metrics_.frames;
  TVehicleGuide after = TVehicleGuidance(route_, bike_.x, bike_.y,
                                        bike_.heading, 5);
  metrics_.max_roll = TBikeMax(metrics_.max_roll, fabs(bike_.roll));
  metrics_.max_speed = TBikeMax(metrics_.max_speed, bike_.forward);
  metrics_.max_cross_track = TBikeMax(metrics_.max_cross_track, after.cross_track);
  metrics_.cross_track_squared += after.cross_track * after.cross_track;
  if (!Done() && GoalDistance() < 1.2 && bike_.forward < 0.1 && bike_.supported)
    status_ = BikeGoal;
  if (!Done() && metrics_.frames >= MaxFrames) status_ = BikeTimeout;
  FPD reward = before.remaining - after.remaining - 0.001 -
      0.002 * bike_.roll * bike_.roll;
  if (status_ == BikeGoal) reward += 10;
  if (status_ == BikeFallen || status_ == BikeCollision || status_ == BikeInvalid)
    reward -= 10;
  Observe(); return FPC(reward);
}
void ABikeEnv::Observe() {
  for (ISC i = 0; i < ObsLength; ++i) obs_[i] = 0;
  if (status_ == BikeInvalid || route_.count < 2 || route_.count > VehicleRouteMax)
    return;
  const TVehiclePoint& goal = route_.points[route_.count - 1];
  FPD ch = cos(bike_.heading), sh = sin(bike_.heading);
  FPD dx = goal.x - bike_.x, dy = goal.y - bike_.y;
  TVehicleGuide guide = TVehicleGuidance(route_, bike_.x, bike_.y,
                                        bike_.heading, 5);
  TVehicleSurface surface = TVehicleSample(map_, bike_.x, bike_.y);
  obs_[0] = FPC(bike_.forward / 5); obs_[1] = FPC(bike_.lateral / 5);
  obs_[2] = FPC(bike_.yaw_rate / 3); obs_[3] = FPC(bike_.roll / 0.7);
  obs_[4] = FPC(bike_.roll_rate / 4); obs_[5] = FPC(bike_.steer / 0.55);
  obs_[6] = FPC(sh); obs_[7] = FPC(ch);
  obs_[8] = FPC((ch * dx + sh * dy) / 120);
  obs_[9] = FPC((-sh * dx + ch * dy) / 64);
  obs_[10] = FPC(hypot(dx, dy) / 120);
  obs_[11] = FPC(guide.curvature * 10); obs_[12] = FPC(guide.cross_track / 5);
  obs_[13] = FPC(surface.friction); obs_[14] = FPC(surface.slope_x);
  obs_[15] = FPC(surface.slope_y);
  obs_[16] = FPC(TVehicleRange(map_, bike_.x, bike_.y, bike_.heading, 14) / 14);
  obs_[17] = FPC(TVehicleRange(map_, bike_.x, bike_.y, bike_.heading + 0.6, 14) / 14);
  obs_[18] = FPC(TVehicleRange(map_, bike_.x, bike_.y, bike_.heading - 0.6, 14) / 14);
  obs_[19] = bike_.supported ? 1 : 0;
  obs_[20] = FPC(FPD(metrics_.frames) / FPD(MaxFrames));
}
}  //< namespace _
#endif
#endif
