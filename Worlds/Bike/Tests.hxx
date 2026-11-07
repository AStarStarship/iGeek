// Copyright AStarship <https://astarship.net>.
#include "TileRuntime.hxx"
#include <stdio.h>
#include "../../CrabsTK/Math/Blas.h"
#if __has_include("BikePhysics.hxx")
#include "BikePhysics.hxx"
#endif
#if __has_include("VehicleMap.hxx")
#include "VehicleMap.hxx"
#endif
#if __has_include("VehicleRoute.hxx")
#include "VehicleRoute.hxx"
#endif
#if __has_include("BikePilot.hxx")
#include "BikePilot.hxx"
#endif
#if __has_include("BikeWorld.hxx")
#include "BikeWorld.hxx"
#endif
#if __has_include("BikeScenarios.hxx")
#include "BikeScenarios.hxx"
#endif
#if __has_include("BikeGym.hxx")
#include "BikeGym.hxx"
#endif
using namespace _;
using namespace iGeek;
static ISC failures = 0;
#define _Check(condition) do { if (!(condition)) { \
  _DROut("FAIL " << __FILE__ << " " << __LINE__ << " " << #condition << '\n'); \
  ++failures; } } while (false)

// Runtime RED even before the additive read-only TileWorld API exists.
template <typename Map>
static void TileQueries(Map& map) {
  if constexpr (requires(Map& m) {
      m.GridSize(); m.TileAt(1, 1); m.CellBlocked(1, 1); }) {
    _Check(map.GridSize() == 2.0f);
    _Check(map.TileAt(1, 1) == _NilP);
    _Check(!map.CellBlocked(1, 1));
    _Check(map.CellBlocked(-1, 1));
    _Check(map.CellBlocked(4, 1));
    map.AddTile(1, 1, TIntRect(), false, 101);
    _Check(map.TileAt(1, 1)->GetType() == 101);
    _Check(!map.CellBlocked(1, 1));
    map.AddTile(2, 1, TIntRect(), true, 102);
    _Check(map.CellBlocked(2, 1));
    map.RemoveTile(2, 1);
    _Check(!map.CellBlocked(2, 1));
  } else {
    _Check(false && "TileWorld read-only vehicle queries missing");
  }
}
static void CoastPhysics() {
#ifdef _iGeekBikeWorldPhysicsHXX
  TBikeParams params;
  params.drag_area = 0; params.roll_damping = 0;
  TVehicleSurface surface; surface.rolling = 0;
  TBikeState bike; bike.forward = 3;
  TBikeControl control;
  for (ISC i = 0; i < 200; ++i)
    _Check(TBikeStep(bike, params, control, surface, surface, 0.005));
  _Check(Absolute(bike.x - 3.0) < 1e-9);
  _Check(Absolute(bike.forward - 3.0) < 1e-10);
  _Check(bike.y == 0 && bike.roll == 0 && bike.heading == 0);
  _Check(!bike.fallen);
#else
  _Check(false && "continuous bicycle physics missing");
#endif
}
static void TerrainPhysics() {
#ifdef _iGeekBikeWorldVehicleMapHXX
  TileMap map(1.0f, 20, 12);
  map.AddTile(4, 5, TIntRect(), false, VehicleIce);
  map.AddTile(8, 5, TIntRect(), true, 777);
  _Check(TVehicleSample(map, 4.5, 5.5).friction < 0.15);
  _Check(TVehicleSample(map, 3.5, 5.5).friction > 0.7);
  FPD contact = 1;
  _Check(TVehicleSweep(map, 2, 5.5, 14, 5.5, 0.7, contact));
  _Check(Absolute(contact - (7.3 - 2.0) / 12.0) < 1e-10);
  _Check(!TVehicleSweep(map, 2, 2, 14, 2, 0.7, contact));
  _Check(TVehicleSweep(map, 2, 2, -4, 2, 0.7, contact));
  _Check(Absolute(TVehicleRange(map, 2, 5.5, 0, 15) - 6) < 1e-10);
#else
  _Check(false && "TileWorld vehicle terrain and swept collision missing");
#endif
}
static void RoutePlanning() {
#ifdef _iGeekBikeWorldVehicleRouteHXX
  TileMap map(1.0f, 60, 40);
  AVehiclePlanner* planner = new AVehiclePlanner;
  TVehicleRoute route;
  _Check(planner->Plan(map, 5.5, 20.5, 54.5, 20.5, 1.2, route));
  _Check(route.count == 2);
  for (ISC x = 28; x <= 30; ++x)
    for (ISC y = 10; y <= 30; ++y)
      map.AddTile(x, y, TIntRect(), true, VehicleWall);
  _Check(planner->Plan(map, 5.5, 20.5, 54.5, 20.5, 1.2, route));
  _Check(route.count > 2);
  for (ISC i = 1; i < route.count; ++i) {
    FPD contact;
    _Check(!TVehicleSweep(map, route.points[i - 1].x,
        route.points[i - 1].y, route.points[i].x, route.points[i].y,
        1.2, contact));
  }
  for (ISC y = 0; y < 40; ++y)
    map.AddTile(29, y, TIntRect(), true, VehicleWall);
  _Check(!planner->Plan(map, 5.5, 20.5, 54.5, 20.5, 1.2, route));
  _Check(route.count == 0);
  _Check(!planner->Plan(map, -2, 20, 55, 20, 1.2, route));
  delete planner;
#else
  _Check(false && "clearance-aware native route planner missing");
#endif
}
static void BalanceControl() {
#ifdef _iGeekBikeWorldPilotHXX
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {8.5, 32.5}; route.points[1] = {108.5, 32.5};
  TBikeParams params; TBikeState bike;
  bike.x = 8.5; bike.y = 32.5; bike.forward = 3; bike.roll = 0.04;
  ABikePilot pilot;
  TVehicleSurface surface;
  for (ISC i = 0; i < 2000 && !bike.fallen; ++i) {
    TBikeControl control = pilot.Control(map, route, bike, params, 3.5, 0.005);
    _Check(Absolute(control.steer) <= params.steer_limit);
    _Check(TBikeStep(bike, params, control, surface, surface, 0.005));
  }
  _DROut("bike.roll=" << bike.roll << '\n');
  _Check(!bike.fallen);
  _Check(Absolute(bike.roll) < 0.005);
  _Check(Absolute(bike.y - 32.5) < 0.25);
  _Check(Absolute(bike.forward - 3.5) < 0.15);
#else
  _Check(false && "steering-based native balance controller missing");
#endif
}
static void HeadlessRoute() {
#ifdef _iGeekBikeWorldHXX
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {8.5, 32.5}; route.points[1] = {108.5, 32.5};
  TBikeState initial; initial.x = 8.5; initial.y = 32.5; initial.forward = 3;
  ABikeEnv env(map, route, initial, 7);
  while (!env.Done()) env.Step(BikeCruise);
  _DROut("TBikeStatusName=" << TBikeStatusName(env.Status()) << '\n');
  _Check(env.Status() == BikeGoal);
  _Check(env.GoalDistance() < 1.2);
  _Check(env.State().forward < 0.1 && env.State().supported);
  TBikeState result = env.State();
  _Check(env.Step(BikeCruise) == 0);
  _Check(env.State().time == result.time);
  env.Reset();
  while (!env.Done()) env.Step(BikeCruise);
  _Check(env.State().x == result.x && env.State().y == result.y);
  _Check(env.State().roll == result.roll && env.State().time == result.time);
  for (ISC i = 0; i < ABikeEnv::ObservationLength; ++i) _Check(iGeek::IsFinite(env.Observation()[i]));
#else
  _Check(false && "headless next-step bicycle world missing");
#endif
}
static void ScenarioRoutes() {
#ifdef _iGeekBikeWorldScenariosHXX
  AVehiclePlanner* planner = new AVehiclePlanner;
  for (ISC scenario = 0; scenario < BikeScenarioCount; ++scenario) {
    TileMap map(1.0f, 120, 64);
    TVehicleRoute route; TBikeState initial;
    _Check(TBikeScenarioBuild(map, scenario, *planner, route, initial));
    ABikeEnv env(map, route, initial, 19);
    while (!env.Done()) env.Step();
    _DROut("TBikeScenarioName=" << TBikeScenarioName(scenario) << " " << TBikeStatusName(env.Status()) << '\n');
    _Check(env.Status() == BikeGoal);
    _Check(env.Metrics().max_roll < 0.35);
    _Check(env.Metrics().max_cross_track < 3.0);
  }
  delete planner;
#else
  _Check(false && "physical terrain and route scenarios missing");
#endif
}
static void NativeGymBatch() {
#ifdef _iGeekBikeWorldGymHXX
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {8.5, 32.5}; route.points[1] = {108.5, 32.5};
  TBikeState initial; initial.x = 8.5; initial.y = 32.5; initial.forward = 3;
  ABikeGym gym(map, route, initial, 4, 123);
  Gym& native = gym;
  _Check(native.EnvironmentCount() == 4);
  /* Manifest contract: the in-Room contract for a joining agent. */
  _Check(EnvironmentManifestValidate(&gym.Bike(0).Manifest()));
  _Check(TSEquals(gym.Bike(0).Manifest().name, "bikeworld") != _NilP);
  FPC first = native.Observations()[3];
  native.StepBatch();
  _Check(gym.Bike(0).State().time > 0);
  native.ResetBatch();
  _Check(native.Observations()[3] == first);
  ISC terminals = 0;
  for (ISC step = 0; step < 1700; ++step) {
    native.StepBatch();
    for (ISC i = 0; i < native.EnvironmentCount(); ++i) {
      if (native.Terminals()[i]) {
        ++terminals; _Check(gym.LastStatus(i) == BikeGoal);
        _Check(!gym.Bike(i).Done());
      }
      _Check(iGeek::IsFinite(native.Rewards()[i]));
    }
  }
  _Check(terminals == 4);
  for (ISC i = 0; i < 4 * ABikeEnv::ObservationLength; ++i)
    _Check(iGeek::IsFinite(native.Observations()[i]));
#else
  _Check(false && "iGeek Gym batch adapter missing");
#endif
}
static void PhysicsInvariants() {
  TBikeParams params; TVehicleSurface surface; TBikeControl control;
  TBikeState bike; bike.forward = 3;
  _Check(!TBikeStep(bike, params, control, surface, surface, 0));
  _Check(!TBikeStep(bike, params, control, surface, surface, NAN));
  _Check(bike.time == 0 && bike.forward == 3);
  params.mass = -1;
  _Check(!TBikeStep(bike, params, control, surface, surface, 0.005));
  params = {}; bike.distance = NAN;
  _Check(!TBikeStep(bike, params, control, surface, surface, 0.005));
  bike = {}; bike.forward = -1;
  _Check(!TBikeStep(bike, params, control, surface, surface, 0.005));
  FPD fx = 300, fy = 400;
  _Check(Absolute(TBikeTire(fx, fy, 100) - 5) < 1e-12);
  _Check(Absolute(hypot(fx, fy) - 100) < 1e-12);
  TBikeState dry, ice; dry.forward = ice.forward = 5;
  TVehicleSurface slippery; slippery.friction = 0.1;
  control.brake = 0.8;
  for (ISC i = 0; i < 200; ++i) {
    _Check(TBikeStep(dry, params, control, surface, surface, 0.005));
    _Check(TBikeStep(ice, params, control, slippery, slippery, 0.005));
  }
  _Check(dry.forward < ice.forward && dry.forward < 1);
  _Check(ice.forward > 3.5);
  TBikeState unbalanced; unbalanced.forward = 3; unbalanced.roll = 0.04;
  control = {};
  for (ISC i = 0; i < 1000 && !unbalanced.fallen; ++i)
    _Check(TBikeStep(unbalanced, params, control, surface, surface, 0.005));
  _Check(unbalanced.fallen);
  TBikeState coarse, fine; coarse.forward = fine.forward = 4;
  coarse.roll = fine.roll = 0.05; control.steer = 0.08;
  for (ISC i = 0; i < 50; ++i)
    _Check(TBikeStep(coarse, params, control, surface, surface, 0.005));
  for (ISC i = 0; i < 100; ++i)
    _Check(TBikeStep(fine, params, control, surface, surface, 0.0025));
  _Check(Absolute(coarse.x - fine.x) < 1e-5);
  _Check(Absolute(coarse.y - fine.y) < 1e-5);
  _Check(Absolute(coarse.roll - fine.roll) < 1e-5);
  TileMap map(1.0f, 20, 12);
  _Check(TVehicleSample(map, 1e300, -1e300).friction == 0.85);
  _DROut(
      "unassisted_fall_time=%.3f rk4_roll_delta=" << dry.forward << "\n");
}
static void FailurePaths() {
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {8.5, 32.5}; route.points[1] = {108.5, 32.5};
  TBikeState initial; initial.x = 8.5; initial.y = 32.5; initial.forward = 3;
  ABikeEnv stop(map, route, initial, 7);
  while (!stop.Done()) stop.Step(BikeStop);
  _Check(stop.Status() == BikeStopped && stop.State().supported);
  ABikeEnv invalid(map, route, initial, 7);
  _Check(invalid.Step(-1) < 0 && invalid.Status() == BikeInvalid);
  _Check(invalid.State().time == 0);
  ABikeEnv emergency(map, route, initial, 7);
  for (ISC y = 0; y < 64; ++y) map.AddTile(28, y, TIntRect(), true, VehicleWall);
  while (!emergency.Done()) emergency.Step();
  _DROut("TBikeStatusName=" << TBikeStatusName(emergency.Status()) << '\n');
  _Check(emergency.Status() == BikeBlocked);
  _Check(emergency.State().x < 27.2 && emergency.State().supported);
  _Check(28 - 0.8 - emergency.State().x > 0.5);
  initial.forward = 300;
  ABikeEnv crash(map, route, initial, 7);
  TBikeControl control;
  for (ISC i = 0; i < 30 && !crash.Done(); ++i) crash.StepControl(control);
  _Check(crash.Status() == BikeCollision);
  _Check(crash.State().x <= 27.2 && crash.State().forward == 0);
}
static void ResetContracts() {
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {8.5, 32.5}; route.points[1] = {108.5, 32.5};
  TBikeState initial; initial.x = 8.5; initial.y = 32.5;
  initial.roll = 0.2;
  ABikeEnv stopped(map, route, initial, 19);
  _Check(stopped.Metrics().max_roll >= Absolute(stopped.State().roll));
  initial.x = NAN;
  ABikeEnv bad(map, route, initial, 19);
  _Check(bad.Status() == BikeInvalid);
  for (ISC i = 0; i < ABikeEnv::ObservationLength; ++i) _Check(iGeek::IsFinite(bad.Observation()[i]));
  initial.x = 8.5; initial.roll = 0;
  route.points[1].y = NAN;
  ABikeEnv bad_route(map, route, initial, 19);
  _Check(bad_route.Status() == BikeInvalid);
}
// Runs the full BikeWorld test suite. Returns the failure count (0 = pass).
// Converted from a standalone ISC main() — now a callable the seam unit
// invokes. (_DROut kept as legacy debt, same as CarWorld/HeadlessTests.hxx.)
ISC RunAllTests() {
  TileMap map(2.0f, 4, 3);
  _Check(map.SizeGridMax().x == 4);
  _Check(map.SizeGridMax().y == 3);
  TileQueries(map);
  CoastPhysics();
  TerrainPhysics();
  RoutePlanning();
  BalanceControl();
  HeadlessRoute();
  ScenarioRoutes();
  NativeGymBatch();
  PhysicsInvariants();
  FailurePaths();
  ResetContracts();
  _DROut("failures=" << failures << '\n');
  return failures;
}
