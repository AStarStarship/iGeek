// Copyright AStarship <https://astarship.net>.
#include "TileRuntime.hxx"
#include <stdio.h>
#include <math.h>
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
static ISC failures = 0;
#define CHECK(condition) do { if (!(condition)) { \
  printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #condition); \
  ++failures; } } while (false)

// Runtime RED even before the additive read-only TileWorld API exists.
template <typename Map>
static void TileQueries(Map& map) {
  if constexpr (requires(const Map& m) {
      m.GridSize(); m.TileAt(1, 1); m.CellBlocked(1, 1); }) {
    CHECK(map.GridSize() == 2.0f);
    CHECK(map.TileAt(1, 1) == NILP);
    CHECK(!map.CellBlocked(1, 1));
    CHECK(map.CellBlocked(-1, 1));
    CHECK(map.CellBlocked(4, 1));
    map.AddTile(1, 1, TIntRect(), false, 101);
    CHECK(map.TileAt(1, 1)->GetType() == 101);
    CHECK(!map.CellBlocked(1, 1));
    map.AddTile(2, 1, TIntRect(), true, 102);
    CHECK(map.CellBlocked(2, 1));
    map.RemoveTile(2, 1);
    CHECK(!map.CellBlocked(2, 1));
  } else {
    CHECK(false && "TileWorld read-only vehicle queries missing");
  }
}
static void CoastPhysics() {
#ifdef CRABS_BIKEWORLD_PHYSICS_HXX
  TBikeParams params;
  params.drag_area = 0; params.roll_damping = 0;
  TVehicleSurface surface; surface.rolling = 0;
  TBikeState bike; bike.forward = 3;
  TBikeControl control;
  for (ISC i = 0; i < 200; ++i)
    CHECK(TBikeStep(bike, params, control, surface, surface, 0.005));
  CHECK(fabs(bike.x - 3.0) < 1e-9);
  CHECK(fabs(bike.forward - 3.0) < 1e-10);
  CHECK(bike.y == 0 && bike.roll == 0 && bike.heading == 0);
  CHECK(!bike.fallen);
#else
  CHECK(false && "continuous bicycle physics missing");
#endif
}
static void TerrainPhysics() {
#ifdef CRABS_BIKEWORLD_VEHICLE_MAP_HXX
  TileMap map(1.0f, 20, 12);
  map.AddTile(4, 5, TIntRect(), false, VehicleIce);
  map.AddTile(8, 5, TIntRect(), true, 777);
  CHECK(TVehicleSample(map, 4.5, 5.5).friction < 0.15);
  CHECK(TVehicleSample(map, 3.5, 5.5).friction > 0.7);
  FPD contact = 1;
  CHECK(TVehicleSweep(map, 2, 5.5, 14, 5.5, 0.7, contact));
  CHECK(fabs(contact - (7.3 - 2.0) / 12.0) < 1e-10);
  CHECK(!TVehicleSweep(map, 2, 2, 14, 2, 0.7, contact));
  CHECK(TVehicleSweep(map, 2, 2, -4, 2, 0.7, contact));
  CHECK(fabs(TVehicleRange(map, 2, 5.5, 0, 15) - 6) < 1e-10);
#else
  CHECK(false && "TileWorld vehicle terrain and swept collision missing");
#endif
}
static void RoutePlanning() {
#ifdef CRABS_BIKEWORLD_VEHICLE_ROUTE_HXX
  TileMap map(1.0f, 60, 40);
  AVehiclePlanner* planner = new AVehiclePlanner;
  TVehicleRoute route;
  CHECK(planner->Plan(map, 5.5, 20.5, 54.5, 20.5, 1.2, route));
  CHECK(route.count == 2);
  for (ISC x = 28; x <= 30; ++x)
    for (ISC y = 10; y <= 30; ++y)
      map.AddTile(x, y, TIntRect(), true, VehicleWall);
  CHECK(planner->Plan(map, 5.5, 20.5, 54.5, 20.5, 1.2, route));
  CHECK(route.count > 2);
  for (ISC i = 1; i < route.count; ++i) {
    FPD contact;
    CHECK(!TVehicleSweep(map, route.points[i - 1].x,
        route.points[i - 1].y, route.points[i].x, route.points[i].y,
        1.2, contact));
  }
  for (ISC y = 0; y < 40; ++y)
    map.AddTile(29, y, TIntRect(), true, VehicleWall);
  CHECK(!planner->Plan(map, 5.5, 20.5, 54.5, 20.5, 1.2, route));
  CHECK(route.count == 0);
  CHECK(!planner->Plan(map, -2, 20, 55, 20, 1.2, route));
  delete planner;
#else
  CHECK(false && "clearance-aware native route planner missing");
#endif
}
static void BalanceControl() {
#ifdef CRABS_BIKEWORLD_PILOT_HXX
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {8.5, 32.5}; route.points[1] = {108.5, 32.5};
  TBikeParams params; TBikeState bike;
  bike.x = 8.5; bike.y = 32.5; bike.forward = 3; bike.roll = 0.04;
  ABikePilot pilot;
  TVehicleSurface surface;
  for (ISC i = 0; i < 2000 && !bike.fallen; ++i) {
    TBikeControl control = pilot.Control(map, route, bike, params, 3.5, 0.005);
    CHECK(fabs(control.steer) <= params.steer_limit);
    CHECK(TBikeStep(bike, params, control, surface, surface, 0.005));
  }
  printf("BALANCE roll=%.6f y_error=%.6f speed=%.3f fallen=%d\n",
      bike.roll, bike.y - 32.5, bike.forward, ISC(bike.fallen));
  CHECK(!bike.fallen);
  CHECK(fabs(bike.roll) < 0.005);
  CHECK(fabs(bike.y - 32.5) < 0.25);
  CHECK(fabs(bike.forward - 3.5) < 0.15);
#else
  CHECK(false && "steering-based native balance controller missing");
#endif
}
static void HeadlessRoute() {
#ifdef CRABS_BIKEWORLD_HXX
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {8.5, 32.5}; route.points[1] = {108.5, 32.5};
  TBikeState initial; initial.x = 8.5; initial.y = 32.5; initial.forward = 3;
  ABikeEnv env(map, route, initial, 7);
  while (!env.Done()) env.Step(BikeCruise);
  printf("ROUTE status=%s time=%.2f distance=%.3f error=%.3f speed=%.3f\n",
      TBikeStatusName(env.Status()), env.State().time, env.State().distance,
      env.GoalDistance(), env.State().forward);
  CHECK(env.Status() == BikeGoal);
  CHECK(env.GoalDistance() < 1.2);
  CHECK(env.State().forward < 0.1 && env.State().supported);
  TBikeState result = env.State();
  CHECK(env.Step(BikeCruise) == 0);
  CHECK(env.State().time == result.time);
  env.Reset();
  while (!env.Done()) env.Step(BikeCruise);
  CHECK(env.State().x == result.x && env.State().y == result.y);
  CHECK(env.State().roll == result.roll && env.State().time == result.time);
  for (ISC i = 0; i < ABikeEnv::ObsLength; ++i) CHECK(isfinite(env.Obs()[i]));
#else
  CHECK(false && "headless next-step bicycle world missing");
#endif
}
static void ScenarioRoutes() {
#ifdef CRABS_BIKEWORLD_SCENARIOS_HXX
  AVehiclePlanner* planner = new AVehiclePlanner;
  for (ISC scenario = 0; scenario < BikeScenarioCount; ++scenario) {
    TileMap map(1.0f, 120, 64);
    TVehicleRoute route; TBikeState initial;
    CHECK(TBikeScenarioBuild(map, scenario, *planner, route, initial));
    ABikeEnv env(map, route, initial, 19);
    while (!env.Done()) env.Step();
    printf("SCENARIO %s status=%s time=%.2f peak_roll=%.4f peak_cte=%.3f\n",
        TBikeScenarioName(scenario), TBikeStatusName(env.Status()),
        env.State().time, env.Metrics().max_roll, env.Metrics().max_cross_track);
    CHECK(env.Status() == BikeGoal);
    CHECK(env.Metrics().max_roll < 0.35);
    CHECK(env.Metrics().max_cross_track < 3.0);
  }
  delete planner;
#else
  CHECK(false && "physical terrain and route scenarios missing");
#endif
}
static void NativeGymBatch() {
#ifdef CRABS_BIKEWORLD_GYM_HXX
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {8.5, 32.5}; route.points[1] = {108.5, 32.5};
  TBikeState initial; initial.x = 8.5; initial.y = 32.5; initial.forward = 3;
  ABikeGym gym(map, route, initial, 4, 123);
  Gym& native = gym;
  CHECK(native.EnvCount() == 4);
  FPC first = native.Observations()[3];
  native.StepBatch();
  CHECK(gym.Bike(0).State().time > 0);
  native.ResetBatch();
  CHECK(native.Observations()[3] == first);
  ISC terminals = 0;
  for (ISC step = 0; step < 1700; ++step) {
    native.StepBatch();
    for (ISC i = 0; i < native.EnvCount(); ++i) {
      if (native.Terminals()[i]) {
        ++terminals; CHECK(gym.LastStatus(i) == BikeGoal);
        CHECK(!gym.Bike(i).Done());
      }
      CHECK(isfinite(native.Rewards()[i]));
    }
  }
  CHECK(terminals == 4);
  for (ISC i = 0; i < 4 * ABikeEnv::ObsLength; ++i)
    CHECK(isfinite(native.Observations()[i]));
#else
  CHECK(false && "iGeek Gym batch adapter missing");
#endif
}
static void PhysicsInvariants() {
  TBikeParams params; TVehicleSurface surface; TBikeControl control;
  TBikeState bike; bike.forward = 3;
  CHECK(!TBikeStep(bike, params, control, surface, surface, 0));
  CHECK(!TBikeStep(bike, params, control, surface, surface, NAN));
  CHECK(bike.time == 0 && bike.forward == 3);
  params.mass = -1;
  CHECK(!TBikeStep(bike, params, control, surface, surface, 0.005));
  params = {}; bike.distance = NAN;
  CHECK(!TBikeStep(bike, params, control, surface, surface, 0.005));
  bike = {}; bike.forward = -1;
  CHECK(!TBikeStep(bike, params, control, surface, surface, 0.005));
  FPD fx = 300, fy = 400;
  CHECK(fabs(TBikeTire(fx, fy, 100) - 5) < 1e-12);
  CHECK(fabs(hypot(fx, fy) - 100) < 1e-12);
  TBikeState dry, ice; dry.forward = ice.forward = 5;
  TVehicleSurface slippery; slippery.friction = 0.1;
  control.brake = 0.8;
  for (ISC i = 0; i < 200; ++i) {
    CHECK(TBikeStep(dry, params, control, surface, surface, 0.005));
    CHECK(TBikeStep(ice, params, control, slippery, slippery, 0.005));
  }
  CHECK(dry.forward < ice.forward && dry.forward < 1);
  CHECK(ice.forward > 3.5);
  TBikeState unbalanced; unbalanced.forward = 3; unbalanced.roll = 0.04;
  control = {};
  for (ISC i = 0; i < 1000 && !unbalanced.fallen; ++i)
    CHECK(TBikeStep(unbalanced, params, control, surface, surface, 0.005));
  CHECK(unbalanced.fallen);
  TBikeState coarse, fine; coarse.forward = fine.forward = 4;
  coarse.roll = fine.roll = 0.05; control.steer = 0.08;
  for (ISC i = 0; i < 50; ++i)
    CHECK(TBikeStep(coarse, params, control, surface, surface, 0.005));
  for (ISC i = 0; i < 100; ++i)
    CHECK(TBikeStep(fine, params, control, surface, surface, 0.0025));
  CHECK(fabs(coarse.x - fine.x) < 1e-5);
  CHECK(fabs(coarse.y - fine.y) < 1e-5);
  CHECK(fabs(coarse.roll - fine.roll) < 1e-5);
  TileMap map(1.0f, 20, 12);
  CHECK(TVehicleSample(map, 1e300, -1e300).friction == 0.85);
  printf("PHYSICS dry_brake_speed=%.3f ice_brake_speed=%.3f "
      "unassisted_fall_time=%.3f rk4_roll_delta=%.3g\n", dry.forward,
      ice.forward, unbalanced.time, fabs(coarse.roll - fine.roll));
}
static void FailurePaths() {
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {8.5, 32.5}; route.points[1] = {108.5, 32.5};
  TBikeState initial; initial.x = 8.5; initial.y = 32.5; initial.forward = 3;
  ABikeEnv stop(map, route, initial, 7);
  while (!stop.Done()) stop.Step(BikeStop);
  CHECK(stop.Status() == BikeStopped && stop.State().supported);
  ABikeEnv invalid(map, route, initial, 7);
  CHECK(invalid.Step(-1) < 0 && invalid.Status() == BikeInvalid);
  CHECK(invalid.State().time == 0);
  ABikeEnv emergency(map, route, initial, 7);
  for (ISC y = 0; y < 64; ++y) map.AddTile(28, y, TIntRect(), true, VehicleWall);
  while (!emergency.Done()) emergency.Step();
  printf("EMERGENCY status=%s gap=%.3f speed=%.3f\n",
      TBikeStatusName(emergency.Status()), 28 - 0.8 - emergency.State().x,
      emergency.State().forward);
  CHECK(emergency.Status() == BikeBlocked);
  CHECK(emergency.State().x < 27.2 && emergency.State().supported);
  CHECK(28 - 0.8 - emergency.State().x > 0.5);
  initial.forward = 300;
  ABikeEnv crash(map, route, initial, 7);
  TBikeControl control;
  for (ISC i = 0; i < 30 && !crash.Done(); ++i) crash.StepControl(control);
  CHECK(crash.Status() == BikeCollision);
  CHECK(crash.State().x <= 27.2 && crash.State().forward == 0);
}
static void ResetContracts() {
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {8.5, 32.5}; route.points[1] = {108.5, 32.5};
  TBikeState initial; initial.x = 8.5; initial.y = 32.5;
  initial.roll = 0.2;
  ABikeEnv stopped(map, route, initial, 19);
  CHECK(stopped.Metrics().max_roll >= fabs(stopped.State().roll));
  initial.x = NAN;
  ABikeEnv bad(map, route, initial, 19);
  CHECK(bad.Status() == BikeInvalid);
  for (ISC i = 0; i < ABikeEnv::ObsLength; ++i) CHECK(isfinite(bad.Obs()[i]));
  initial.x = 8.5; initial.roll = 0;
  route.points[1].y = NAN;
  ABikeEnv bad_route(map, route, initial, 19);
  CHECK(bad_route.Status() == BikeInvalid);
}
ISC main() {
  TileMap map(2.0f, 4, 3);
  CHECK(map.SizeGridMax().x == 4);
  CHECK(map.SizeGridMax().y == 3);
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
  printf("BIKEWORLD_TESTS failures=%d\n", failures);
  return failures ? 1 : 0;
}
