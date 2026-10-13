// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_BIKEWORLD_PILOT_HXX
#define CRABS_BIKEWORLD_PILOT_HXX
#include "BikePilot.h"
#include "VehicleRoute.hxx"
#if SEAM >= CRABS_BIKEWORLD
namespace _ {
TBikeControl ABikePilot::Control(const TileMap& map,
    const TVehicleRoute& route, const TBikeState& s, const TBikeParams& p,
    FPD cruise_speed, FPD dt) {
  TBikeControl control; control.foot_down = true;
  if (route.count < 2 || route.count > VehicleRouteMax ||
      !isfinite(cruise_speed) || !isfinite(dt) || dt <= 0) {
    control.brake = 1; return control;
  }
  FPD lookahead = 3.5 + 0.8 * s.forward;
  guide_ = TVehicleGuidance(route, s.x, s.y, s.heading, lookahead);
  FPD ch = cos(s.heading), sh = sin(s.heading);
  TVehicleSurface sf = TVehicleSample(map, s.x + p.front_arm * ch,
                                          s.y + p.front_arm * sh);
  TVehicleSurface sr = TVehicleSample(map, s.x - p.rear_arm * ch,
                                          s.y - p.rear_arm * sh);
  FPD friction = TBikeMin(sf.friction, sr.friction);
  FPD lateral_budget = TBikeMin(2.0, 0.4 * friction * BikeGravity);
  target_speed_ = TBikeClamp(cruise_speed, 0, 5.0);
  target_speed_ = TBikeMin(target_speed_, sqrt(lateral_budget /
      TBikeMax(fabs(guide_.curvature), 0.001)));
  // Taper speed for a supported halt 0.5 m before the final waypoint.
  FPD stop_decel = TBikeMin(1.2, 0.4 * friction * BikeGravity);
  target_speed_ = TBikeMin(target_speed_,
      sqrt(2 * stop_decel * TBikeMax(guide_.remaining - 0.5, 0)));
  // Emergency braking uses an inflated swept envelope, not a point ray.
  FPD horizon = 14, contact;
  FPD motion_angle = s.heading + atan2(s.lateral, TBikeMax(s.forward, 0.5));
  TVehicleSweep(map, s.x, s.y, s.x + horizon * cos(motion_angle),
      s.y + horizon * sin(motion_angle), 0.8, contact);
  FPD gap = horizon * contact;
  target_speed_ = TBikeMin(target_speed_,
      sqrt(2 * stop_decel * TBikeMax(gap - 1.5, 0)));
  FPD requested_ay = TBikeClamp(s.forward * s.forward * guide_.curvature,
                              -lateral_budget, lateral_budget);
  FPD desired_roll = atan2(requested_ay, BikeGravity);
  if (target_speed_ < 0.6) desired_roll = 0;
  roll_target_ += TBikeMin(dt / 0.25, 1.0) * (desired_roll - roll_target_);
  // Invert the front tire force law to control roll acceleration. The g*tan
  // term supplies equilibrium lean; PD error supplies countersteering.
  FPD ay = BikeGravity * tan(s.roll) +
      14.4 * (s.roll - roll_target_) + 5.76 * s.roll_rate;
  ay = TBikeClamp(ay, -0.8 * friction * BikeGravity,
                      0.8 * friction * BikeGravity);
  FPD speed_chart = TBikeMax(s.forward, 0.5);
  FPD rear_force = -p.rear_stiffness *
      atan2(s.lateral - p.rear_arm * s.yaw_rate, speed_chart);
  FPD rear_normal = p.mass * BikeGravity * p.front_arm /
      (p.front_arm + p.rear_arm);
  rear_force = TBikeClamp(rear_force, -sr.friction * rear_normal,
                                      sr.friction * rear_normal);
  FPD slope_x = (sf.slope_x + sr.slope_x) * 0.5;
  FPD slope_y = (sf.slope_y + sr.slope_y) * 0.5;
  FPD grade_lateral = BikeGravity * (-slope_x * sh + slope_y * ch);
  control.steer = TBikeClamp((p.mass * (ay + grade_lateral) - rear_force) /
      p.front_stiffness + atan2(s.lateral + p.front_arm * s.yaw_rate,
                                speed_chart), -p.steer_limit, p.steer_limit);
  FPD resistance = p.mass * BikeGravity * (sf.rolling + sr.rolling) * 0.5 +
      0.5 * p.air_density * p.drag_area * s.forward * s.forward +
      p.mass * BikeGravity * (slope_x * ch + slope_y * sh);
  FPD force = resistance + p.mass * 1.6 * (target_speed_ - s.forward);
  FPD drive = TBikeMin(p.drive_force,
                      p.drive_power / TBikeMax(s.forward, 1.0));
  if (force >= 0) control.throttle = TBikeClamp(force / drive, 0, 1);
  else control.brake = TBikeClamp(-force / p.brake_force, 0, 1);
  if (target_speed_ == 0 && s.forward < 0.1) {
    control.throttle = 0; control.brake = 0.3;
  }
  return control;
}
}  //< namespace _
#endif
#endif
