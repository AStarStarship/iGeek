// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_SPACEWORLD_MATH_HXX
#define CRABS_SPACEWORLD_MATH_HXX
#include "SpaceMath.h"
#include <math.h>
#if SEAM >= CRABS_SPACEWORLD
namespace iGeek {
constexpr FPD SpacePi = 3.14159265358979323846;
FPD TSpaceMax(FPD a, FPD b) { return a > b ? a : b; }
FPD TSpaceMin(FPD a, FPD b) { return a < b ? a : b; }
FPD TSpaceAbs(FPD a) { return a < 0 ? -a : a; }
FPD TSpaceClamp(FPD v, FPD lo, FPD hi) { return v < lo ? lo : (v > hi ? hi : v); }
TSpaceVec TSpaceVecAdd(const TSpaceVec& a, const TSpaceVec& b) {
  return TSpaceVec(a.x + b.x, a.y + b.y, a.z + b.z);
}
TSpaceVec TSpaceVecSub(const TSpaceVec& a, const TSpaceVec& b) {
  return TSpaceVec(a.x - b.x, a.y - b.y, a.z - b.z);
}
TSpaceVec TSpaceVecScale(const TSpaceVec& a, FPD s) {
  return TSpaceVec(a.x * s, a.y * s, a.z * s);
}
FPD TSpaceVecDot(const TSpaceVec& a, const TSpaceVec& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}
TSpaceVec TSpaceVecCross(const TSpaceVec& a, const TSpaceVec& b) {
  return TSpaceVec(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                   a.x * b.y - a.y * b.x);
}
FPD TSpaceVecLength(const TSpaceVec& a) {
  return sqrt(TSpaceVecDot(a, a));
}
FPD TSpaceVecLengthSq(const TSpaceVec& a) { return TSpaceVecDot(a, a); }
TSpaceVec TSpaceVecNormalize(const TSpaceVec& a) {
  FPD n = TSpaceVecLength(a);
  if (n < 1e-12 || !isfinite(n)) return TSpaceVec(0, 0, 0);
  FPD inv = 1 / n;
  return TSpaceVec(a.x * inv, a.y * inv, a.z * inv);
}
BOL TSpaceQuatNormalize(TSpaceQuat& q) {
  FPD n = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
  if (!isfinite(n) || n < 1e-12) return false;
  FPD inv = 1 / sqrt(n);
  q.x *= inv; q.y *= inv; q.z *= inv; q.w *= inv;
  return true;
}
TSpaceQuat TSpaceQuatInvert(const TSpaceQuat& q) {
  return TSpaceQuat(-q.x, -q.y, -q.z, q.w);
}
TSpaceQuat TSpaceQuatMul(const TSpaceQuat& a, const TSpaceQuat& b) {
  return TSpaceQuat(
      a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
      a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
      a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
      a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z);
}
TSpaceVec TSpaceQuatRotate(const TSpaceQuat& q, const TSpaceVec& v) {
  TSpaceVec qv(q.x, q.y, q.z);
  // v' = v + 2*q.w*(qv x v) + 2*qv x (qv x v). Unit-rotation verified 2026-10-21.
  TSpaceVec t1 = TSpaceVecCross(qv, v);
  TSpaceVec t2 = TSpaceVecCross(qv, t1);
  return TSpaceVecAdd(v, TSpaceVecAdd(TSpaceVecScale(t1, 2 * q.w),
                                      TSpaceVecScale(t2, 2)));
}
TSpaceVec TSpaceQuatRotateInv(const TSpaceQuat& q, const TSpaceVec& v) {
  return TSpaceQuatRotate(TSpaceQuatInvert(q), v);
}
TSpaceQuat TSpaceQuatFromUnitVectors(const TSpaceVec& from, const TSpaceVec& to) {
  TSpaceVec f = TSpaceVecNormalize(from);
  TSpaceVec t = TSpaceVecNormalize(to);
  FPD d = TSpaceVecDot(f, t);
  if (d >= 1 - 1e-12) return TSpaceQuat(0, 0, 0, 1);
  if (d <= -1 + 1e-12) return TSpaceQuat(1, 0, 0, 0);
  TSpaceVec v = TSpaceVecCross(f, t);
  TSpaceQuat q(v.x, v.y, v.z, 1 + d);
  TSpaceQuatNormalize(q);
  return q;
}
TSpaceQuat TSpaceQuatFromAxisAngle(const TSpaceVec& axis_in, FPD angle) {
  TSpaceVec a = TSpaceVecNormalize(axis_in);
  FPD s = sin(angle / 2);
  return TSpaceQuat(a.x * s, a.y * s, a.z * s, cos(angle / 2));
}
void TSpaceQuatAngleAxis(const TSpaceQuat& q_in, FPD& angle, TSpaceVec& axis) {
  TSpaceQuat q = q_in;
  if (q.w < 0) q = TSpaceQuatInvert(q);
  TSpaceQuatNormalize(q);
  FPD wc = TSpaceClamp(q.w, -1, 1);
  angle = 2 * acos(wc);
  FPD sin_half = sqrt(TSpaceMax(0.0, 1 - wc * wc));
  if (sin_half > 1e-6) {
    axis = TSpaceVec(q.x / sin_half, q.y / sin_half, q.z / sin_half);
    axis = TSpaceVecNormalize(axis);
  } else {
    axis = TSpaceVec(1, 0, 0);
  }
  if (angle > SpacePi) {
    angle = 2 * SpacePi - angle;
    axis = TSpaceVecScale(axis, -1);
  }
}
TSpaceQuat TSpaceQuatShortest(const TSpaceQuat& q) {
  return q.w < 0 ? TSpaceQuatInvert(q) : q;
}
} //< namespace iGeek
#endif
#endif
