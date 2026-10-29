// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_ARMWORLD_PHYSICS_HXX
#define CRABS_ARMWORLD_PHYSICS_HXX
#include "ArmPhysics.h"
#include "../../CrabsTK/Math/Blas.h"
#include <math.h>
#if SEAM >= CRABS_ARMWORLD
namespace iGeek {
/* Arm dynamics row-major layout (2 x 2 inertia, 2-vector force). */
enum { ArmMatSize = 2, ArmVecSize = 2 };
/* Closed-form two-link forward kinematics from absolute joint angles. */
void TArmFK(const TArmParams& params, FPD j1, FPD j2,
           FPD& tip_x, FPD& tip_y) {
  FPD c1 = cos(j1), s1 = sin(j1), c2 = cos(j2), s2 = sin(j2);
  tip_x = params.length_1 * c1 + params.length_2 * c2;
  tip_y = params.length_1 * s1 + params.length_2 * s2;
}
/* Point masses on uniform links (mass at 0.5 l), gravity in +y.
I11 is constant; I12 = I12_base + (m2/4) l1 l2 (cos j1 + cos j2);
Coriolis c12 = -(m2/2) l1 l2 sin(j1 - j2).
@return false when inputs are invalid; forces are untouched. */
BOL TArmForces(const TArmParams& params, const TArmState& state,
              TArmDynamicsTerms& forces) {
  if (!TArmParamsValid(params) || !TArmStateValid(state)) return false;
  FPD g = 9.81;
  FPD lc = 0.5 * params.length_1, lc2 = 0.5 * params.length_2;
  FPD mc = 0.5 * params.mass_2;
  FPD c1 = cos(state.j1), s1 = sin(state.j1), c2 = cos(state.j2), s2 = sin(state.j2);
  forces.I11 = params.inertia_1 + params.mass_2 * params.length_1 * params.length_1 +
      mc * lc * lc + params.mass_2 * lc2 * lc2;
  forces.I12_base = params.inertia_2 + params.mass_2 * lc2 * lc2;
  forces.I22 = params.inertia_2 + params.mass_2 * lc2 * lc2;
  forces.I12 = forces.I12_base +
      mc * lc * lc2 * (c1 + c2);
  forces.c12 = -mc * lc * lc2 * sin(state.j1 - state.j2);
  forces.grav_1 = -(params.mass_1 * lc) * g * s1 -
      params.mass_2 * g * (params.length_1 * s1 + lc2 * s2);
  forces.grav_2 = -(params.mass_2 * lc2) * g * s2;
  return true;
}
/* One dynamics evaluation: solve I(w) a = tau + C(w) w + g for a.
Uses Math::GemmRR/GemvR (2 x 2) + explicit 2 x 2 inverse.
@return false for invalid inputs; acc is untouched. */
BOL TArmDynamics(const TArmParams& params, FPD j1, FPD j2, FPD w1, FPD w2,
                FPD tau_1, FPD tau_2, FPD* acc) {
  if (!TArmParamsValid(params)) return false;
  if (!isfinite(j1) || !isfinite(j2) || !isfinite(w1) || !isfinite(w2) ||
      !isfinite(tau_1) || !isfinite(tau_2)) return false;
  TArmDynamicsTerms forces = TArmDynamicsTerms();
  TArmState state;
  state.j1 = j1; state.j2 = j2; state.w1 = w1; state.w2 = w2;
  FPD fx = 0, fy = 0;
  TArmFK(params, j1, j2, fx, fy);
  state.tip_x = fx; state.tip_y = fy; state.reach = hypot(fx, fy);
  if (!TArmForces(params, state, forces)) return false;
  FPD I[ArmMatSize * ArmMatSize] = {
      forces.I11, forces.I12, forces.I12, forces.I22};
  FPD f[ArmVecSize] = {
      tau_1 + forces.c12 * w2 * w2 - params.damping * w1 + forces.grav_1,
      tau_2 - forces.c12 * w1 * w1 - params.damping * w2 + forces.grav_2};
  FPD Iinv[ArmMatSize * ArmMatSize], a[ArmVecSize];
  FPD det = I[0] * I[3] - I[1] * I[2];
  if (!isfinite(det) || fabs(det) < 1e-12) return false;
  Iinv[0] = I[3] / det; Iinv[1] = -I[1] / det;
  Iinv[2] = -I[2] / det; Iinv[3] = I[0] / det;
  Math::GemvR(1.0, Iinv, ArmMatSize, ArmMatSize, ArmMatSize, f, 0.0, a);
  acc[0] = a[0]; acc[1] = a[1];
  return true;
}
BOL TArmParamsValid(const TArmParams& params) {
  return isfinite(params.length_1) && params.length_1 > 0 &&
      isfinite(params.length_2) && params.length_2 > 0 &&
      isfinite(params.mass_1) && params.mass_1 > 0 &&
      isfinite(params.mass_2) && params.mass_2 > 0 &&
      isfinite(params.inertia_1) && params.inertia_1 > 0 &&
      isfinite(params.inertia_2) && params.inertia_2 > 0 &&
      isfinite(params.max_torque) && params.max_torque > 0 &&
      isfinite(params.damping) && params.damping >= 0 &&
      isfinite(params.dt) && params.dt > 0 && params.dt < 0.1 &&
      isfinite(params.half_step) && params.half_step > 0 &&
      params.half_step < params.dt;
}
BOL TArmStateValid(const TArmState& state) {
  FPD jmax = 1.5707963267948966;
  return isfinite(state.j1) && isfinite(state.j2) &&
      isfinite(state.w1) && isfinite(state.w2) &&
      isfinite(state.time) && isfinite(state.tip_x) && isfinite(state.tip_y) &&
      isfinite(state.reach) && fabs(state.w1) < 1e6 && fabs(state.w2) < 1e6 &&
      state.j1 >= -1e-9 && state.j1 <= jmax + 1e-9 &&
      state.j2 >= -1e-9 && state.j2 <= jmax + 1e-9;
}
/* One 0.02 s frame = four 0.01 s RK4 substeps of the torque model.
Joint angles are clamped to [0, pi/2]; velocity zeros at the bound.
@return false for invalid inputs; state is untouched. */
BOL TArmStep(TArmState& state, const TArmParams& params,
            const TArmControl& control) {
  if (!TArmParamsValid(params) || !TArmStateValid(state) ||
      !isfinite(control.tau_1) || !isfinite(control.tau_2)) return false;
  FPD tau_1 = control.tau_1 > params.max_torque ? params.max_torque
      : control.tau_1 < -params.max_torque ? -params.max_torque : control.tau_1;
  FPD tau_2 = control.tau_2 > params.max_torque ? params.max_torque
      : control.tau_2 < -params.max_torque ? -params.max_torque : control.tau_2;
  FPD x[4] = {state.j1, state.j2, state.w1, state.w2};
  FPD h = params.half_step;
  for (ISC sub = 0; sub < 4; ++sub) {
    FPD k1[4], k2[4], k3[4], k4[4];
    FPD a[2];
    // k1 at x
    TArmDynamics(params, x[0], x[1], x[2], x[3], tau_1, tau_2, a);
    k1[0] = x[2]; k1[1] = x[3]; k1[2] = a[0]; k1[3] = a[1];
    // k2 at x + h/2*k1
    FPD x2[4];
    for (ISC v = 0; v < 4; ++v) x2[v] = x[v] + h / 2 * k1[v];
    TArmDynamics(params, x2[0], x2[1], x2[2], x2[3], tau_1, tau_2, a);
    k2[0] = x2[2]; k2[1] = x2[3]; k2[2] = a[0]; k2[3] = a[1];
    // k3 at x + h/2*k2
    FPD x3[4];
    for (ISC v = 0; v < 4; ++v) x3[v] = x[v] + h / 2 * k2[v];
    TArmDynamics(params, x3[0], x3[1], x3[2], x3[3], tau_1, tau_2, a);
    k3[0] = x3[2]; k3[1] = x3[3]; k3[2] = a[0]; k3[3] = a[1];
    // k4 at x + h*k3
    FPD x4[4];
    for (ISC v = 0; v < 4; ++v) x4[v] = x[v] + h * k3[v];
    TArmDynamics(params, x4[0], x4[1], x4[2], x4[3], tau_1, tau_2, a);
    k4[0] = x4[2]; k4[1] = x4[3]; k4[2] = a[0]; k4[3] = a[1];
    // x += h/6*(k1 + 2*k2 + 2*k3 + k4)
    for (ISC v = 0; v < 4; ++v)
      x[v] += h / 6 * (k1[v] + 2 * k2[v] + 2 * k3[v] + k4[v]);
  }
  FPD jmax = 1.5707963267948966;
  if (x[0] < 0) { x[0] = 0; if (x[2] < 0) x[2] = 0; }
  if (x[0] > jmax) { x[0] = jmax; if (x[2] > 0) x[2] = 0; }
  if (x[1] < 0) { x[1] = 0; if (x[3] < 0) x[3] = 0; }
  if (x[1] > jmax) { x[1] = jmax; if (x[3] > 0) x[3] = 0; }
  state.j1 = x[0]; state.j2 = x[1]; state.w1 = x[2]; state.w2 = x[3];
  state.time += params.dt;
  TArmFK(params, state.j1, state.j2, state.tip_x, state.tip_y);
  state.reach = hypot(state.tip_x, state.tip_y);
  if (!TArmStateValid(state)) {
    state.j1 = jmax; state.j2 = jmax; state.w1 = 0; state.w2 = 0;
    TArmFK(params, state.j1, state.j2, state.tip_x, state.tip_y);
    state.reach = hypot(state.tip_x, state.tip_y);
  }
  return true;
}
} //< namespace iGeek
#endif
#endif
