// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_BIKEWORLD_VEHICLE_MAP_HXX
#define CRABS_BIKEWORLD_VEHICLE_MAP_HXX
#include "BikePhysics.hxx"
#include "../TileWorld/TileMap.h"
#if SEAM >= CRABS_BIKEWORLD
namespace iGeek {
/** Tile types are material labels; collision remains Tile's independent flag. */
enum {
  VehicleAsphalt = 100, VehicleGravel, VehicleIce, VehicleGrass,
  VehicleUphill, VehicleDownhill, VehicleWall
};
/** Shared BikeWorld/CarWorld terrain contact adapter. Empty cells = asphalt. */
inline TVehicleSurface TVehicleSample(const TileMap& map, FPD x, FPD y) {
  TVehicleSurface surface;
  if (!isfinite(x) || !isfinite(y)) return surface;
  FPD cell = map.GridSize();
  if (!isfinite(cell) || cell <= 0 || x < 0 || y < 0 ||
      x >= map.SizeGridMax().x * cell || y >= map.SizeGridMax().y * cell)
    return surface;
  const Tile* tile = map.TileAt(ISC(floor(x / cell)), ISC(floor(y / cell)));
  if (tile == NILP) return surface;
  switch (tile->GetType()) {
    case VehicleGravel: surface.friction = 0.45; surface.rolling = 0.025; break;
    case VehicleIce: surface.friction = 0.10; surface.rolling = 0.004; break;
    case VehicleGrass: surface.friction = 0.40; surface.rolling = 0.05; break;
    case VehicleUphill: surface.slope_x = 0.05; break;
    case VehicleDownhill: surface.slope_x = -0.05; break;
    default: break;
  }
  return surface;
}
/** Slab intersection against a closed AABB; updates earliest contact. */
inline BOL TVehicleBoxHit(FPD x0, FPD y0, FPD x1, FPD y1,
    FPD left, FPD bottom, FPD right, FPD top, FPD& contact) {
  FPD enter = 0, exit = 1;
  FPD origins[2] = {x0, y0}, deltas[2] = {x1 - x0, y1 - y0};
  FPD lows[2] = {left, bottom}, highs[2] = {right, top};
  for (ISC axis = 0; axis < 2; ++axis) {
    if (fabs(deltas[axis]) < 1e-14) {
      if (origins[axis] < lows[axis] || origins[axis] > highs[axis]) return false;
    } else {
      FPD near = (lows[axis] - origins[axis]) / deltas[axis];
      FPD far = (highs[axis] - origins[axis]) / deltas[axis];
      if (near > far) { FPD swap = near; near = far; far = swap; }
      enter = TBikeMax(enter, near); exit = TBikeMin(exit, far);
      if (enter > exit) return false;
    }
  }
  contact = TBikeMin(contact, enter);
  return true;
}
/** Swept square safety envelope of radius r around the bike/rider centre.
 * Conservative at corners, but cannot tunnel through a tile or world boundary.
 * Uses TileMap::CellBlocked; no duplicate occupancy grid or entity clamping.
 * Invalid coordinates fail closed. contact is in [0,1], 1 if clear.
 */
inline BOL TVehicleSweep(const TileMap& map, FPD x0, FPD y0,
    FPD x1, FPD y1, FPD radius, FPD& contact) {
  contact = 1;
  if (!isfinite(x0) || !isfinite(y0) || !isfinite(x1) || !isfinite(y1) ||
      !isfinite(radius) || radius < 0) { contact = 0; return true; }
  FPD cell = map.GridSize();
  ISC width = map.SizeGridMax().x, height = map.SizeGridMax().y;
  if (x0 < radius || y0 < radius || x0 > width * cell - radius ||
      y0 > height * cell - radius) { contact = 0; return true; }
  // Clip before converting to ISC, including a solid one-cell outer ring.
  ISC from_x = ISC(floor(TBikeClamp(TBikeMin(x0, x1) - radius, -cell,
                                    width * cell) / cell));
  ISC to_x = ISC(floor(TBikeClamp(TBikeMax(x0, x1) + radius, -cell,
                                  width * cell) / cell));
  ISC from_y = ISC(floor(TBikeClamp(TBikeMin(y0, y1) - radius, -cell,
                                    height * cell) / cell));
  ISC to_y = ISC(floor(TBikeClamp(TBikeMax(y0, y1) + radius, -cell,
                                  height * cell) / cell));
  BOL hit = false;
  for (ISC y = from_y; y <= to_y; ++y)
    for (ISC x = from_x; x <= to_x; ++x)
      if (map.CellBlocked(x, y) && TVehicleBoxHit(x0, y0, x1, y1,
          x * cell - radius, y * cell - radius,
          (x + 1) * cell + radius, (y + 1) * cell + radius, contact)) hit = true;
  return hit;
}
/** Ideal range sensor in metres. Same collision source, no rendering required. */
inline FPD TVehicleRange(const TileMap& map, FPD x, FPD y, FPD angle,
                         FPD range) {
  if (!isfinite(angle) || !isfinite(range) || range <= 0) return 0;
  FPD contact;
  TVehicleSweep(map, x, y, x + range * cos(angle), y + range * sin(angle),
                0, contact);
  return contact * range;
}
} //< namespace iGeek
#endif
#endif
