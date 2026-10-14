// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_BIKEWORLD_PHYSICS_HXX
#define CRABS_BIKEWORLD_PHYSICS_HXX
#include "BikePhysics.h"
#include <math.h>
#if SEAM >= CRABS_BIKEWORLD
namespace _ {
constexpr FPD BikeGravity = 9.80665;
constexpr FPD BikePi = 3.14159265358979323846;
inline FPD TBikeClamp(FPD value, FPD low, FPD high) {
  return value < low ? low : value > high ? high : value;
}
inline FPD TBikeMin(FPD a, FPD b) { return a < b ? a : b; }
inline FPD TBikeMax(FPD a, FPD b) { return a > b ? a : b; }
inline FPD TBikeAngle(FPD angle) { return atan2(sin(angle), cos(angle)); }
/** Continuous state packing is explicit; never reinterpret a struct as an array. */
inline void TBikePack(const TBikeState& s, FPD* v) {
  v[0] = s.x; v[1] = s.y; v[2] = s.heading; v[3] = s.forward;
  v[4] = s.lateral; v[5] = s.yaw_rate; v[6] = s.roll;
  v[7] = s.roll_rate; v[8] = s.steer;
}
inline BOL TBikeSurfaceValid(const TVehicleSurface& s) {
  return isfinite(s.friction) && s.friction > 0 &&
      isfinite(s.rolling) && s.rolling >= 0 &&
      isfinite(s.slope_x) && isfinite(s.slope_y);
}
/** Project an axle force onto its Coulomb friction circle. */
inline FPD TBikeTire(FPD& fx, FPD& fy, FPD limit) {
  FPD magnitude = hypot(fx, fy);
  if (magnitude > limit) {
    fx *= limit / magnitude; fy *= limit / magnitude;
  }
  return magnitude / limit;
}
/** Newton-Euler planar single-track model with finite cornering stiffness.
 * Roll torque: m*h*(g*sin(phi) - ay*cos(phi)); no hidden balance controller.
 * Body-forward speed is regularized only in the low-speed slip-angle chart.
 */
inline void TBikeDerivative(const FPD* v, const TBikeParams& p,
    const TBikeControl& c, const TVehicleSurface& sf,
    const TVehicleSurface& sr, FPD* d, FPD& utilization, FPD& ay) {
  FPD length = p.front_arm + p.rear_arm;
  FPD slope_x = (sf.slope_x + sr.slope_x) * 0.5;
  FPD slope_y = (sf.slope_y + sr.slope_y) * 0.5;
  FPD normal_g = BikeGravity / sqrt(1 + slope_x * slope_x + slope_y * slope_y);
  FPD nf = p.mass * normal_g * p.rear_arm / length;
  FPD nr = p.mass * normal_g * p.front_arm / length;
  FPD u = TBikeMax(v[3], 0.5);
  FPD brake = TBikeClamp(c.brake, 0, 1) * p.brake_force;
  FPD fxf = -brake * p.rear_arm / length;
  FPD fxr = TBikeClamp(c.throttle, 0, 1) * TBikeMin(p.drive_force,
      p.drive_power / TBikeMax(v[3], 1.0)) - brake * p.front_arm / length;
  FPD fyf = p.front_stiffness *
      (v[8] - atan2(v[4] + p.front_arm * v[5], u));
  FPD fyr = -p.rear_stiffness * atan2(v[4] - p.rear_arm * v[5], u);
  utilization = TBikeMax(TBikeTire(fxf, fyf, nf * sf.friction),
                         TBikeTire(fxr, fyr, nr * sr.friction));
  FPD cs = cos(v[8]), sn = sin(v[8]);
  FPD fx = fxf * cs - fyf * sn + fxr;
  FPD fy = fyf * cs + fxf * sn + fyr;
  FPD ch = cos(v[2]), sh = sin(v[2]);
  FPD grade_forward = normal_g * (slope_x * ch + slope_y * sh);
  FPD grade_lateral = normal_g * (-slope_x * sh + slope_y * ch);
  FPD resistance = 0.5 * p.air_density * p.drag_area * v[3] * fabs(v[3]);
  if (v[3] > 0) resistance += nf * sf.rolling + nr * sr.rolling;
  ay = fy / p.mass - grade_lateral;
  d[0] = v[3] * ch - v[4] * sh;
  d[1] = v[3] * sh + v[4] * ch;
  d[2] = v[5];
  d[3] = (fx - resistance) / p.mass + v[4] * v[5] - grade_forward;
  d[4] = ay - v[3] * v[5];
  d[5] = (p.front_arm * (fyf * cs + fxf * sn) - p.rear_arm * fyr) /
      p.yaw_inertia;
  d[6] = v[7];
  d[7] = p.mass * p.cg_height / p.roll_inertia *
      (normal_g * sin(v[6]) - ay * cos(v[6])) - p.roll_damping * v[7];
  d[8] = TBikeClamp((TBikeClamp(c.steer, -p.steer_limit, p.steer_limit) -
      v[8]) / p.steer_tau, -p.steer_rate, p.steer_rate);
}
BOL TBikeStep(TBikeState& bike, const TBikeParams& p,
    const TBikeControl& c, const TVehicleSurface& front,
    const TVehicleSurface& rear, FPD dt) {
  FPD values[9], work[9], k1[9], k2[9], k3[9], k4[9];
  TBikePack(bike, values);
  for (ISC i = 0; i < 9; ++i) if (!isfinite(values[i])) return false;
  if (bike.forward < 0 || !isfinite(bike.time) || bike.time < 0 ||
      !isfinite(bike.distance) || bike.distance < 0 ||
      !isfinite(bike.lateral_accel) || !isfinite(bike.utilization)) return false;
  FPD positive[] = {p.mass, p.front_arm, p.rear_arm, p.cg_height,
      p.yaw_inertia, p.roll_inertia, p.front_stiffness, p.rear_stiffness,
      p.steer_limit, p.steer_rate, p.steer_tau, p.fall_angle};
  for (FPD value : positive) if (!isfinite(value) || value <= 0) return false;
  FPD nonnegative[] = {p.drag_area, p.air_density, p.drive_force,
      p.drive_power, p.brake_force, p.roll_damping};
  for (FPD value : nonnegative)
    if (!isfinite(value) || value < 0) return false;
  if (!isfinite(dt) || dt <= 0 || dt > 0.01 ||
      !isfinite(c.steer) || !isfinite(c.throttle) || !isfinite(c.brake) ||
      !TBikeSurfaceValid(front) || !TBikeSurfaceValid(rear)) return false;
  if (bike.fallen) return true;
  FPD utilization = 0, ay = 0;
  TBikeDerivative(values, p, c, front, rear, k1, utilization, ay);
  for (ISC i = 0; i < 9; ++i) work[i] = values[i] + 0.5 * dt * k1[i];
  TBikeDerivative(work, p, c, front, rear, k2, utilization, ay);
  for (ISC i = 0; i < 9; ++i) work[i] = values[i] + 0.5 * dt * k2[i];
  TBikeDerivative(work, p, c, front, rear, k3, utilization, ay);
  for (ISC i = 0; i < 9; ++i) work[i] = values[i] + dt * k3[i];
  TBikeDerivative(work, p, c, front, rear, k4, utilization, ay);
  for (ISC i = 0; i < 9; ++i) {
    values[i] += dt / 6 * (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]);
    if (!isfinite(values[i])) return false;
  }
  bike.distance += hypot(values[0] - bike.x, values[1] - bike.y);
  bike.x = values[0]; bike.y = values[1];
  bike.heading = TBikeAngle(values[2]);
  bike.forward = TBikeMax(values[3], 0);  // Forward-only drivetrain/stop.
  bike.lateral = values[4]; bike.yaw_rate = values[5];
  bike.roll = values[6]; bike.roll_rate = values[7]; bike.steer = values[8];
  bike.time += dt; bike.lateral_accel = ay; bike.utilization = utilization;
  bike.supported = c.foot_down && bike.forward < 0.45 && fabs(bike.roll) < 0.35;
  if (bike.supported) {
    bike.roll = 0; bike.roll_rate = 0;
    if (bike.forward < 0.02) { bike.lateral = 0; bike.yaw_rate = 0; }
  }
  bike.fallen = fabs(bike.roll) >= p.fall_angle;
  return true;
}
}  //< namespace _
#endif
#endif
