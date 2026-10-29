// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_SPACEWORLD_ALLOC_H
#define CRABS_SPACEWORLD_ALLOC_H
#include "SpaceConfig.h"
#include "SpaceMath.h"
#include "SpaceRcs.h"
#include "SpaceCaps.h"
#if SEAM >= CRABS_SPACEWORLD
namespace iGeek {
/** Distributes signed per-axis commands to the 24 RCS thrusters
 * (ThrusterAllocator.ts). Rotation works in the torque domain: a positive
 * axis command fires the group that produces that body-axis torque.
 * Translation works in the force domain: a positive axis command fires the
 * group that pushes that body axis. EMA smoothing + per-thruster capping.
 * Stateful (smoothed last commands).
 */
struct TSpaceAllocator
{
  FPD thrust = 312.0;                //< Per-thruster reference thrust, N.
  FPD thrust_max[SpaceRcsTotal] = {};  //< Per-thruster max, N.
  FPD epsilon = 0.5 * SpacePi / 180;  //< Attitude deadband, rad.
  FPD rot_alpha = 0.4;               //< Rotation smoothing (0 = none).
  FPD lin_alpha = 0.5;               //< Translation smoothing.
  FPD allocation_scale = 1.0;        //< Global scale [0,1].
  TSpaceVec last_rot;                //< Smoothed rotation command.
  TSpaceVec last_lin;                //< Smoothed translation command.
  TSpaceCaps caps;                   //< Capabilities (for torque/force caps).
  /** Fill `out` (SpaceRcsTotal, caller zeroed) with per-thruster demands
   * for a signed rotation torque command (body frame, N m).
   * out[idx] in [0, thrust_max[idx]].
   */
  void AllocateRotation(const TSpaceVec& torque, FPD out[SpaceRcsTotal]) {
    last_rot.x = last_rot.x * rot_alpha + torque.x * (1 - rot_alpha);
    last_rot.y = last_rot.y * rot_alpha + torque.y * (1 - rot_alpha);
    last_rot.z = last_rot.z * rot_alpha + torque.z * (1 - rot_alpha);
    FPD eps = epsilon * 2.0;
    TSpaceGroups g;
    // X axis (pitch): +x -> pitch[0] (+X torque), -x -> pitch[1].
    if (TSpaceAbs(last_rot.x) > eps) {
      ISC idx[8] = {};
      FPD tmax = TSpaceMax(1e-6, caps.ang_torque.x);
      FPD frac = TSpaceMin(1.0, TSpaceAbs(last_rot.x) / tmax);
      const ISC* grp = last_rot.x >= 0 ? g.pitch[0] : g.pitch[1];
      for (ISC i = 0; i < 8; ++i) {
        ISC id = grp[i];
        FPD cap = thrust_max[id] > 0 ? thrust_max[id] : thrust;
        FPD per = thrust * frac * allocation_scale;
        out[id] += TSpaceMin(cap, per);
      }
    }
    // Y axis (yaw): +y -> yaw[0], -y -> yaw[1].
    if (TSpaceAbs(last_rot.y) > eps) {
      FPD tmax = TSpaceMax(1e-6, caps.ang_torque.y);
      FPD frac = TSpaceMin(1.0, TSpaceAbs(last_rot.y) / tmax);
      const ISC* grp = last_rot.y >= 0 ? g.yaw[0] : g.yaw[1];
      for (ISC i = 0; i < 4; ++i) {
        ISC id = grp[i];
        FPD cap = thrust_max[id] > 0 ? thrust_max[id] : thrust;
        FPD per = thrust * frac * allocation_scale;
        out[id] += TSpaceMin(cap, per);
      }
    }
    // Z axis (roll): +z -> roll[1], -z -> roll[0] (roll[0] is -Z torque).
    if (TSpaceAbs(last_rot.z) > eps) {
      FPD tmax = TSpaceMax(1e-6, caps.ang_torque.z);
      FPD frac = TSpaceMin(1.0, TSpaceAbs(last_rot.z) / tmax);
      const ISC* grp = last_rot.z >= 0 ? g.roll[1] : g.roll[0];
      for (ISC i = 0; i < 4; ++i) {
        ISC id = grp[i];
        FPD cap = thrust_max[id] > 0 ? thrust_max[id] : thrust;
        FPD per = thrust * frac * allocation_scale;
        out[id] += TSpaceMin(cap, per);
      }
    }
  }
  /** Fill `out` (SpaceRcsTotal, caller zeroed) with per-thruster demands
   * for a signed translation force command (body frame, N).
   */
  void AllocateTranslation(const TSpaceVec& force, FPD out[SpaceRcsTotal]) {
    last_lin.x = last_lin.x * lin_alpha + force.x * (1 - lin_alpha);
    last_lin.y = last_lin.y * lin_alpha + force.y * (1 - lin_alpha);
    last_lin.z = last_lin.z * lin_alpha + force.z * (1 - lin_alpha);
    FPD eps = epsilon * 2.0;
    TSpaceGroups g;
    // Z (forward): +z -> forward[0], -z -> forward[1].
    if (TSpaceAbs(last_lin.z) > eps) {
      const ISC* grp = last_lin.z >= 0 ? g.forward[0] : g.forward[1];
      FPD sumcap = 0;
      for (ISC i = 0; i < 4; ++i)
        sumcap += thrust_max[grp[i]] > 0 ? thrust_max[grp[i]] : thrust;
      FPD total = TSpaceClamp(TSpaceAbs(last_lin.z), 0, sumcap);
      if (sumcap > 1e-6)
        for (ISC i = 0; i < 4; ++i) {
          ISC id = grp[i];
          FPD cap = thrust_max[id] > 0 ? thrust_max[id] : thrust;
          out[id] += TSpaceMin(cap, total * (cap / sumcap) * allocation_scale);
        }
    }
    // Y (up): +y -> up[0], -y -> up[1].
    if (TSpaceAbs(last_lin.y) > eps) {
      const ISC* grp = last_lin.y >= 0 ? g.up[0] : g.up[1];
      FPD sumcap = 0;
      for (ISC i = 0; i < 4; ++i)
        sumcap += thrust_max[grp[i]] > 0 ? thrust_max[grp[i]] : thrust;
      FPD total = TSpaceClamp(TSpaceAbs(last_lin.y), 0, sumcap);
      if (sumcap > 1e-6)
        for (ISC i = 0; i < 4; ++i) {
          ISC id = grp[i];
          FPD cap = thrust_max[id] > 0 ? thrust_max[id] : thrust;
          out[id] += TSpaceMin(cap, total * (cap / sumcap) * allocation_scale);
        }
    }
    // X: +x -> left[1] (+X), -x -> left[0] (-X).
    if (TSpaceAbs(last_lin.x) > eps) {
      const ISC* grp = last_lin.x >= 0 ? g.left[1] : g.left[0];
      FPD sumcap = 0;
      for (ISC i = 0; i < 4; ++i)
        sumcap += thrust_max[grp[i]] > 0 ? thrust_max[grp[i]] : thrust;
      FPD total = TSpaceClamp(TSpaceAbs(last_lin.x), 0, sumcap);
      if (sumcap > 1e-6)
        for (ISC i = 0; i < 4; ++i) {
          ISC id = grp[i];
          FPD cap = thrust_max[id] > 0 ? thrust_max[id] : thrust;
          out[id] += TSpaceMin(cap, total * (cap / sumcap) * allocation_scale);
        }
    }
  }
  /** Reset smoothing accumulators. */
  void Reset() { last_rot = TSpaceVec(); last_lin = TSpaceVec(); }
};
} //< namespace iGeek
#endif
#endif
