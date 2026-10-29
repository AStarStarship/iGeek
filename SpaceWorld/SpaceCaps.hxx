// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_SPACEWORLD_CAPS_HXX
#define CRABS_SPACEWORLD_CAPS_HXX
#include "SpaceCaps.h"
#include <math.h>
#if SEAM >= CRABS_SPACEWORLD
namespace iGeek {
FPD TSpaceCapsInertiaAxis(FPD mass, const TSpaceHalf& half, ISC a) {
  FPD hx2 = half.x * half.x, hy2 = half.y * half.y, hz2 = half.z * half.z;
  if (a == 0) return mass / 12 * (hy2 + hz2);
  if (a == 1) return mass / 12 * (hx2 + hz2);
  return mass / 12 * (hx2 + hy2);
}
FPD TSpaceCapsEffInertia(const TSpaceCaps& caps, const TSpaceVec& dir) {
  TSpaceVec d = TSpaceVecNormalize(dir);
  FPD nx = d.x, ny = d.y, nz = d.z;
  return caps.inertia.x * nx * nx + caps.inertia.y * ny * ny +
         caps.inertia.z * nz * nz;
}
static FPD TSpaceCapsGroupTorque(const TSpaceThruster thr[SpaceRcsTotal],
                                 const FPD thrust_max[SpaceRcsTotal],
                                 const ISC idx[], ISC count, ISC axis) {
  FPD tau = 0;
  for (ISC i = 0; i < count; ++i) {
    ISC id = idx[i];
    FPD cap = thrust_max[id] > 0 ? thrust_max[id] : 1;
    // Torque about body axis = | (pos x force) . axis | * thrust.
    TSpaceVec cross = TSpaceVecCross(thr[id].position, thr[id].force_dir);
    FPD ax = axis == 0 ? cross.x : (axis == 1 ? cross.y : cross.z);
    tau += TSpaceAbs(ax) * cap;
  }
  return tau;
}
static FPD TSpaceCapsGroupForce(const FPD thrust_max[SpaceRcsTotal],
                                const ISC idx[], ISC count) {
  FPD s = 0;
  for (ISC i = 0; i < count; ++i)
    s += thrust_max[idx[i]] > 0 ? thrust_max[idx[i]] : 1;
  return s;
}
void TSpaceCapsCompute(TSpaceCaps& caps, FPD mass, const TSpaceHalf& half,
                       const TSpaceThruster thr[SpaceRcsTotal],
                       const FPD thrust_max[SpaceRcsTotal]) {
  TSpaceGroups g;
  caps.inertia.x = TSpaceCapsInertiaAxis(mass, half, 0);
  caps.inertia.y = TSpaceCapsInertiaAxis(mass, half, 1);
  caps.inertia.z = TSpaceCapsInertiaAxis(mass, half, 2);
  // Linear force per axis = max of the two direction groups.
  caps.lin_force.x = TSpaceMax(TSpaceCapsGroupForce(thrust_max, g.left[0], 4),
                               TSpaceCapsGroupForce(thrust_max, g.left[1], 4));
  caps.lin_force.y = TSpaceMax(TSpaceCapsGroupForce(thrust_max, g.up[0], 4),
                               TSpaceCapsGroupForce(thrust_max, g.up[1], 4));
  caps.lin_force.z = TSpaceMax(TSpaceCapsGroupForce(thrust_max, g.forward[0], 4),
                               TSpaceCapsGroupForce(thrust_max, g.forward[1], 4));
  FPD m = mass > 1e-6 ? mass : 1e-6;
  caps.lin_accel.x = caps.lin_force.x / m;
  caps.lin_accel.y = caps.lin_force.y / m;
  caps.lin_accel.z = caps.lin_force.z / m;
  // Angular torque per axis = max of the two rotation groups.
  caps.ang_torque.x = TSpaceMax(
      TSpaceCapsGroupTorque(thr, thrust_max, g.pitch[0], 8, 0),
      TSpaceCapsGroupTorque(thr, thrust_max, g.pitch[1], 8, 0));
  caps.ang_torque.y = TSpaceMax(
      TSpaceCapsGroupTorque(thr, thrust_max, g.yaw[0], 4, 1),
      TSpaceCapsGroupTorque(thr, thrust_max, g.yaw[1], 4, 1));
  caps.ang_torque.z = TSpaceMax(
      TSpaceCapsGroupTorque(thr, thrust_max, g.roll[0], 4, 2),
      TSpaceCapsGroupTorque(thr, thrust_max, g.roll[1], 4, 2));
  caps.ang_accel.x = caps.ang_torque.x / (caps.inertia.x > 1e-6 ? caps.inertia.x : 1e-6);
  caps.ang_accel.y = caps.ang_torque.y / (caps.inertia.y > 1e-6 ? caps.inertia.y : 1e-6);
  caps.ang_accel.z = caps.ang_torque.z / (caps.inertia.z > 1e-6 ? caps.inertia.z : 1e-6);
}
void TSpaceCapsAngCap(const TSpaceCaps& caps, FPD max_angular_accel,
                      FPD max_angular_velocity, FPD& alpha_max,
                      FPD& omega_max) {
  FPD a_min = TSpaceMin(TSpaceMin(caps.ang_accel.x, caps.ang_accel.y),
                        caps.ang_accel.z);
  FPD alpha_dyn = TSpaceMax(1e-4, a_min) * 0.6;
  alpha_max = TSpaceMin(max_angular_accel, alpha_dyn);
  FPD omega_from_alpha = sqrt(2 * alpha_max * 0.5);
  omega_max = TSpaceMin(max_angular_velocity,
                        TSpaceMax(0.2, omega_from_alpha));
}
} //< namespace iGeek
#endif
#endif
