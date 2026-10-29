// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_SPACEWORLD_PHYSICS_H
#define CRABS_SPACEWORLD_PHYSICS_H
#include "SpaceConfig.h"
#include "SpaceMath.h"
#if SEAM >= CRABS_SPACEWORLD
namespace iGeek {
/** 6-DoF rigid spacecraft, zero-g, SI units, FP64. Pose is world frame
 * (position + quaternion), velocity is world linear + body angular.
 * Solid box of mass m and half-extents; principal inertias computed from
 * the box formula at init. No gravity, no damping (space), so the craft
 * coasts until thrusters act.
 */
struct TSpaceParams
{
  FPD mass = 1000.0;          //< Spacecraft mass, kg.
  TSpaceHalf half;            //< Body half-extents, m.
  FPD Ixx = 0, Iyy = 0, Izz = 0;  //< Principal inertias, kg m^2 (computed).
  FPD dt = 1.0 / 60.0;        //< Physics step, s.
  FPD omega_max = 10.0;       //< Angular-velocity sanity bound, rad/s.
  FPD speed_max = 1000.0;     //< Linear-velocity sanity bound, m/s.
};
/** Rigid-body state: world position + orientation, world linear velocity
 * and body angular velocity, plus elapsed time.
 */
struct alignas(ACPUCacheLineSize) TSpaceState
{
  FPD x = 0, y = 0, z = 0;         //< World position, m.
  FPD qx = 0, qy = 0, qz = 0, qw = 1;  //< World orientation (unit quat).
  FPD vx = 0, vy = 0, vz = 0;     //< World linear velocity, m/s.
  FPD wx = 0, wy = 0, wz = 0;     //< Body angular velocity, rad/s.
  FPD time = 0;                   //< Simulated time, s.
};
/** Per-step actuator command: net force (world N) + net torque (body N m).
 * The RCS layer produces this from 24 thruster demands.
 */
struct TSpaceControl
{
  FPD fx = 0, fy = 0, fz = 0;  //< Net force, world frame, N.
  FPD tx = 0, ty = 0, tz = 0;  //< Net torque, body frame, N m.
};
/** Principal inertias of a solid box: Ixx=(m/12)(hy^2+hz^2), etc. */
void TSpaceInertia(TSpaceParams& params);
/** 6-DoF rigid-body step. Force applied in world frame, torque in body
 * frame. Semi-implicit Euler with a 2-substep split at 60 Hz.
 * Deterministic. Clamps velocities to sanity bounds.
 * @return false for invalid inputs; state untouched.
 */
BOL TSpaceStep(TSpaceState& state, const TSpaceParams& params,
              const TSpaceControl& control);
/** True when all state fields are finite and orientation is near-unit. */
BOL TSpaceStateValid(const TSpaceState& state);
/** True when params are positive/finite and inertias are set. */
BOL TSpaceParamsValid(const TSpaceParams& params);
} //< namespace iGeek
#endif
#endif
