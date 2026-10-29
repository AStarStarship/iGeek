// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_CARWORLD_2D_HXX
#define CRABS_CARWORLD_2D_HXX
#include "CarWorld2D.h"
#include "CarPilot.hxx"
#if SEAM >= CRABS_CARWORLD
namespace iGeek {
const CHA* TCarStatusName(ISC status) {
  const CHA* names[] = {"running", "goal", "collision", "timeout", "invalid", "blocked", "stopped"};
  return status >= CarRunning && status <= CarStopped ? names[status] : "?";
}
ACarEnv::ACarEnv(const TileMap& map, const TVehicleRoute& route,
    const TCarState& initial, IUD seed)
    : map_(map), route_(route), initial_(initial), seed_(seed) { Reset(); }
void ACarEnv::Reset() {
  car_ = initial_; pilot_.Reset(); metrics_ = {}; status_ = CarRunning;
  IUD rng = seed_ + IUD(0x9E3779B97F4A7C15ull);
  rng ^= rng >> 12; rng ^= rng << 25; rng ^= rng >> 27;
  car_.heading += (FPD(rng % 2001) / 1000 - 1) * 0.025;
  car_.y += (FPD((rng >> 16) % 2001) / 1000 - 1) * 0.15;
  if (!TCarStateValid(car_) || !TCarParamsValid(params_)) status_ = CarInvalid;
  if (!Done() && (route_.count < 2 || route_.count > VehicleRouteMax)) status_ = CarBlocked;
  if (!Done()) {
    for (ISC i = 0; i < route_.count; ++i) {
      const TVehiclePoint& point = route_.points[i];
      if (!isfinite(point.x) || !isfinite(point.y) || point.x < 0 || point.y < 0 ||
          point.x >= map_.SizeMaxF().x || point.y >= map_.SizeMaxF().y) status_ = CarInvalid;
    }
  }
  if (!Done()) {
    FPD contact;
    if (TVehicleSweep(map_, car_.x, car_.y, car_.x, car_.y, params_.body_radius, contact))
      status_ = CarCollision;
    Peaks();
  }
  Observe();
}
FPD ACarEnv::GoalDistance() const {
  if (route_.count < 2 || route_.count > VehicleRouteMax) return 1e100;
  const TVehiclePoint& goal = route_.points[route_.count - 1];
  return hypot(goal.x - car_.x, goal.y - car_.y);
}
void ACarEnv::Peaks() {
  TVehicleGuide guide = TVehicleGuidance(route_, car_.x, car_.y, car_.heading, 5);
  metrics_.max_cross_track = TBikeMax(metrics_.max_cross_track, guide.cross_track);
  metrics_.max_speed = TBikeMax(metrics_.max_speed, hypot(car_.forward, car_.lateral));
  metrics_.max_lateral_accel = TBikeMax(metrics_.max_lateral_accel, fabs(car_.lateral_accel));
  metrics_.max_utilization = TBikeMax(metrics_.max_utilization, car_.utilization);
}
FPC ACarEnv::Step(ISC action) {
  if (Done()) return 0;
  if (action < 0 || action >= CarActionCount) {
    status_ = CarInvalid; Observe(); return -10;
  }
  FPD speeds[CarActionCount] = {8, 5, 12, 0, 2};
  TCarControl control = pilot_.Control(map_, route_, car_, params_, speeds[action]);
  FPC reward = StepControl(control);
  if (!Done() && hypot(car_.forward, car_.lateral) < 0.1 && metrics_.frames > 25) {
    if (action == CarStop) status_ = CarStopped;
    else if (pilot_.TargetSpeed() < 0.1) { status_ = CarBlocked; reward -= 5; }
    if (Done()) Observe();
  }
  return reward;
}
FPC ACarEnv::StepControl(const TCarControl& control) {
  if (Done()) return 0;
  if (!TCarParamsValid(params_) || !isfinite(control.steer) ||
      !isfinite(control.throttle) || !isfinite(control.brake)) {
    status_ = CarInvalid; Observe(); return -10;
  }
  TVehicleGuide before = TVehicleGuidance(route_, car_.x, car_.y, car_.heading, 5);
  for (ISC substep = 0; substep < 4; ++substep) {
    TCarState old = car_;
    TCarContacts contacts = TCarSampleContacts(map_, car_, params_);
    if (!TCarStep(car_, params_, control, contacts, 0.005)) { status_ = CarInvalid; break; }
    ++metrics_.physics_steps;
    FPD contact;
    if (TVehicleSweep(map_, old.x, old.y, car_.x, car_.y, params_.body_radius, contact)) {
      FPD fraction = TBikeMax(0, contact - 1e-8);
      car_.x = old.x + fraction * (car_.x - old.x);
      car_.y = old.y + fraction * (car_.y - old.y);
      car_.distance = old.distance + hypot(car_.x - old.x, car_.y - old.y);
      car_.forward = 0; car_.lateral = 0; car_.yaw_rate = 0;
      status_ = CarCollision;
    }
    Peaks();
    if (Done()) break;
  }
  ++metrics_.frames;
  TVehicleGuide after = TVehicleGuidance(route_, car_.x, car_.y, car_.heading, 5);
  metrics_.cross_track_squared += after.cross_track * after.cross_track;
  if (!Done() && GoalDistance() < 1.2 && hypot(car_.forward, car_.lateral) < 0.1)
    status_ = CarGoal;
  if (!Done() && metrics_.frames >= MaxFrames) status_ = CarTimeout;
  FPD reward = before.remaining - after.remaining - 0.001 - 0.001 * after.cross_track;
  if (status_ == CarGoal) reward += 10;
  if (status_ == CarCollision || status_ == CarInvalid) reward -= 10;
  Observe(); return FPC(reward);
}
void ACarEnv::Observe() {
  for (ISC i = 0; i < ObsLength; ++i) obs_[i] = 0;
  if (status_ == CarInvalid || route_.count < 2 || route_.count > VehicleRouteMax) return;
  const TVehiclePoint& goal = route_.points[route_.count - 1];
  FPD ch = cos(car_.heading), sh = sin(car_.heading);
  FPD dx = goal.x - car_.x, dy = goal.y - car_.y;
  TVehicleGuide guide = TVehicleGuidance(route_, car_.x, car_.y, car_.heading, 5);
  TCarContacts contacts = TCarSampleContacts(map_, car_, params_);
  obs_[0] = FPC(car_.forward / 12); obs_[1] = FPC(car_.lateral / 12);
  obs_[2] = FPC(car_.yaw_rate / 2); obs_[3] = FPC(car_.steer / 0.55);
  obs_[4] = FPC(sh); obs_[5] = FPC(ch);
  obs_[6] = FPC((ch * dx + sh * dy) / 120);
  obs_[7] = FPC((-sh * dx + ch * dy) / 64); obs_[8] = FPC(hypot(dx, dy) / 120);
  obs_[9] = FPC(guide.curvature * 10); obs_[10] = FPC(guide.cross_track / 8);
  obs_[11] = FPC(guide.remaining / 160);
  for (ISC i = 0; i < CarWheelCount; ++i) {
    obs_[12 + i] = FPC(contacts.wheels[i].friction);
    obs_[16] += FPC(contacts.wheels[i].slope_x * 0.25);
    obs_[17] += FPC(contacts.wheels[i].slope_y * 0.25);
  }
  for (ISC ray = 0; ray < 3; ++ray) {
    FPD offset = ray == 0 ? 0 : ray == 1 ? 0.6 : -0.6;
    obs_[18 + ray] = FPC(TVehicleRange(map_, car_.x, car_.y, car_.heading + offset, 40) / 40);
  }
  obs_[21] = FPC(FPD(metrics_.frames) / FPD(MaxFrames));
}
} //< namespace iGeek
#endif
#endif
