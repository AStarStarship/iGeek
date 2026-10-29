// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_CARWORLD_BICYCLE_H
#define CRABS_CARWORLD_BICYCLE_H
#include "CarPhysics.h"
#include "../BikeWorld/BikePhysics.hxx"
#include <math.h>
#if SEAM >= CRABS_CARWORLD
namespace iGeek {
/** Kinematic bicycle model (Course 1 w4). 4-dim state for MPC rollout.
 * dx/dt = V cos(psi), dy/dt = V sin(psi), dpsi/dt = V/L tan(delta).
 * Curvature kappa = tan(delta)/L. This is the planning model (cheap,
 * run hundreds of times per MPC step); the 7-dim TCarStep is the truth model. */
struct TBicycleState
{
  FPD x = 0, y = 0, heading = 0, speed = 0;
};
/** One bicycle step. @pre dt in (0, 0.1]; speed >= 0; |steer| <= 1.2. */
inline BOL TBicycleStep(TBicycleState& b, FPD steer, FPD accel, FPD dt, FPD wheelbase) {
  if (!isfinite(b.x) || !isfinite(b.y) || !isfinite(b.heading) ||
      !isfinite(b.speed) || b.speed < 0 || !isfinite(steer) ||
      !isfinite(accel) || !isfinite(dt) || dt <= 0 || dt > 0.1 ||
      wheelbase <= 0 || !isfinite(wheelbase)) return false;
  b.speed = TBikeMax(0, b.speed + accel * dt);
  FPD ch = cos(b.heading), sh = sin(b.heading);
  FPD old_x = b.x, old_y = b.y;
  b.x += b.speed * ch * dt;
  b.y += b.speed * sh * dt;
  b.heading = TBikeAngle(b.heading + b.speed / wheelbase * tan(TBikeClamp(steer, -1.2, 1.2)) * dt);
  if (!isfinite(b.x) || !isfinite(b.y) || !isfinite(b.heading)) return false;
  return true;
}
/** Roll out N bicycle steps. @pre N in [1, 64]; buffers sized [N+1]. */
inline BOL TBicycleRollout(const TBicycleState& start, const FPD* steer,
    const FPD* accel, ISC n, FPD wheelbase, FPD* xs, FPD* ys,
    FPD* headings, FPD* speeds) {
  if (n < 1 || n > 64 || wheelbase <= 0) return false;
  TBicycleState b = start;
  xs[0] = b.x; ys[0] = b.y; headings[0] = b.heading; speeds[0] = b.speed;
  FPD dt = 0.1;  // Fixed 100 ms rollout step.
  for (ISC k = 0; k < n; ++k) {
    if (!TBicycleStep(b, steer[k], accel[k], dt, wheelbase)) return false;
    xs[k + 1] = b.x; ys[k + 1] = b.y;
    headings[k + 1] = b.heading; speeds[k + 1] = b.speed;
  }
  return true;
}
} //< namespace iGeek
#endif
#endif
