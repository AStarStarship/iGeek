// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_CARWORLD_SCENARIOS_HXX
#define CRABS_CARWORLD_SCENARIOS_HXX
#include "CarWorld2D.hxx"
#if SEAM >= CRABS_CARWORLD
namespace _ {
enum { CarStraight, CarCurves, CarObstacle, CarGravel, CarIce,
       CarUphill, CarDownhill, CarLaunch, CarIceCurves, CarScenarioCount };
inline const CHA* TCarScenarioName(ISC scenario) {
  const CHA* names[] = {"straight", "curves", "obstacle", "gravel", "ice",
                       "uphill", "downhill", "launch", "ice_curves"};
  return scenario >= 0 && scenario < CarScenarioCount ? names[scenario] : "?";
}
/** 120 x 64 m ideal fixtures. Caller supplies a fresh 1 m TileMap.
 * Seeds perturb initial pose, not these fixed terrain layouts.
 */
inline BOL TCarScenarioBuild(TileMap& map, ISC scenario,
    AVehiclePlanner& planner, TVehicleRoute& route, TCarState& initial) {
  route.count = 0;
  if (scenario < 0 || scenario >= CarScenarioCount || map.GridSize() != 1 ||
      map.SizeGridMax().x != 120 || map.SizeGridMax().y != 64) return false;
  initial = {}; initial.x = 12.5; initial.y = 32.5; initial.forward = 6;
  if (scenario == CarLaunch) initial.forward = 0;
  if (scenario == CarObstacle) {
    for (ISC x = 50; x <= 58; ++x)
      for (ISC y = 25; y <= 40; ++y)
        map.AddTile(x, y, TIntRect(), true, VehicleWall);
  }
  if ((scenario >= CarGravel && scenario <= CarDownhill) || scenario == CarIceCurves) {
    ISC type = scenario == CarGravel ? VehicleGravel :
        scenario == CarIce || scenario == CarIceCurves ? VehicleIce :
        scenario == CarUphill ? VehicleUphill : VehicleDownhill;
    for (ISC x = 30; x <= 75; ++x)
      for (ISC y = 18; y <= 47; ++y)
        map.AddTile(x, y, TIntRect(), false, type);
  }
  if (scenario == CarCurves || scenario == CarIceCurves) {
    route.count = 4;
    route.points[0] = {initial.x, initial.y};
    route.points[1] = {36.5, 42.5}; route.points[2] = {72.5, 22.5};
    route.points[3] = {108.5, 32.5};
    initial.heading = atan2(10.0, 24.0);
    return true;
  }
  // Margin for the square body envelope and non-curvature-constrained follower.
  return planner.Plan(map, initial.x, initial.y, 108.5, 32.5, 6.0, route);
}
}  //< namespace _
#endif
#endif
