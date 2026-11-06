// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekSpaceWorldRcsH
#define _iGeekSpaceWorldRcsH
#include "SpaceConfig.h"
#include "SpaceMath.h"
#if _Seam >= _iGeekSpaceWorldCore
namespace iGeek {
constexpr ISC SpaceRcsTotal = 24;  //< Total RCS thrusters per spacecraft.
/** One RCS thruster: body-frame mount position + unit force direction
 * (the direction the exhaust pushes the body). Immutable after build.
 */
struct TSpaceThruster
{
  TSpaceVec position;
  TSpaceVec force_dir;
};
/** Fill `out` (SpaceRcsTotal entries) with the default 24-thruster RCS
 * layout for a body of half-extents `half`. Force directions are unit and
 * chosen so the pitch/yaw/roll groups in TSpaceGroups produce the intended
 * rotation about the body x/y/z axes respectively.
 * @pre half.x, half.y, half.z > 0 and finite.
 */
void TSpaceRcsBuild(TSpaceThruster out[SpaceRcsTotal], TSpaceHalf& half);
/** Sum of max thrust of a group (two index lists). */
FPD TSpaceRcsGroupSum(const ISC idx[2][4], const FPD max[SpaceRcsTotal]);
} //< namespace iGeek
#endif  //< _Seam >= _iGeekSpaceWorldCore
#endif  //< _iGeekSpaceWorldRcsH
