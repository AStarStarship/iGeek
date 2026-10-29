// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_CARWORLD_PHYSICS_HXX
#define CRABS_CARWORLD_PHYSICS_HXX
#include "CarPhysics.h"
#include "../BikeWorld/VehicleMap.hxx"
#if SEAM >= CRABS_CARWORLD
namespace iGeek {
inline void TCarPack(const TCarState& s, FPD* v) {
  v[0] = s.x; v[1] = s.y; v[2] = s.heading; v[3] = s.forward;
  v[4] = s.lateral; v[5] = s.yaw_rate; v[6] = s.steer;
}
inline BOL TCarParamsValid(const TCarParams& p) {
  FPD positive[] = {p.mass, p.front_arm, p.rear_arm, p.track, p.yaw_inertia,
      p.front_stiffness, p.rear_stiffness, p.steer_limit, p.steer_rate,
      p.steer_tau, p.body_radius};
  for (FPD value : positive)
    if (!isfinite(value) || value <= 0 || value > 1e12) return false;
  FPD nonnegative[] = {p.drag_area, p.air_density, p.drive_force,
      p.drive_power, p.brake_force};
  for (FPD value : nonnegative)
    if (!isfinite(value) || value < 0 || value > 1e12) return false;
  return p.steer_limit <= 1.3 && p.body_radius <= 100 &&
      isfinite(p.brake_bias) && p.brake_bias >= 0 && p.brake_bias <= 1;
}
inline BOL TCarStateValid(const TCarState& s) {
  FPD values[7]; TCarPack(s, values);
  for (FPD value : values) if (!isfinite(value)) return false;
  return fabs(s.x) <= 1e9 && fabs(s.y) <= 1e9 && fabs(s.heading) <= 1e6 &&
      s.forward >= 0 && s.forward <= 1e4 && fabs(s.lateral) <= 1e4 &&
      fabs(s.yaw_rate) <= 1e4 && fabs(s.steer) <= 1.3 &&
      isfinite(s.time) && s.time >= 0 && s.time <= 1e12 &&
      isfinite(s.distance) && s.distance >= 0 && s.distance <= 1e15 &&
      isfinite(s.longitudinal_accel) && isfinite(s.lateral_accel) &&
      isfinite(s.utilization) && s.utilization >= 0;
}
/** Each wheel has its own slip angle and friction circle. Front wheels share
 * one steer angle (parallel-steer approximation), rear wheels supply drive.
 * Static loads omit longitudinal/lateral transfer and suspension dynamics.
 */
inline void TCarDerivative(const FPD* v, const TCarParams& p,
    const TCarControl& c, const TCarContacts& contacts, FPD* d,
    FPD& utilization, FPD& ax, FPD& ay) {
  FPD slope_x = 0, slope_y = 0;
  for (const TVehicleSurface& s : contacts.wheels) {
    slope_x += 0.25 * s.slope_x; slope_y += 0.25 * s.slope_y;
  }
  FPD normal_g = BikeGravity / sqrt(1 + slope_x * slope_x + slope_y * slope_y);
  FPD length = p.front_arm + p.rear_arm;
  FPD fx_total = 0, fy_total = 0, moment = 0, rolling = 0;
  utilization = 0;
  for (ISC wheel = 0; wheel < CarWheelCount; ++wheel) {
    BOL front = wheel < CarRearLeft;
    FPD x = front ? p.front_arm : -p.rear_arm;
    FPD y = (wheel % 2 == 0 ? 0.5 : -0.5) * p.track;
    FPD angle = front ? v[6] : 0, cs = cos(angle), sn = sin(angle);
    FPD u = v[3] - v[5] * y, side = v[4] + v[5] * x;
    FPD u_tire = u * cs + side * sn, v_tire = side * cs - u * sn;
    FPD normal = 0.5 * p.mass * normal_g *
        (front ? p.rear_arm : p.front_arm) / length;
    FPD fx = front ? 0 : 0.5 * TBikeClamp(c.throttle, 0, 1) *
        TBikeMin(p.drive_force, p.drive_power / TBikeMax(v[3], 1.0));
    fx -= 0.5 * TBikeClamp(c.brake, 0, 1) * p.brake_force *
        (front ? p.brake_bias : 1 - p.brake_bias) * tanh(u_tire / 0.05);
    // Low-speed slip chart is regularized at 2 m/s, not a hidden driver.
    FPD fy = -0.5 * (front ? p.front_stiffness : p.rear_stiffness) *
        atan2(v_tire, TBikeMax(u_tire, 2.0));
    const TVehicleSurface& surface = contacts.wheels[wheel];
    utilization = TBikeMax(utilization,
        TBikeTire(fx, fy, surface.friction * normal));
    FPD bx = fx * cs - fy * sn, by = fx * sn + fy * cs;
    fx_total += bx; fy_total += by; moment += x * by - y * bx;
    rolling += normal * surface.rolling * tanh(v[3] / 0.2);
  }
  FPD ch = cos(v[2]), sh = sin(v[2]);
  FPD drag = 0.5 * p.air_density * p.drag_area * v[3] * fabs(v[3]);
  ax = (fx_total - drag - rolling) / p.mass -
      normal_g * (slope_x * ch + slope_y * sh);
  ay = fy_total / p.mass - normal_g * (-slope_x * sh + slope_y * ch);
  d[0] = v[3] * ch - v[4] * sh; d[1] = v[3] * sh + v[4] * ch;
  d[2] = v[5]; d[3] = ax + v[4] * v[5]; d[4] = ay - v[3] * v[5];
  d[5] = moment / p.yaw_inertia;
  d[6] = TBikeClamp((TBikeClamp(c.steer, -p.steer_limit, p.steer_limit) -
      v[6]) / p.steer_tau, -p.steer_rate, p.steer_rate);
}
BOL TCarStep(TCarState& car, const TCarParams& p,
    const TCarControl& c, const TCarContacts& contacts, FPD dt) {
  if (!isfinite(dt) || dt <= 0 || dt > 0.01 || !TCarParamsValid(p) ||
      !TCarStateValid(car) || !isfinite(c.steer) || !isfinite(c.throttle) ||
      !isfinite(c.brake)) return false;
  for (const TVehicleSurface& s : contacts.wheels)
    if (!TBikeSurfaceValid(s) || s.friction > 2 || s.rolling > 1 ||
        fabs(s.slope_x) > 1 || fabs(s.slope_y) > 1) return false;
  FPD v[7], work[7], k1[7], k2[7], k3[7], k4[7];
  TCarPack(car, v);
  FPD utilization = 0, ax = 0, ay = 0;
  TCarDerivative(v, p, c, contacts, k1, utilization, ax, ay);
  for (ISC i = 0; i < 7; ++i) work[i] = v[i] + 0.5 * dt * k1[i];
  TCarDerivative(work, p, c, contacts, k2, utilization, ax, ay);
  for (ISC i = 0; i < 7; ++i) work[i] = v[i] + 0.5 * dt * k2[i];
  TCarDerivative(work, p, c, contacts, k3, utilization, ax, ay);
  for (ISC i = 0; i < 7; ++i) work[i] = v[i] + dt * k3[i];
  TCarDerivative(work, p, c, contacts, k4, utilization, ax, ay);
  for (ISC i = 0; i < 7; ++i) {
    v[i] += dt / 6 * (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]);
    if (!isfinite(v[i])) return false;
  }
  v[3] = TBikeMax(v[3], 0);  // No reverse gear in this pilot.
  TCarDerivative(v, p, c, contacts, work, utilization, ax, ay);
  if (!isfinite(utilization) || !isfinite(ax) || !isfinite(ay)) return false;
  TCarState next = car;
  next.x = v[0]; next.y = v[1]; next.heading = TBikeAngle(v[2]);
  next.forward = v[3]; next.lateral = v[4]; next.yaw_rate = v[5]; next.steer = v[6];
  next.time += dt; next.distance += hypot(next.x - car.x, next.y - car.y);
  next.longitudinal_accel = ax; next.lateral_accel = ay; next.utilization = utilization;
  if (!TCarStateValid(next)) return false;
  car = next;
  return true;
}
} //< namespace iGeek
#endif
#endif
