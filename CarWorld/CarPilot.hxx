// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_CARWORLD_PILOT_HXX
#define CRABS_CARWORLD_PILOT_HXX
#include "CarPilot.h"
#include "CarPhysics.hxx"
#include "../BikeWorld/VehicleRoute.hxx"
#if SEAM >= CRABS_CARWORLD
namespace _ {
inline TCarContacts TCarSampleContacts(const TileMap& map,
    const TCarState& car, const TCarParams& p) {
  TCarContacts contacts;
  FPD ch = cos(car.heading), sh = sin(car.heading);
  for (ISC i = 0; i < CarWheelCount; ++i) {
    FPD x = i < CarRearLeft ? p.front_arm : -p.rear_arm;
    FPD y = (i % 2 == 0 ? 0.5 : -0.5) * p.track;
    contacts.wheels[i] = TVehicleSample(map, car.x + x * ch - y * sh,
                                            car.y + x * sh + y * ch);
  }
  return contacts;
}
/** Anticipate polyline corners early enough to shed speed on low grip.
 * Corner curvature models the follower's lookahead rounding, not exact road
 * curvature. The braking envelope uses distance along the planned route.
 */
inline FPD TCarCornerSpeed(const TVehicleRoute& route, const TCarState& s,
    FPD lookahead, FPD lateral_budget, FPD decel, FPD speed) {
  FPD nearest = 1e100, progress = 0, length_before = 0;
  for (ISC i = 1; i < route.count; ++i) {
    FPD dx = route.points[i].x - route.points[i - 1].x;
    FPD dy = route.points[i].y - route.points[i - 1].y;
    FPD length = hypot(dx, dy);
    FPD t = length > 1e-9 ? TBikeClamp(((s.x - route.points[i - 1].x) * dx +
        (s.y - route.points[i - 1].y) * dy) / (length * length), 0, 1) : 0;
    FPD error = hypot(s.x - route.points[i - 1].x - t * dx,
                      s.y - route.points[i - 1].y - t * dy);
    if (error < nearest) { nearest = error; progress = length_before + t * length; }
    length_before += length;
  }
  FPD corner_at = 0;
  for (ISC i = 1; i + 1 < route.count; ++i) {
    FPD ax = route.points[i].x - route.points[i - 1].x;
    FPD ay = route.points[i].y - route.points[i - 1].y;
    FPD bx = route.points[i + 1].x - route.points[i].x;
    FPD by = route.points[i + 1].y - route.points[i].y;
    corner_at += hypot(ax, ay);
    FPD ahead = corner_at - progress;
    if (ahead < -lookahead) continue;
    FPD turn = fabs(atan2(ax * by - ay * bx, ax * bx + ay * by));
    FPD curvature = 2 * sin(0.5 * turn) / lookahead;
    if (curvature < 1e-4) continue;
    FPD corner_speed2 = lateral_budget / curvature;
    speed = TBikeMin(speed, sqrt(corner_speed2 +
        2 * decel * TBikeMax(ahead - lookahead, 0)));
  }
  return speed;
}
TCarControl ACarPilot::Control(const TileMap& map, const TVehicleRoute& route,
    const TCarState& s, const TCarParams& p, FPD cruise_speed) {
  TCarControl control;
  if (route.count < 2 || route.count > VehicleRouteMax ||
      !isfinite(cruise_speed) || !TCarParamsValid(p) || !TCarStateValid(s)) {
    target_speed_ = 0; control.brake = 1; return control;
  }
  FPD lookahead = 4.0 + 0.7 * s.forward;
  FPD motion_angle = s.heading + atan2(s.lateral, TBikeMax(s.forward, 2.0));
  guide_ = TVehicleGuidance(route, s.x, s.y, motion_angle, lookahead);
  TCarContacts contacts = TCarSampleContacts(map, s, p);
  FPD friction = 2, rolling = 0, slope_x = 0, slope_y = 0;
  for (const TVehicleSurface& surface : contacts.wheels) {
    friction = TBikeMin(friction, surface.friction);
    rolling += surface.rolling * 0.25;
    slope_x += surface.slope_x * 0.25; slope_y += surface.slope_y * 0.25;
  }
  // Ideal material preview is declared; brake before entering an ice strip.
  FPD horizon = 40;
  for (FPD ahead = 4; ahead <= horizon; ahead += 4)
    friction = TBikeMin(friction, TVehicleSample(map,
        s.x + ahead * cos(motion_angle), s.y + ahead * sin(motion_angle)).friction);
  FPD lateral_budget = TBikeMin(2.5, 0.4 * friction * BikeGravity);
  FPD stop_decel = TBikeMin(2.5, 0.55 * friction * BikeGravity);
  target_speed_ = TCarCornerSpeed(route, s, lookahead, lateral_budget,
                                 stop_decel, TBikeClamp(cruise_speed, 0, 12));
  target_speed_ = TBikeMin(target_speed_,
      sqrt(lateral_budget / TBikeMax(fabs(guide_.curvature), 0.001)));
  target_speed_ = TBikeMin(target_speed_,
      sqrt(2 * stop_decel * TBikeMax(guide_.remaining - 0.7, 0)));
  FPD contact;
  TVehicleSweep(map, s.x, s.y, s.x + horizon * cos(motion_angle),
      s.y + horizon * sin(motion_angle), p.body_radius, contact);
  target_speed_ = TBikeMin(target_speed_,
      sqrt(2 * stop_decel * TBikeMax(horizon * contact - 2.0, 0)));
  FPD length = p.front_arm + p.rear_arm;
  FPD understeer = p.mass / length *
      (p.rear_arm / p.front_stiffness - p.front_arm / p.rear_stiffness);
  control.steer = TBikeClamp(atan((length + understeer * s.forward * s.forward) *
      guide_.curvature), -p.steer_limit, p.steer_limit);
  FPD resistance = p.mass * BikeGravity * rolling +
      0.5 * p.air_density * p.drag_area * s.forward * s.forward +
      p.mass * BikeGravity * (slope_x * cos(s.heading) + slope_y * sin(s.heading));
  FPD force = resistance + p.mass * 1.8 * (target_speed_ - s.forward);
  FPD drive = TBikeMin(p.drive_force, p.drive_power / TBikeMax(s.forward, 1.0));
  if (force >= 0 && drive > 0) control.throttle = TBikeClamp(force / drive, 0, 1);
  else if (force < 0 && p.brake_force > 0)
    control.brake = TBikeClamp(-force / p.brake_force, 0, 1);
  if (target_speed_ < 0.05 && s.forward < 0.1) {
    control.throttle = 0; control.brake = 0.4;
  }
  return control;
}
}  //< namespace _
#endif
#endif
