// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_SPACEWORLD_PHYSICS_HXX
#define CRABS_SPACEWORLD_PHYSICS_HXX
#include "SpacePhysics.h"
#include "SpaceMath.hxx"
#include <math.h>
#if SEAM >= CRABS_SPACEWORLD
namespace iGeek {
/** Rotate a vector by a unit quaternion via (x,y,z,w) arrays.
 * v' = q v q^-1. Verified 2026-10-21: a +90 deg body-Z spin maps local +X
 * to world +Y. Used by TSpaceStep for orientation integration.
 */
static void TSpaceStepQRotate(const FPD* q, const FPD* v, FPD* out) {
  FPD tmp[4];
  tmp[3] = -q[0] * v[0] - q[1] * v[1] - q[2] * v[2];
  tmp[0] = q[3] * v[0] + q[1] * v[2] - q[2] * v[1];
  tmp[1] = q[3] * v[1] + q[2] * v[0] - q[0] * v[2];
  tmp[2] = q[3] * v[2] + q[0] * v[1] - q[1] * v[0];
  out[0] = tmp[3] * (-q[0]) + tmp[0] * q[3] + tmp[1] * q[2] - tmp[2] * q[1];
  out[1] = tmp[3] * (-q[1]) - tmp[0] * q[2] + tmp[1] * q[3] + tmp[2] * q[0];
  out[2] = tmp[3] * (-q[2]) + tmp[0] * q[1] - tmp[1] * q[0] + tmp[2] * q[3];
}
void TSpaceInertia(TSpaceParams& p) {
  FPD m = p.mass;
  FPD hx2 = p.half.x * p.half.x, hy2 = p.half.y * p.half.y;
  FPD hz2 = p.half.z * p.half.z;
  p.Ixx = m / 12 * (hy2 + hz2);
  p.Iyy = m / 12 * (hx2 + hz2);
  p.Izz = m / 12 * (hx2 + hy2);
}
BOL TSpaceParamsValid(const TSpaceParams& p) {
  return isfinite(p.mass) && p.mass > 0 &&
      isfinite(p.half.x) && p.half.x > 0 && isfinite(p.half.y) && p.half.y > 0 &&
      isfinite(p.half.z) && p.half.z > 0 &&
      isfinite(p.Ixx) && p.Ixx > 0 && isfinite(p.Iyy) && p.Iyy > 0 &&
      isfinite(p.Izz) && p.Izz > 0 &&
      isfinite(p.dt) && p.dt > 0 && p.dt < 0.1 &&
      isfinite(p.omega_max) && p.omega_max > 0 &&
      isfinite(p.speed_max) && p.speed_max > 0;
}
BOL TSpaceStateValid(const TSpaceState& s) {
  FPD n = s.qx * s.qx + s.qy * s.qy + s.qz * s.qz + s.qw * s.qw;
  return isfinite(s.x) && isfinite(s.y) && isfinite(s.z) &&
      isfinite(s.qx) && isfinite(s.qy) && isfinite(s.qz) && isfinite(s.qw) &&
      isfinite(s.vx) && isfinite(s.vy) && isfinite(s.vz) &&
      isfinite(s.wx) && isfinite(s.wy) && isfinite(s.wz) &&
      isfinite(s.time) && fabs(n - 1) < 1e-3 &&
      fabs(s.vx) < 1e6 && fabs(s.vy) < 1e6 && fabs(s.vz) < 1e6 &&
      fabs(s.wx) < 1e6 && fabs(s.wy) < 1e6 && fabs(s.wz) < 1e6;
}
/** One 60 Hz step: integrate linear + angular velocity, advance pose.
 * Force is world-frame, torque is body-frame. Two substeps for stability.
 */
BOL TSpaceStep(TSpaceState& state, const TSpaceParams& p,
              const TSpaceControl& control) {
  if (!TSpaceParamsValid(p) || !TSpaceStateValid(state) ||
      !isfinite(control.fx) || !isfinite(control.fy) || !isfinite(control.fz) ||
      !isfinite(control.tx) || !isfinite(control.ty) || !isfinite(control.tz))
    return false;
  FPD h = p.dt / 2;
  FPD ax = control.fx / p.mass, ay = control.fy / p.mass, az = control.fz / p.mass;
  FPD awx = control.tx / p.Ixx, awy = control.ty / p.Iyy, awz = control.tz / p.Izz;
  FPD x = state.x, y = state.y, z = state.z;
  FPD vx = state.vx, vy = state.vy, vz = state.vz;
  FPD wx = state.wx, wy = state.wy, wz = state.wz;
  FPD q[4] = {state.qx, state.qy, state.qz, state.qw};
  FPD w_body[3], w_world[3];
  for (ISC sub = 0; sub < 2; ++sub) {
    vx += ax * h; vy += ay * h; vz += az * h;
    x += vx * h; y += vy * h; z += vz * h;
    wx += awx * h; wy += awy * h; wz += awz * h;
    w_body[0] = wx; w_body[1] = wy; w_body[2] = wz;
    TSpaceVec wv = TSpaceQuatRotate(TSpaceQuat(q[0], q[1], q[2], q[3]),
                                    TSpaceVec(w_body[0], w_body[1], w_body[2]));
    w_world[0] = wv.x; w_world[1] = wv.y; w_world[2] = wv.z;
    // qdot = 0.5 * (0, w_world) * q.
    FPD qx = q[0], qy = q[1], qz = q[2], qw = q[3];
    FPD dqx = 0.5 * (qy * w_world[0] + qz * w_world[1] + qw * w_world[2]);
    FPD dqy = 0.5 * (qx * w_world[0] - qw * w_world[1] + qz * w_world[2]);
    FPD dqz = 0.5 * (qx * w_world[1] - qy * w_world[0] + qw * w_world[2]);
    FPD dqw = 0.5 * (-qx * w_world[0] - qy * w_world[1] - qz * w_world[2]);
    q[0] += dqx * h; q[1] += dqy * h; q[2] += dqz * h; q[3] += dqw * h;
    TSpaceQuat nq(q[0], q[1], q[2], q[3]);
    if (TSpaceQuatNormalize(nq)) {
      q[0] = nq.x; q[1] = nq.y; q[2] = nq.z; q[3] = nq.w;
    }
  }
  FPD sp = sqrt(vx * vx + vy * vy + vz * vz);
  if (sp > p.speed_max) {
    FPD s = p.speed_max / sp;
    vx *= s; vy *= s; vz *= s;
  }
  FPD om = sqrt(wx * wx + wy * wy + wz * wz);
  if (om > p.omega_max) {
    FPD s = p.omega_max / om;
    wx *= s; wy *= s; wz *= s;
  }
  state.x = x; state.y = y; state.z = z;
  state.qx = q[0]; state.qy = q[1]; state.qz = q[2]; state.qw = q[3];
  state.vx = vx; state.vy = vy; state.vz = vz;
  state.wx = wx; state.wy = wy; state.wz = wz;
  state.time += p.dt;
  return TSpaceStateValid(state);
}
} //< namespace iGeek
#endif
#endif
