// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_SPACEWORLD_CAPS_H
#define CRABS_SPACEWORLD_CAPS_H
#include "SpaceConfig.h"
#include "SpaceMath.h"
#include "SpaceRcs.h"
#if SEAM >= CRABS_SPACEWORLD
namespace iGeek {
/** Per-axis dynamic capabilities of a spacecraft (CapabilityCalculator.ts).
 * lin_force/lin_accel per axis, ang_torque/ang_accel per axis, and the
 * principal inertias. Computed once from the RCS layout + mass + half.
 */
struct TSpaceCaps
{
  TSpaceAxis3 lin_force;
  TSpaceAxis3 lin_accel;
  TSpaceAxis3 ang_torque;
  TSpaceAxis3 ang_accel;
  TSpaceAxis3 inertia;
};
/** Compute capabilities for a spacecraft with `mass`, body `half`, RCS
 * layout `thr` (SpaceRcsTotal thrusters) and per-thruster max `thrust_max`.
 * @pre mass > 0, half finite/positive, thrust_max[i] >= 0.
 */
void TSpaceCapsCompute(TSpaceCaps& caps, FPD mass, const TSpaceHalf& half,
                       const TSpaceThruster thr[SpaceRcsTotal],
                       const FPD thrust_max[SpaceRcsTotal]);
/** Principal inertia about body axis a (0=x,1=y,2=z) of a solid box. */
FPD TSpaceCapsInertiaAxis(FPD mass, const TSpaceHalf& half, ISC a);
/** Effective inertia along a unit direction (box inertia tensor projection). */
FPD TSpaceCapsEffInertia(const TSpaceCaps& caps, const TSpaceVec& dir);
/** Dynamic angular accel cap (AutopilotMode.getDynamicAngularAccelCap). */
void TSpaceCapsAngCap(const TSpaceCaps& caps, FPD max_angular_accel,
                      FPD max_angular_velocity, FPD& alpha_max,
                      FPD& omega_max);
} //< namespace iGeek
#endif
#endif
