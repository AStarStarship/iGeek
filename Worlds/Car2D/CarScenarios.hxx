// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekCarWorldScenariosHXX
#define _iGeekCarWorldScenariosHXX
#include "CarWorld2D.hxx"
#if _Seam >= _iGeekCarWorldCore
namespace iGeek {
enum { CarStraight, CarCurves, CarObstacle, CarGravel, CarIce,
       CarUphill, CarDownhill, CarLaunch, CarIceCurves, CarTrack, CarScenarioCount };
inline const CHA* TCarScenarioName(ISC scenario) {
  const CHA* names[] = {"straight", "curves", "obstacle", "gravel", "ice",
                       "uphill", "downhill", "launch", "ice_curves", "track"};
  return scenario >= 0 && scenario < CarScenarioCount ? names[scenario] : "?";
}
/** Signed distance (<= 0 inside) from a point to a rounded rectangle. */
inline FPD TRoundRectDistance(FPD x, FPD y, FPD cx, FPD cy,
    FPD half_w, FPD half_h, FPD radius) {
  FPD qx = Absolute(x - cx) - half_w + radius;
  FPD qy = Absolute(y - cy) - half_h + radius;
  FPD dx = TBikeMax(qx, 0), dy = TBikeMax(qy, 0);
  return hypot(dx, dy) + TBikeMin(TBikeMax(qx, qy), 0) - radius;
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
  if (scenario == CarTrack) {
    // Border walls contain the car on the 120 x 64 map.
    for (ISC x = 0; x < 120; ++x) {
      map.AddTile(x, 0, TIntRect(), true, VehicleWall);
      map.AddTile(x, 63, TIntRect(), true, VehicleWall);
    }
    for (ISC y = 0; y < 64; ++y) {
      map.AddTile(0, y, TIntRect(), true, VehicleWall);
      map.AddTile(119, y, TIntRect(), true, VehicleWall);
    }
    // Rounded-rectangle circuit: asphalt (default) is the track band, grass elsewhere.
    for (ISC x = 1; x < 119; ++x)
      for (ISC y = 1; y < 63; ++y) {
        FPD cx = FPD(x) + 0.5, cy = FPD(y) + 0.5;
        if (TRoundRectDistance(cx, cy, 60, 32, 32, 14, 10) <= 0 ||
            TRoundRectDistance(cx, cy, 60, 32, 40, 22, 16) > 0)
          map.AddTile(x, y, TIntRect(), false, VehicleGrass);
      }
    route.count = 8;
    route.points[0] = {60, 48}; route.points[1] = {88, 44};
    route.points[2] = {98, 32}; route.points[3] = {88, 20};
    route.points[4] = {60, 16}; route.points[5] = {32, 20};
    route.points[6] = {22, 32}; route.points[7] = {32, 44};
    initial = {}; initial.x = 60; initial.y = 48; initial.forward = 0;
    initial.heading = 0;
    return true;
  }
  return planner.Plan(map, initial.x, initial.y, 108.5, 32.5, 6.0, route);
}
} //< namespace iGeek
#endif
#endif
