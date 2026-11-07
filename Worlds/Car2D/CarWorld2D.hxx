// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekCarWorld2DHXX
#define _iGeekCarWorld2DHXX
#include "CarWorld2D.h"
#include "../../Gym.h"
#include "../../../ASCIICrabs/Math.hxx"
#include "CarPilot.hxx"
#if _Seam >= _iGeekCarWorldCore
namespace iGeek {
const CHA* TCarStatusName(ISC status) {
  const CHA* names[] = {"running", "goal", "collision", "timeout", "invalid", "blocked", "stopped"};
  return status >= CarRunning && status <= CarStopped ? names[status] : "?";
}
ACarEnv::ACarEnv(TileMap& map, TVehicleRoute& route,
    TCarState& initial, IUD seed)
    : Environment("carworld", 2), map_(map), route_(route),
      initial_(initial), seed_(seed) {
  /* The car env IS the Environment: it owns the Room and the manifest a joining
     agent reads. The Gym only batches it (and may widen the action set to raw
     actuator torques, which the manifest's action_count does not describe). */
  TSPrint(manifest_.name, ISW(ManifestNameLen), "carworld");
  TSPrint(manifest_.version, ISW(ManifestVersionLen), "1.0");
  manifest_.kind = ManifestKindEnv;
  manifest_.domain = ManifestDomainRobotics;
  TSPrint(manifest_.description, ISW(ManifestDescriptionLen),
          "2D car driving with classical driver and MPC");
  TSPrint(manifest_.modality, ISW(ManifestModalityLen), "vector");
  manifest_.reward_low = FPC(-1.0);
  manifest_.reward_high = FPC(1.0);
  manifest_.reward_higher_is_better = true;
  manifest_.determinism = ManifestDeterminismSeeded;
  manifest_.observation_count = ISC(ObservationLength);
  manifest_.action_count = ISC(CarActionCount);
  manifest_.max_episode_steps = ISC(MaxFrames);
  manifest_.environment_count_max = ISC(GymEnvMax);
  TSPrint(manifest_.world_name, ISW(ManifestWorldLen), "CarWorld");
  manifest_.seam_number = _iGeekCarWorldCore;
  manifest_.manifest_version = ISC(1);
  Reset();
}
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
      TVehiclePoint& point = route_.points[i];
      if (!iGeek::IsFinite(point.x) || !iGeek::IsFinite(point.y) || point.x < 0 || point.y < 0 ||
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
FPD ACarEnv::GoalDistance() {
  if (route_.count < 2 || route_.count > VehicleRouteMax) return 1e100;
  TVehiclePoint& goal = route_.points[route_.count - 1];
  return hypot(goal.x - car_.x, goal.y - car_.y);
}
void ACarEnv::Peaks() {
  TVehicleGuide guide = TVehicleGuidance(route_, car_.x, car_.y, car_.heading, 5);
  metrics_.max_cross_track = TBikeMax(metrics_.max_cross_track, guide.cross_track);
  metrics_.max_speed = TBikeMax(metrics_.max_speed, hypot(car_.forward, car_.lateral));
  metrics_.max_lateral_accel = TBikeMax(metrics_.max_lateral_accel, Absolute(car_.lateral_accel));
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
FPC ACarEnv::StepControl(TCarControl& control) {
  if (Done()) return 0;
  if (!TCarParamsValid(params_) || !iGeek::IsFinite(control.steer) ||
      !iGeek::IsFinite(control.throttle) || !iGeek::IsFinite(control.brake)) {
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
  for (ISC i = 0; i < ObservationLength; ++i) observation_[i] = 0;
  if (status_ == CarInvalid || route_.count < 2 || route_.count > VehicleRouteMax) return;
  TVehiclePoint& goal = route_.points[route_.count - 1];
  FPD ch = cos(car_.heading), sh = sin(car_.heading);
  FPD dx = goal.x - car_.x, dy = goal.y - car_.y;
  TVehicleGuide guide = TVehicleGuidance(route_, car_.x, car_.y, car_.heading, 5);
  TCarContacts contacts = TCarSampleContacts(map_, car_, params_);
  observation_[0] = FPC(car_.forward / 12); observation_[1] = FPC(car_.lateral / 12);
  observation_[2] = FPC(car_.yaw_rate / 2); observation_[3] = FPC(car_.steer / 0.55);
  observation_[4] = FPC(sh); observation_[5] = FPC(ch);
  observation_[6] = FPC((ch * dx + sh * dy) / 120);
  observation_[7] = FPC((-sh * dx + ch * dy) / 64); observation_[8] = FPC(hypot(dx, dy) / 120);
  observation_[9] = FPC(guide.curvature * 10); observation_[10] = FPC(guide.cross_track / 8);
  observation_[11] = FPC(guide.remaining / 160);
  for (ISC i = 0; i < CarWheelCount; ++i) {
    observation_[12 + i] = FPC(contacts.wheels[i].friction);
    observation_[16] += FPC(contacts.wheels[i].slope_x * 0.25);
    observation_[17] += FPC(contacts.wheels[i].slope_y * 0.25);
  }
  for (ISC ray = 0; ray < 3; ++ray) {
    FPD offset = ray == 0 ? 0 : ray == 1 ? 0.6 : -0.6;
    observation_[18 + ray] = FPC(TVehicleRange(map_, car_.x, car_.y, car_.heading + offset, 40) / 40);
  }
  observation_[21] = FPC(FPD(metrics_.frames) / FPD(MaxFrames));
}
} //< namespace iGeek
#endif
#endif
