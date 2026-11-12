// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekArmWorldPhysicsHXX
#define _iGeekArmWorldPhysicsHXX
#include "ArmPhysics.h"
#include "../../../CrabsTK/Math/Blas.h"
#include "../../CrabsTK/Math/Blas.h"
#if _Seam >= _iGeekArmWorldCore
namespace iGeek {
using namespace ::_;
using namespace ::CTK;

/* Arm dynamics row-major layout (2 x 2 inertia, 2-vector force). */
enum { ArmMatSize = 2, ArmVecSize = 2 };
/* Closed-form two-link forward kinematics from absolute joint angles. */
void ArmFK(ArmParams& params, FPD j1, FPD j2,
           FPD& tip_x, FPD& tip_y) {
  FPD c1 = cos(j1), s1 = sin(j1), c2 = cos(j2), s2 = sin(j2);
  tip_x = params.length_1 * c1 + params.length_2 * c2;
  tip_y = params.length_1 * s1 + params.length_2 * s2;
}
/* Absolute link angles measured from +x, y up, gravity down.
COMs lie at half length; inertia_i is about each link's COM.
M12 = m2*l1*lc2*cos(j1-j2). The RHS centrifugal coefficient is
-m2*l1*lc2*sin(j1-j2). Intermediate RK stages may cross a stop.
@return false when inputs are invalid; forces are untouched. */
BOL ArmForces(ArmParams& params, ArmState& state,
              ArmDynamicsTerms& forces) {
  if (!ArmParamsValid(params) || !iGeek::IsFinite(state.j1) ||
      !iGeek::IsFinite(state.j2)) return false;
  FPD lc1 = 0.5 * params.length_1, lc2 = 0.5 * params.length_2;
  ArmDynamicsTerms candidate;
  candidate.I11 = params.inertia_1 + params.mass_1 * lc1 * lc1 +
      params.mass_2 * params.length_1 * params.length_1;
  candidate.I22 = params.inertia_2 + params.mass_2 * lc2 * lc2;
  candidate.I12_base = params.mass_2 * params.length_1 * lc2;
  candidate.I12 = candidate.I12_base * cos(state.j1 - state.j2);
  candidate.c12 = -candidate.I12_base * sin(state.j1 - state.j2);
  candidate.grav_1 = -(params.mass_1 * lc1 + params.mass_2 * params.length_1) *
      9.81 * cos(state.j1);
  candidate.grav_2 = -params.mass_2 * lc2 * 9.81 * cos(state.j2);
  if (!iGeek::IsFinite(candidate.I11) || !iGeek::IsFinite(candidate.I22) ||
      !iGeek::IsFinite(candidate.I12) || !iGeek::IsFinite(candidate.c12) ||
      !iGeek::IsFinite(candidate.grav_1) || !iGeek::IsFinite(candidate.grav_2))
    return false;
  forces = candidate;
  return true;
}
/* One dynamics evaluation: solve I(w) a = tau + C(w) w + g for a.
Uses Math::GemmRR/GemvR (2 x 2) + explicit 2 x 2 inverse.
@return false for invalid inputs; acc is untouched. */
BOL TArmDynamics(ArmParams& params, FPD j1, FPD j2, FPD w1, FPD w2,
                FPD tau_1, FPD tau_2, FPD* acc) {
  if (!ArmParamsValid(params)) return false;
  if (!iGeek::IsFinite(j1) || !iGeek::IsFinite(j2) || !iGeek::IsFinite(w1) || !iGeek::IsFinite(w2) ||
      !iGeek::IsFinite(tau_1) || !iGeek::IsFinite(tau_2)) return false;
  ArmDynamicsTerms forces = ArmDynamicsTerms();
  ArmState state;
  state.j1 = j1; state.j2 = j2; state.w1 = w1; state.w2 = w2;
  FPD fx = 0, fy = 0;
  ArmFK(params, j1, j2, fx, fy);
  state.tip_x = fx; state.tip_y = fy; state.reach = hypot(fx, fy);
  if (!ArmForces(params, state, forces)) return false;
  FPD I[ArmMatSize * ArmMatSize] = {
      forces.I11, forces.I12, forces.I12, forces.I22};
  FPD f[ArmVecSize] = {
      tau_1 + forces.c12 * w2 * w2 - params.damping * w1 + forces.grav_1,
      tau_2 - forces.c12 * w1 * w1 - params.damping * w2 + forces.grav_2};
  FPD Iinv[ArmMatSize * ArmMatSize], a[ArmVecSize];
  FPD det = I[0] * I[3] - I[1] * I[2];
  if (!iGeek::IsFinite(det) || det <= 0) return false;
  Iinv[0] = I[3] / det; Iinv[1] = -I[1] / det;
  Iinv[2] = -I[2] / det; Iinv[3] = I[0] / det;
  CTK::Math::GemvR(1.0, Iinv, ArmMatSize, ArmMatSize, ArmMatSize, f, 0.0, a);
  if (!iGeek::IsFinite(a[0]) || !iGeek::IsFinite(a[1])) return false;
  acc[0] = a[0]; acc[1] = a[1];
  return true;
}
BOL ArmParamsValid(ArmParams& params) {
  return iGeek::IsFinite(params.length_1) && params.length_1 > 0 &&
      iGeek::IsFinite(params.length_2) && params.length_2 > 0 &&
      iGeek::IsFinite(params.mass_1) && params.mass_1 > 0 &&
      iGeek::IsFinite(params.mass_2) && params.mass_2 > 0 &&
      iGeek::IsFinite(params.inertia_1) && params.inertia_1 > 0 &&
      iGeek::IsFinite(params.inertia_2) && params.inertia_2 > 0 &&
      iGeek::IsFinite(params.max_torque) && params.max_torque > 0 &&
      iGeek::IsFinite(params.damping) && params.damping >= 0 &&
      iGeek::IsFinite(params.dt) && params.dt > 0 && params.dt < 0.1 &&
      iGeek::IsFinite(params.half_step) && params.half_step > 0 &&
      params.half_step <= params.dt && params.dt / params.half_step <= 4096;
}
BOL ArmStateValid(ArmState& state) {
  FPD jmax = 1.5707963267948966;
  return iGeek::IsFinite(state.j1) && iGeek::IsFinite(state.j2) &&
      iGeek::IsFinite(state.w1) && iGeek::IsFinite(state.w2) &&
      iGeek::IsFinite(state.time) && iGeek::IsFinite(state.tip_x) && iGeek::IsFinite(state.tip_y) &&
      iGeek::IsFinite(state.reach) && Absolute(state.w1) < 1e6 && Absolute(state.w2) < 1e6 &&
      state.j1 >= -1e-9 && state.j1 <= jmax + 1e-9 &&
      state.j2 >= -1e-9 && state.j2 <= jmax + 1e-9;
}
/* Count bounded RK4 substeps without unsafe float-to-integer conversion.
Call only after ArmParamsValid. */
ISC TArmSubsteps(ArmParams& params) {
  FPD ratio = params.dt / params.half_step;
  ISC count = ISC(ratio);
  return count + (FPD(count) < ratio ? 1 : 0);
}
/* One frame advances exactly dt; half_step is the maximum RK4 substep.
Joint angles are clamped to [0, pi/2]; velocity zeros at the bound.
@return false for invalid inputs; state is untouched. */
BOL ArmStep(ArmState& state, ArmParams& params,
            ArmControl& control) {
  if (!ArmParamsValid(params) || !ArmStateValid(state) ||
      !iGeek::IsFinite(control.tau_1) || !iGeek::IsFinite(control.tau_2)) return false;
  FPD tau_1 = control.tau_1 > params.max_torque ? params.max_torque
      : control.tau_1 < -params.max_torque ? -params.max_torque : control.tau_1;
  FPD tau_2 = control.tau_2 > params.max_torque ? params.max_torque
      : control.tau_2 < -params.max_torque ? -params.max_torque : control.tau_2;
  FPD x[4] = {state.j1, state.j2, state.w1, state.w2};
  ISC count = TArmSubsteps(params);
  FPD h = params.dt / FPD(count);
  for (ISC sub = 0; sub < count; ++sub) {
    FPD k1[4], k2[4], k3[4], k4[4];
    FPD a[2];
    // k1 at x
    if (!TArmDynamics(params, x[0], x[1], x[2], x[3], tau_1, tau_2, a)) return false;
    k1[0] = x[2]; k1[1] = x[3]; k1[2] = a[0]; k1[3] = a[1];
    // k2 at x + h/2*k1
    FPD x2[4];
    for (ISC v = 0; v < 4; ++v) x2[v] = x[v] + h / 2 * k1[v];
    if (!TArmDynamics(params, x2[0], x2[1], x2[2], x2[3], tau_1, tau_2, a)) return false;
    k2[0] = x2[2]; k2[1] = x2[3]; k2[2] = a[0]; k2[3] = a[1];
    // k3 at x + h/2*k2
    FPD x3[4];
    for (ISC v = 0; v < 4; ++v) x3[v] = x[v] + h / 2 * k2[v];
    if (!TArmDynamics(params, x3[0], x3[1], x3[2], x3[3], tau_1, tau_2, a)) return false;
    k3[0] = x3[2]; k3[1] = x3[3]; k3[2] = a[0]; k3[3] = a[1];
    // k4 at x + h*k3
    FPD x4[4];
    for (ISC v = 0; v < 4; ++v) x4[v] = x[v] + h * k3[v];
    if (!TArmDynamics(params, x4[0], x4[1], x4[2], x4[3], tau_1, tau_2, a)) return false;
    k4[0] = x4[2]; k4[1] = x4[3]; k4[2] = a[0]; k4[3] = a[1];
    // x += h/6*(k1 + 2*k2 + 2*k3 + k4)
    for (ISC v = 0; v < 4; ++v)
      x[v] += h / 6 * (k1[v] + 2 * k2[v] + 2 * k3[v] + k4[v]);
    for (ISC v = 0; v < 4; ++v)
      if (!iGeek::IsFinite(x[v])) return false;
  }
  FPD jmax = 1.5707963267948966;
  if (x[0] < 0) { x[0] = 0; if (x[2] < 0) x[2] = 0; }
  if (x[0] > jmax) { x[0] = jmax; if (x[2] > 0) x[2] = 0; }
  if (x[1] < 0) { x[1] = 0; if (x[3] < 0) x[3] = 0; }
  if (x[1] > jmax) { x[1] = jmax; if (x[3] > 0) x[3] = 0; }
  ArmState candidate = state;
  candidate.j1 = x[0]; candidate.j2 = x[1];
  candidate.w1 = x[2]; candidate.w2 = x[3];
  candidate.time += params.dt;
  ArmFK(params, candidate.j1, candidate.j2, candidate.tip_x, candidate.tip_y);
  candidate.reach = hypot(candidate.tip_x, candidate.tip_y);
  if (!ArmStateValid(candidate)) return false;
  state = candidate;
  return true;
}
} //< namespace iGeek
#endif
#endif
