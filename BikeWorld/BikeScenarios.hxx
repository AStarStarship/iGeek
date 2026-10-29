// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_BIKEWORLD_SCENARIOS_HXX
#define CRABS_BIKEWORLD_SCENARIOS_HXX
#include "BikeWorld.hxx"
#if SEAM >= CRABS_BIKEWORLD
namespace iGeek {
enum {
  BikeStraight, BikeCurves, BikeObstacle, BikeGravel, BikeIce,
  BikeUphill, BikeDownhill, BikeLaunch, BikeScenarioCount
};
inline const CHA* TBikeScenarioName(ISC scenario) {
  const CHA* names[] = {"straight", "curves", "obstacle", "gravel", "ice",
                       "uphill", "downhill", "launch"};
  return scenario >= 0 && scenario < BikeScenarioCount ? names[scenario] : "?";
}
/** Deterministic 120 x 64 metre fixtures, with 1 metre material cells.
 * Obstacles are TileWorld collision tiles. Materials do not block routing.
 * Not a road/traffic model; fixtures isolate one physics/control behavior.
 */
inline BOL TBikeScenarioBuild(TileMap& map, ISC scenario,
    AVehiclePlanner& planner, TVehicleRoute& route, TBikeState& initial) {
  if (scenario < 0 || scenario >= BikeScenarioCount || map.GridSize() != 1 ||
      map.SizeGridMax().x != 120 || map.SizeGridMax().y != 64) return false;
  initial = {}; initial.x = 8.5; initial.y = 32.5; initial.forward = 3;
  if (scenario == BikeLaunch) initial.forward = 0;
  if (scenario == BikeObstacle) {
    for (ISC x = 50; x <= 58; ++x)
      for (ISC y = 25; y <= 40; ++y)
        map.AddTile(x, y, TIntRect(), true, VehicleWall);
  }
  if (scenario >= BikeGravel && scenario <= BikeDownhill) {
    ISC type = scenario == BikeGravel ? VehicleGravel :
        scenario == BikeIce ? VehicleIce :
        scenario == BikeUphill ? VehicleUphill : VehicleDownhill;
    for (ISC x = 30; x <= 75; ++x)
      for (ISC y = 18; y <= 47; ++y)
        map.AddTile(x, y, TIntRect(), false, type);
  }
  if (scenario == BikeCurves) {
    route.count = 4;
    route.points[0] = {initial.x, initial.y};
    route.points[1] = {36.5, 42.5};
    route.points[2] = {72.5, 22.5};
    route.points[3] = {108.5, 32.5};
    initial.heading = atan2(10.0, 28.0);
    return true;
  }
  // Extra planning margin covers the baseline follower's corner cutting.
  return planner.Plan(map, initial.x, initial.y, 108.5, 32.5, 5.0, route);
}
} //< namespace iGeek
#endif
#endif
