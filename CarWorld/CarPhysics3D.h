// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_CARWORLD_PHYSICS_3D_H
#define CRABS_CARWORLD_PHYSICS_3D_H
#include "CarPhysics.h"
#include "CarPhysics.hxx"
#include <math.h>
#if SEAM >= CRABS_CARWORLD
namespace iGeek {
/** 3D extension. Appends pitch/roll + 4x suspension to the 2D TCarState.
 * wheel_height = suspension compression (m): positive = compressed.
 * wheel_vel = d(wheel_height)/dt (m/s). vertical_tire_force = derived Fz (N). */
struct alignas(ACPUCacheLineSize) TCarState3D
{
  // --- 2D base (identical order to TCarState) ---
  FPD x = 0, y = 0, heading = 0, forward = 0, lateral = 0;
  FPD yaw_rate = 0, steer = 0, time = 0, distance = 0;
  FPD longitudinal_accel = 0, lateral_accel = 0, utilization = 0;
  // --- 3D extension ---
  FPD pitch = 0, pitch_rate = 0;
  FPD roll = 0, roll_rate = 0;
  FPD wheel_height[CarWheelCount] = {};
  FPD wheel_vel[CarWheelCount] = {};
  FPD vertical_tire_force[CarWheelCount] = {};
};
/** 3D extension of TCarParams. Inertias derived at init. */
struct TCarParams3D
{
  TCarParams base;
  FPD pitch_inertia = 3214;
  FPD roll_inertia = 2941;
  FPD suspension_stiffness = 30000;
  FPD suspension_damping = 3000;
  FPD suspension_rest_height = 0.15;
  FPD center_of_mass_height = 0.5;
  FPD body_length = 4.5, body_width = 1.8, body_height = 1.4;
  FPD unsprung_mass = 70;
  FPD pacejka_stiffness = 10.0, pacejka_peak = 1.9, pacejka_relax = -0.3;
  FPD friction_circle_slack = 1.0;
};
/** 3D contacts: 4x surface + ground height per wheel. */
struct TCarContacts3D
{
  TVehicleSurface wheels[CarWheelCount];
  FPD ground_height[CarWheelCount] = {};
};
/** Derive pitch/roll inertia from mass + body dimensions. */
inline void TCarParams3DDerive(TCarParams3D& p) {
  FPD m = p.base.mass, L = p.body_length, W = p.body_width, H = p.body_height;
  p.pitch_inertia = 0.5 * m * (p.base.front_arm * p.base.front_arm +
      p.base.rear_arm * p.base.rear_arm) + (1.0 / 12.0) * m * (W * W + H * H);
  p.roll_inertia = (1.0 / 12.0) * m * (L * L + H * H) + m * p.center_of_mass_height * p.center_of_mass_height;
}
/** Pack 19 continuous DOF into v. */
inline void TCarState3DPack(const TCarState3D& s, FPD* v) {
  v[0] = s.x; v[1] = s.y; v[2] = s.heading; v[3] = s.forward;
  v[4] = s.lateral; v[5] = s.yaw_rate; v[6] = s.pitch;
  v[7] = s.pitch_rate; v[8] = s.roll; v[9] = s.roll_rate; v[10] = s.steer;
  for (ISC i = 0; i < CarWheelCount; ++i) {
    v[11 + i] = s.wheel_height[i];
    v[15 + i] = s.wheel_vel[i];
  }
}
inline BOL TCarParams3DValid(const TCarParams3D& p) {
  if (!TCarParamsValid(p.base)) return false;
  FPD pos[] = {p.pitch_inertia, p.roll_inertia, p.suspension_stiffness,
      p.suspension_damping, p.suspension_rest_height, p.center_of_mass_height,
      p.body_length, p.body_width, p.body_height, p.unsprung_mass};
  for (FPD v : pos) if (!isfinite(v) || v <= 0) return false;
  return isfinite(p.pacejka_stiffness) && p.pacejka_stiffness > 0 &&
      isfinite(p.pacejka_peak) && p.pacejka_peak > 0;
}
inline BOL TCarState3DValid(const TCarState3D& s) {
  FPD v[19]; TCarState3DPack(s, v);
  for (ISC i = 0; i < 19; ++i) if (!isfinite(v[i])) return false;
  for (ISC i = 0; i < CarWheelCount; ++i)
    if (!isfinite(s.vertical_tire_force[i]) || s.vertical_tire_force[i] < 0) return false;
  return fabs(s.x) <= 1e9 && fabs(s.y) <= 1e9 && fabs(s.pitch) < 1.5 &&
      fabs(s.roll) < 1.5 && s.forward >= 0 && s.forward <= 1e4;
}
/** Per-wheel 3D tire force (Pacejka lateral + friction circle). */
inline void TCarTire3D(const FPD* v, const TCarParams3D& p,
    const TCarControl& c, ISC wheel, const TVehicleSurface& surface,
    const FPD* fz, FPD* fx, FPD* fy, FPD* utilization) {
  FPD front_arm = p.base.front_arm, rear_arm = p.base.rear_arm;
  FPD track = p.base.track;
  BOL front = wheel < CarRearLeft;
  FPD x = front ? front_arm : -rear_arm;
  FPD y = (wheel % 2 == 0 ? 0.5 : -0.5) * track;
  FPD steer_i = front ? v[10] : 0;
  FPD cs = cos(steer_i), sn = sin(steer_i);
  FPD u = v[3] - v[5] * y, side = v[4] + v[5] * x;
  FPD u_tire = u * cs + side * sn, v_tire = side * cs - u * sn;
  FPD Fz = fz[wheel];
  if (Fz < 10) { *fx = 0; *fy = 0; *utilization = 0; return; }
  // Pacejka lateral: fy = -Fz * D * sin(C * atan(B*alpha - E*(B*alpha - atan(B*alpha))))
  FPD alpha = atan2(v_tire, TBikeMax(fabs(u_tire), 2.0));
  FPD B = p.pacejka_stiffness, C = p.pacejka_peak, E = p.pacejka_relax;
  FPD Balpha = B * alpha;
  FPD fy_pacejka = -Fz * sin(C * (Balpha - E * (Balpha - atan(Balpha))));
  // Longitudinal: commanded drive/brake (2D behaviour) with friction limit.
  FPD fx_cmd = 0;
  if (!front)
    fx_cmd = 0.5 * TBikeClamp(c.throttle, 0, 1) *
        TBikeMin(p.base.drive_force, p.base.drive_power / TBikeMax(v[3], 1.0));
  fx_cmd -= 0.5 * TBikeClamp(c.brake, 0, 1) * p.base.brake_force *
      (front ? p.base.brake_bias : 1 - p.base.brake_bias) * tanh(u_tire / 0.05);
  // Friction circle projection.
  FPD limit = surface.friction * Fz * p.friction_circle_slack;
  *utilization = TBikeTire(fx_cmd, fy_pacejka, limit);
  *fx = fx_cmd; *fy = fy_pacejka;
  // Rescale to circle if over limit.
  FPD mag = hypot(*fx, *fy);
  if (mag > limit && limit > 0) {
    FPD scale = limit / mag;
    *fx *= scale; *fy *= scale;
  }
}
/** Full 3D Newton-Euler derivative (19 DOF). */
inline void TCarDerivative3D(const FPD* v, const TCarParams3D& p,
    const TCarControl& c, const TCarContacts3D& contacts, FPD* d,
    FPD& utilization, FPD& ax, FPD& ay) {
  FPD front_arm = p.base.front_arm, rear_arm = p.base.rear_arm;
  FPD track = p.base.track, m = p.base.mass;
  FPD slope_x = 0, slope_y = 0;
  for (const TVehicleSurface& s : contacts.wheels) {
    slope_x += 0.25 * s.slope_x; slope_y += 0.25 * s.slope_y;
  }
  FPD g = BikeGravity;
  FPD normal_g = g / sqrt(1 + slope_x * slope_x + slope_y * slope_y);
  // Suspension + Fz (quasi-static with load transfer).
  FPD fz[CarWheelCount], h[CarWheelCount];
  FPD front_axle = front_arm, rear_axle = rear_arm;
  // Static per-wheel load (no transfer): 0.25 * m * g each.
  FPD static_fz = 0.25 * m * normal_g;
  // Longitudinal load transfer: m * ax * h_cg / wheelbase (front gets -ax, rear gets +ax).
  FPD lt_long = m * ax * p.center_of_mass_height / (front_axle + rear_axle);
  // Lateral load transfer: m * ay * h_cg / track (left gets -ay, right gets +ay).
  FPD lt_lat = m * ay * p.center_of_mass_height / track;
  for (ISC i = 0; i < CarWheelCount; ++i) {
    BOL is_front = i < CarRearLeft;
    BOL is_left = (i % 2 == 0);
    // Load transfer: braking (ax<0) shifts load forward; left turn (ay>0) shifts right.
    FPD transfer = (is_front ? -lt_long : lt_long) * 0.5 +
                   (is_left ? -lt_lat : lt_lat) * 0.5;
    // Clamp so no wheel goes negative.
    fz[i] = TBikeMax(static_fz + transfer, 0);
    // Quasi-static compression from the spring force.
    h[i] = p.suspension_rest_height - fz[i] / p.suspension_stiffness;
    // Damping term (uses current wheel_vel from state).
    h[i] -= p.suspension_damping * v[15 + i] / p.suspension_stiffness;
    h[i] = TBikeClamp(h[i], -p.suspension_rest_height, 2 * p.suspension_rest_height);
  }
  // Tire forces.
  FPD fx_total = 0, fy_total = 0, moment = 0, rolling = 0;
  utilization = 0;
  FPD fx_w[CarWheelCount], fy_w[CarWheelCount];
  for (ISC i = 0; i < CarWheelCount; ++i) {
    FPD util;
    TCarTire3D(v, p, c, i, contacts.wheels[i], fz, &fx_w[i], &fy_w[i], &util);
    utilization = TBikeMax(utilization, util);
    FPD steer_i = i < CarRearLeft ? v[10] : 0;
    FPD cs = cos(steer_i), sn = sin(steer_i);
    FPD bx = fx_w[i] * cs - fy_w[i] * sn;
    FPD by = fx_w[i] * sn + fy_w[i] * cs;
    fx_total += bx; fy_total += by;
    FPD x = i < CarRearLeft ? front_arm : -rear_arm;
    FPD y = (i % 2 == 0 ? 0.5 : -0.5) * track;
    moment += x * by - y * bx;
    rolling += fz[i] * contacts.wheels[i].rolling * tanh(v[3] / 0.2);
  }
  FPD ch = cos(v[2]), sh = sin(v[2]);
  FPD drag = 0.5 * p.base.air_density * p.base.drag_area * v[3] * fabs(v[3]);
  ax = (fx_total - drag - rolling) / m -
      normal_g * (slope_x * ch + slope_y * sh);
  ay = fy_total / m - normal_g * (-slope_x * sh + slope_y * ch);
  // 2D kinematics (unchanged).
  d[0] = v[3] * ch - v[4] * sh;
  d[1] = v[3] * sh + v[4] * ch;
  d[2] = v[5];
  d[3] = ax + v[4] * v[5];
  d[4] = ay - v[3] * v[5];
  d[5] = moment / p.base.yaw_inertia;
  // Pitch: inverted-pendulum form (same shape as TBikeDerivative roll).
  // Accel (ax>0) -> nose up (pitch>0). Brake (ax<0) -> nose down (pitch<0).
  // Coupling gain tuned for realistic magnitudes (~1-2 deg under hard accel).
  d[6] = v[7];
  d[7] = (normal_g * sin(v[6]) + ax * cos(v[6])) * (p.center_of_mass_height / p.pitch_inertia) * 100.0;
  // Roll: inverted-pendulum form. Lateral accel (ay>0, left turn) -> roll.
  d[8] = v[9];
  d[9] = (normal_g * sin(v[8]) - ay * cos(v[8])) * (p.center_of_mass_height / p.roll_inertia) * 200.0;
  // Steer (first-order lag, unchanged from 2D).
  d[10] = TBikeClamp((TBikeClamp(c.steer, -p.base.steer_limit, p.base.steer_limit) -
      v[10]) / p.base.steer_tau, -p.base.steer_rate, p.base.steer_rate);
  // Suspension velocity (quasi-static: wheel_vel tracks the compression rate).
  // Not a stiff spring ODE — the body pitch/roll 2nd-order carries the dynamics.
  for (ISC i = 0; i < CarWheelCount; ++i) {
    FPD target_vel = (fz[i] - 0.25 * m * normal_g) / (0.25 * m);
    // Low-pass: wheel_vel relaxes toward the target (stable, no oscillation).
    d[11 + i] = v[15 + i];  // h_dot = wheel_vel (state carried for obs).
    d[15 + i] = (target_vel - v[15 + i]) / p.suspension_damping;
  }
}
/** RK4 3D step. dt in (0,0.01]; returns false without mutating on invalid input. */
inline BOL TCarStep3D(TCarState3D& car, const TCarParams3D& p,
    const TCarControl& c, const TCarContacts3D& contacts, FPD dt) {
  if (!isfinite(dt) || dt <= 0 || dt > 0.01 || !TCarParams3DValid(p) ||
      !TCarState3DValid(car) || !isfinite(c.steer) || !isfinite(c.throttle) ||
      !isfinite(c.brake)) return false;
  for (const TVehicleSurface& s : contacts.wheels)
    if (!TBikeSurfaceValid(s) || s.friction > 2 || s.rolling > 1 ||
        fabs(s.slope_x) > 1 || fabs(s.slope_y) > 1) return false;
  FPD v[19], work[19], k1[19], k2[19], k3[19], k4[19];
  TCarState3DPack(car, v);
  FPD utilization = 0, ax = 0, ay = 0;
  TCarDerivative3D(v, p, c, contacts, k1, utilization, ax, ay);
  for (ISC i = 0; i < 19; ++i) work[i] = v[i] + 0.5 * dt * k1[i];
  TCarDerivative3D(work, p, c, contacts, k2, utilization, ax, ay);
  for (ISC i = 0; i < 19; ++i) work[i] = v[i] + 0.5 * dt * k2[i];
  TCarDerivative3D(work, p, c, contacts, k3, utilization, ax, ay);
  for (ISC i = 0; i < 19; ++i) work[i] = v[i] + dt * k3[i];
  TCarDerivative3D(work, p, c, contacts, k4, utilization, ax, ay);
  for (ISC i = 0; i < 19; ++i) {
    v[i] += dt / 6 * (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]);
    if (!isfinite(v[i])) return false;
  }
  v[3] = TBikeMax(v[3], 0);
  TCarDerivative3D(v, p, c, contacts, work, utilization, ax, ay);
  if (!isfinite(utilization) || !isfinite(ax) || !isfinite(ay)) return false;
  TCarState3D next = car;
  next.x = v[0]; next.y = v[1]; next.heading = TBikeAngle(v[2]);
  next.forward = v[3]; next.lateral = v[4]; next.yaw_rate = v[5];
  next.pitch = v[6]; next.pitch_rate = v[7];
  next.roll = v[8]; next.roll_rate = v[9];
  next.steer = v[10];
  for (ISC i = 0; i < CarWheelCount; ++i) {
    next.wheel_height[i] = v[11 + i];
    next.wheel_vel[i] = v[15 + i];
  }
  next.time += dt; next.distance += hypot(next.x - car.x, next.y - car.y);
  next.longitudinal_accel = ax; next.lateral_accel = ay; next.utilization = utilization;
  // Store Fz from the derivative (recompute with load transfer).
  FPD slope_x2 = 0, slope_y2 = 0;
  for (const TVehicleSurface& s : contacts.wheels) {
    slope_x2 += 0.25 * s.slope_x; slope_y2 += 0.25 * s.slope_y;
  }
  FPD normal_g2 = BikeGravity / sqrt(1 + slope_x2 * slope_x2 + slope_y2 * slope_y2);
  FPD static_fz2 = 0.25 * p.base.mass * normal_g2;
  FPD lt_long2 = p.base.mass * ax * p.center_of_mass_height / (p.base.front_arm + p.base.rear_arm);
  FPD lt_lat2 = p.base.mass * ay * p.center_of_mass_height / p.base.track;
  for (ISC i = 0; i < CarWheelCount; ++i) {
    BOL is_front = i < CarRearLeft;
    BOL is_left = (i % 2 == 0);
    FPD transfer = (is_front ? -lt_long2 : lt_long2) * 0.5 +
                   (is_left ? -lt_lat2 : lt_lat2) * 0.5;
    next.vertical_tire_force[i] = TBikeMax(static_fz2 + transfer, 0);
  }
  if (!TCarState3DValid(next)) return false;
  car = next;
  return true;
}
} //< namespace iGeek
#endif
#endif
