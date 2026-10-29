// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_BIKEWORLD_VEHICLE_ROUTE_H
#define CRABS_BIKEWORLD_VEHICLE_ROUTE_H
#include "_Config.h"
#include "../TileWorld/TileMap.h"
#if SEAM >= CRABS_BIKEWORLD
namespace iGeek {
enum { VehicleCellMax = 8192, VehicleRouteMax = 256 };
struct TVehiclePoint
{
  FPD x = 0, y = 0;
};
struct TVehicleRoute
{
  TVehiclePoint points[VehicleRouteMax];
  ISC count = 0;
};
struct TVehicleGuide
{
  FPD target_x = 0, target_y = 0, remaining = 0;
  FPD cross_track = 0, curvature = 0;
};
/** Project onto a simple polyline and compute pure-pursuit curvature. */
TVehicleGuide TVehicleGuidance(const TVehicleRoute& route, FPD x, FPD y,
                             FPD heading, FPD lookahead);
/** Fixed-workspace deterministic A*, with clearance and line-of-sight pruning.
 * No per-search/tick allocation. Allocate this workspace off the hot stack.
 * Obstacles are read from TileWorld, not copied to a second world model.
 */
class AVehiclePlanner
{
 public:
  AVehiclePlanner() = default;
  AVehiclePlanner(const AVehiclePlanner&) = delete;
  AVehiclePlanner& operator=(const AVehiclePlanner&) = delete;
  /** @return false with empty route for bad inputs, no path or capacity limits. */
  BOL Plan(const TileMap& map, FPD start_x, FPD start_y,
           FPD goal_x, FPD goal_y, FPD clearance, TVehicleRoute& route);
 private:
  FPD costs_[VehicleCellMax], priorities_[VehicleCellMax];
  ISC parents_[VehicleCellMax], heap_[VehicleCellMax];
  ISC heap_positions_[VehicleCellMax], heap_count_ = 0;
  IUA states_[VehicleCellMax];
  BOL Earlier(ISC a, ISC b) const;
  void HeapRaise(ISC node);
  ISC HeapPop();
};
} //< namespace iGeek
#endif
#endif
