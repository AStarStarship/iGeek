// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_SPACEWORLD_RCS_HXX
#define CRABS_SPACEWORLD_RCS_HXX
#include "SpaceRcs.h"
#if SEAM >= CRABS_SPACEWORLD
namespace iGeek {
void TSpaceRcsBuild(TSpaceThruster out[SpaceRcsTotal], const TSpaceHalf& half) {
  FPD hw = half.x, hh = half.y, hd = half.z;
  FPD c = 0.1;  // Cone half-height (m).
  // Front face (mounted at -Z), fires +Z. indices 0-3.
  out[0] = {TSpaceVec(-hw, -hh, -hd - c), TSpaceVec(0, 0, 1)};
  out[1] = {TSpaceVec(-hw,  hh, -hd - c), TSpaceVec(0, 0, 1)};
  out[2] = {TSpaceVec( hw, -hh, -hd - c), TSpaceVec(0, 0, 1)};
  out[3] = {TSpaceVec( hw,  hh, -hd - c), TSpaceVec(0, 0, 1)};
  // Back face (mounted at +Z), fires -Z. indices 4-7.
  out[4] = {TSpaceVec(-hw, -hh,  hd + c), TSpaceVec(0, 0, -1)};
  out[5] = {TSpaceVec(-hw,  hh,  hd + c), TSpaceVec(0, 0, -1)};
  out[6] = {TSpaceVec( hw, -hh,  hd + c), TSpaceVec(0, 0, -1)};
  out[7] = {TSpaceVec( hw,  hh,  hd + c), TSpaceVec(0, 0, -1)};
  // Bottom face (mounted at -Y), fires +Y. indices 8-11.
  out[8]  = {TSpaceVec(-hw, -hh - c, -hd), TSpaceVec(0, 1, 0)};
  out[9]  = {TSpaceVec( hw, -hh - c, -hd), TSpaceVec(0, 1, 0)};
  out[10] = {TSpaceVec( hw, -hh - c,  hd), TSpaceVec(0, 1, 0)};
  out[11] = {TSpaceVec(-hw, -hh - c,  hd), TSpaceVec(0, 1, 0)};
  // Top face (mounted at +Y), fires -Y. indices 12-15.
  out[12] = {TSpaceVec(-hw,  hh + c, -hd), TSpaceVec(0, -1, 0)};
  out[13] = {TSpaceVec( hw,  hh + c, -hd), TSpaceVec(0, -1, 0)};
  out[14] = {TSpaceVec( hw,  hh + c,  hd), TSpaceVec(0, -1, 0)};
  out[15] = {TSpaceVec(-hw,  hh + c,  hd), TSpaceVec(0, -1, 0)};
  // +X side (mounted at +X), fires -X. indices 16-19.
  out[16] = {TSpaceVec( hw + c,  hh, -hd), TSpaceVec(-1, 0, 0)};
  out[17] = {TSpaceVec( hw + c, -hh, -hd), TSpaceVec(-1, 0, 0)};
  out[18] = {TSpaceVec( hw + c,  hh,  hd), TSpaceVec(-1, 0, 0)};
  out[19] = {TSpaceVec( hw + c, -hh,  hd), TSpaceVec(-1, 0, 0)};
  // -X side (mounted at -X), fires +X. indices 20-23.
  out[20] = {TSpaceVec(-hw - c,  hh, -hd), TSpaceVec(1, 0, 0)};
  out[21] = {TSpaceVec(-hw - c, -hh, -hd), TSpaceVec(1, 0, 0)};
  out[22] = {TSpaceVec(-hw - c,  hh,  hd), TSpaceVec(1, 0, 0)};
  out[23] = {TSpaceVec(-hw - c, -hh,  hd), TSpaceVec(1, 0, 0)};
}
FPD TSpaceRcsGroupSum(const ISC idx[2][4], const FPD max[SpaceRcsTotal]) {
  FPD s = 0;
  for (ISC r = 0; r < 2; ++r)
    for (ISC i = 0; i < 4; ++i) s += max[idx[r][i]] > 0 ? max[idx[r][i]] : 0;
  return s;
}
} //< namespace iGeek
#endif
#endif
