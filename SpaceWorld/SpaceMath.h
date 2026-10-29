// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_SPACEWORLD_MATH_H
#define CRABS_SPACEWORLD_MATH_H
#include "SpaceConfig.h"
#if SEAM >= CRABS_SPACEWORLD
namespace iGeek {
/** 3-component vector, FP64 SI. All SpaceWorld math uses these. */
struct TSpaceVec
{
  FPD x = 0, y = 0, z = 0;
  TSpaceVec() {}
  TSpaceVec(FPD x_, FPD y_, FPD z_) : x(x_), y(y_), z(z_) {}
};
/** Unit quaternion (world frame), Hamilton convention (x,y,z,w). */
struct TSpaceQuat
{
  FPD x = 0, y = 0, z = 0, w = 1;
  TSpaceQuat() {}
  TSpaceQuat(FPD x_, FPD y_, FPD z_, FPD w_) : x(x_), y(y_), z(z_), w(w_) {}
};
/** Spacecraft half-extents (body box), m. Immutable after init. */
struct TSpaceHalf
{
  FPD x = 0.5, y = 0.5, z = 1.0;
  TSpaceHalf() {}
  TSpaceHalf(FPD x_, FPD y_, FPD z_) : x(x_), y(y_), z(z_) {}
};
/** Per-axis capability triple (force/accel/torque/inertia), FP64. */
struct TSpaceAxis3
{
  FPD x = 0, y = 0, z = 0;
  TSpaceAxis3() {}
  TSpaceAxis3(FPD x_, FPD y_, FPD z_) : x(x_), y(y_), z(z_) {}
};
/** 24-thruster RCS group index sets. Each axis has a positive and a
 * negative group (two index lists); rotation axes map to pitch/yaw/roll.
 * Mirrors spacecraftConfig.ts ordering (0-3 front, 4-7 back, 8-11 bottom,
 * 12-15 top, 16-19 +X side, 20-23 -X side).
 */
struct TSpaceGroups
{
  ISC forward[2][4] = {{0, 1, 2, 3}, {4, 5, 6, 7}};
  ISC up[2][4] = {{12, 13, 14, 15}, {8, 9, 10, 11}};
  ISC left[2][4] = {{16, 17, 18, 19}, {20, 21, 22, 23}};
  ISC pitch[2][8] = {{0, 2, 5, 7, 8, 9, 14, 15}, {1, 3, 4, 6, 10, 11, 12, 13}};
  ISC yaw[2][4] = {{0, 1, 6, 7}, {2, 3, 4, 5}};
  ISC roll[2][4] = {{8, 11, 13, 14}, {9, 10, 12, 15}};
};
/** Static math helpers (pure, no state). */
FPD TSpaceMax(FPD a, FPD b);
FPD TSpaceMin(FPD a, FPD b);
FPD TSpaceAbs(FPD a);
FPD TSpaceClamp(FPD v, FPD lo, FPD hi);
TSpaceVec TSpaceVecAdd(const TSpaceVec& a, const TSpaceVec& b);
TSpaceVec TSpaceVecSub(const TSpaceVec& a, const TSpaceVec& b);
TSpaceVec TSpaceVecScale(const TSpaceVec& a, FPD s);
FPD TSpaceVecDot(const TSpaceVec& a, const TSpaceVec& b);
TSpaceVec TSpaceVecCross(const TSpaceVec& a, const TSpaceVec& b);
FPD TSpaceVecLength(const TSpaceVec& a);
FPD TSpaceVecLengthSq(const TSpaceVec& a);
TSpaceVec TSpaceVecNormalize(const TSpaceVec& a);
/** Normalize a quaternion in place. @return false if not finite/too small. */
BOL TSpaceQuatNormalize(TSpaceQuat& q);
/** Conjugate/inverse of a unit quaternion. */
TSpaceQuat TSpaceQuatInvert(const TSpaceQuat& q);
/** Hamilton product a * b. */
TSpaceQuat TSpaceQuatMul(const TSpaceQuat& a, const TSpaceQuat& b);
/** Rotate vector v by unit quaternion q (world frame). */
TSpaceVec TSpaceQuatRotate(const TSpaceQuat& q, const TSpaceVec& v);
/** Rotate world vector into the frame described by q (apply inverse). */
TSpaceVec TSpaceQuatRotateInv(const TSpaceQuat& q, const TSpaceVec& v);
/** Shortest-arc quaternion rotating unit vector from to unit vector to. */
TSpaceQuat TSpaceQuatFromUnitVectors(const TSpaceVec& from, const TSpaceVec& to);
/** Quaternion rotating about unit axis by angle (rad). */
TSpaceQuat TSpaceQuatFromAxisAngle(const TSpaceVec& axis, FPD angle);
/** Error angle (rad, 0..pi) and unit axis between two quaternions. */
void TSpaceQuatAngleAxis(const TSpaceQuat& q, FPD& angle, TSpaceVec& axis);
/** Quaternion sign-flipped so w >= 0 (shortest arc). */
TSpaceQuat TSpaceQuatShortest(const TSpaceQuat& q);
} //< namespace iGeek
#endif
#endif
