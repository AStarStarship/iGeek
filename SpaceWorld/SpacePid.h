// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_SPACEWORLD_PID_H
#define CRABS_SPACEWORLD_PID_H
#include "SpaceConfig.h"
#include "SpaceMath.h"
#include "SpaceMath.hxx"
#if SEAM >= CRABS_SPACEWORLD
namespace iGeek {
/** 3-axis PID controller (vector form of PIDController.ts). Works on a
 * TSpaceVec error; integral clamped to maxIntegral, derivative low-pass
 * filtered by alpha. Stateful (integral + last error + last derivative).
 */
struct TSpacePid
{
  FPD kp = 0, ki = 0, kd = 0;
  FPD max_integral = 1.0;
  FPD alpha = 0.9;
  TSpaceVec integral;
  TSpaceVec last_error;
  TSpaceVec last_derivative;
  /** Compute control output for a new error. @return kp*e + ki*I + kd*D. */
  TSpaceVec Update(const TSpaceVec& error, FPD dt) {
    TSpaceVec p_term = TSpaceVecScale(error, kp);
    TSpaceVec i_new = TSpaceVecAdd(integral, TSpaceVecScale(error, dt));
    FPD inorm = TSpaceVecLength(i_new);
    if (inorm > max_integral && inorm > 1e-12)
      integral = TSpaceVecScale(i_new, max_integral / inorm);
    else
      integral = i_new;
    TSpaceVec i_term = TSpaceVecScale(integral, ki);
    TSpaceVec d_new = TSpaceVecScale(TSpaceVecSub(error, last_error), 1 / dt);
    last_derivative = TSpaceVecAdd(TSpaceVecScale(last_derivative, alpha),
                                   TSpaceVecScale(d_new, 1 - alpha));
    TSpaceVec d_term = TSpaceVecScale(last_derivative, kd);
    last_error = error;
    return TSpaceVecAdd(TSpaceVecAdd(p_term, i_term), d_term);
  }
  /** Clear integral/derivative/last-error accumulators. */
  void Reset() {
    integral = TSpaceVec();
    last_error = TSpaceVec();
    last_derivative = TSpaceVec();
  }
};
} //< namespace iGeek
#endif
#endif
