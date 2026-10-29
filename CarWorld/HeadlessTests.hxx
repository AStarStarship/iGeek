// Copyright AStarship <https://astarship.net>.
#include "../BikeWorld/TileRuntime.hxx"
#include <stdio.h>
#include <math.h>
#if __has_include("CarPhysics.hxx")
#include "CarPhysics.hxx"
#endif
#if __has_include("CarWorld2D.hxx")
#include "CarWorld2D.hxx"
#endif
#if __has_include("CarScenarios.hxx")
#include "CarScenarios.hxx"
#endif
#if __has_include("CarGym.hxx")
#include "CarGym.hxx"
#endif
using namespace _;
static ISC cw_failures = 0;
#define CHECK(condition) do { if (!(condition)) { \
  D_COUT("FAIL " << __FILE__ <<  << __LINE__ <<  << #condition <<); \
  ++cw_failures; } } while (false)

static void FreeCoast() {
#ifdef CRABS_CARWORLD_PHYSICS_HXX
  TCarParams params; params.drag_area = 0;
  TCarContacts contacts;
  for (TVehicleSurface& surface : contacts.wheels) surface.rolling = 0;
  TCarState car; car.forward = 8;
  for (ISC i = 0; i < 200; ++i)
    CHECK(TCarStep(car, params, TCarControl(), contacts, 0.005));
  CHECK(fabs(car.x - 8) < 1e-10);
  CHECK(car.forward == 8 && car.y == 0 && car.heading == 0);
  CHECK(fabs(car.time - 1) < 1e-12);
#else
  CHECK(false && "native 2D car physics missing");
#endif
}
static void ForceAndTurning() {
  TCarParams params; params.drag_area = 0; params.drive_power = 1e12;
  TCarContacts contacts;
  for (TVehicleSurface& surface : contacts.wheels) surface.rolling = 0;
  TCarState drive; drive.forward = 8;
  TCarControl control; control.throttle = 0.2;
  for (ISC i = 0; i < 200; ++i)
    CHECK(TCarStep(drive, params, control, contacts, 0.005));
  CHECK(fabs(drive.forward - 9) < 1e-9);
  CHECK(fabs(drive.x - 8.5) < 1e-9);
  CHECK(fabs(drive.yaw_rate) < 1e-12);
  TCarState turn; turn.forward = 8;
  control.throttle = 0; control.steer = 0.1;
  for (ISC i = 0; i < 400; ++i)
    CHECK(TCarStep(turn, params, control, contacts, 0.005));
  CHECK(turn.y > 1 && turn.heading > 0.3 && turn.yaw_rate > 0.15);
  CHECK(fabs(turn.steer - 0.1) < 1e-5);
  D_COUT("(label)");
}
static void InputRejection() {
  TCarState car; car.forward = 8;
  TCarParams params; TCarControl control; TCarContacts contacts;
  TCarState saved = car;
  params.mass = -1;
  CHECK(!TCarStep(car, params, control, contacts, 0.005));
  CHECK(car.x == saved.x && car.forward == saved.forward && car.time == saved.time);
  params = {}; control.steer = NAN;
  CHECK(!TCarStep(car, params, control, contacts, 0.005));
  control = {}; car.distance = NAN;
  CHECK(!TCarStep(car, params, control, contacts, 0.005));
  CHECK(isnan(car.distance) && car.time == 0);
  car = saved; contacts.wheels[0].friction = 0;
  CHECK(!TCarStep(car, params, control, contacts, 0.005));
  contacts = {}; params.steer_limit = 2;
  CHECK(!TCarStep(car, params, control, contacts, 0.005));
  params = {}; car.x = 1e300;
  CHECK(!TCarStep(car, params, control, contacts, 0.005));
  CHECK(car.x == 1e300 && car.time == 0);
  car = saved;
  CHECK(!TCarStep(car, params, control, contacts, 0));
  CHECK(!TCarStep(car, params, control, contacts, 0.02));
  CHECK(!TCarStep(car, params, control, contacts, NAN));
}
static void GripAndConvergence() {
  TCarParams params; TCarContacts dry, ice, split;
  for (TVehicleSurface& surface : ice.wheels) surface.friction = 0.1;
  split.wheels[CarFrontLeft].friction = 0.1;
  split.wheels[CarRearLeft].friction = 0.1;
  TCarControl control; control.brake = 1;
  TCarState a, b, c; a.forward = b.forward = c.forward = 10;
  for (ISC i = 0; i < 400; ++i) {
    CHECK(TCarStep(a, params, control, dry, 0.005));
    CHECK(TCarStep(b, params, control, ice, 0.005));
    CHECK(TCarStep(c, params, control, split, 0.005));
  }
  CHECK(a.forward < 0.1 && b.forward > 7.5);
  CHECK(a.distance < b.distance && fabs(c.heading) > 0.05);
  FPD fx = 300, fy = 400;
  CHECK(fabs(TBikeTire(fx, fy, 100) - 5) < 1e-12);
  CHECK(fabs(hypot(fx, fy) - 100) < 1e-12);
  TCarState coarse, fine; coarse.forward = fine.forward = 8;
  control = {}; control.steer = 0.1;
  for (ISC i = 0; i < 100; ++i)
    CHECK(TCarStep(coarse, params, control, dry, 0.005));
  for (ISC i = 0; i < 200; ++i)
    CHECK(TCarStep(fine, params, control, dry, 0.0025));
  CHECK(fabs(coarse.x - fine.x) < 1e-5);
  CHECK(fabs(coarse.y - fine.y) < 1e-5);
  CHECK(fabs(coarse.heading - fine.heading) < 1e-5);
  D_COUT(
      "ice_distance=%.6f split_heading=%.6f rk4_y_delta=" << a.forward << "\n");
}
static void RouteFollower() {
#ifdef CRABS_CARWORLD_2D_HXX
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {12.5, 32.5}; route.points[1] = {108.5, 32.5};
  TCarState initial; initial.x = 12.5; initial.y = 32.5; initial.forward = 6;
  ACarEnv env(map, route, initial, 7);
  while (!env.Done()) env.Step();
  CHECK(env.Status() == CarGoal);
  CHECK(env.GoalDistance() < 1.2 && env.State().forward < 0.1);
  CHECK(fabs(env.State().lateral) < 0.1);
  CHECK(env.Metrics().physics_steps == 4 * env.Metrics().frames);
  TCarState first = env.State();
  CHECK(env.Step() == 0 && env.State().time == first.time);
  env.Reset();
  while (!env.Done()) env.Step();
  CHECK(env.State().x == first.x && env.State().y == first.y);
  CHECK(env.State().time == first.time && env.State().heading == first.heading);
  for (ISC i = 0; i < ACarEnv::ObsLength; ++i) CHECK(isfinite(env.Obs()[i]));
  D_COUT("TCarStatusName=" << TCarStatusName(env.Status()) <<);
#else
  CHECK(false && "native car route follower missing");
#endif
}
static void CarScenarioRoutes() {
#ifdef CRABS_CARWORLD_SCENARIOS_HXX
  AVehiclePlanner* planner = new AVehiclePlanner;
  for (ISC scenario = 0; scenario < CarScenarioCount; ++scenario) {
    TileMap map(1.0f, 120, 64);
    TVehicleRoute route; TCarState initial;
    CHECK(TCarScenarioBuild(map, scenario, *planner, route, initial));
    ACarEnv env(map, route, initial, 19);
    while (!env.Done()) env.Step();
    CHECK(env.Status() == CarGoal);
    CHECK(env.GoalDistance() < 1.2 && env.State().forward < 0.1);
    CHECK(env.Metrics().max_cross_track < 4);
    D_COUT("TCarScenarioName=" << TCarScenarioName(scenario) <<  << TCarStatusName(env.Status()) <<);
  }
  delete planner;
#else
  CHECK(false && "car terrain/route fixtures missing");
#endif
}
static void SpeedStops() {
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {12.5, 32.5}; route.points[1] = {108.5, 32.5};
  TCarState initial; initial.x = 12.5; initial.y = 32.5; initial.forward = 6;
  ISC actions[] = {CarCruise, CarSlow, CarFast, CarCrawl};
  for (ISC action : actions) {
    ACarEnv env(map, route, initial, 7);
    while (!env.Done()) env.Step(action);
    CHECK(env.Status() == CarGoal);
    CHECK(env.GoalDistance() < 1.2 && env.State().forward < 0.1);
    D_COUT("action=" << action <<  << TCarStatusName(env.Status()) <<);
  }
}
static void CarFailurePaths() {
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {12.5, 32.5}; route.points[1] = {108.5, 32.5};
  TCarState initial; initial.x = 12.5; initial.y = 32.5; initial.forward = 8;
  ACarEnv stop(map, route, initial, 7);
  while (!stop.Done()) stop.Step(CarStop);
  CHECK(stop.Status() == CarStopped && stop.State().forward < 0.1);
  ACarEnv invalid(map, route, initial, 7);
  CHECK(invalid.Step(-1) < 0 && invalid.Status() == CarInvalid);
  CHECK(invalid.State().time == 0);
  TCarControl bad; bad.steer = NAN;
  ACarEnv bad_control(map, route, initial, 7);
  CHECK(bad_control.StepControl(bad) < 0 && bad_control.Status() == CarInvalid);
  CHECK(bad_control.State().time == 0);
  ACarEnv emergency(map, route, initial, 7);
  for (ISC y = 0; y < 64; ++y) map.AddTile(45, y, TIntRect(), true, 777);
  while (!emergency.Done()) emergency.Step();
  FPD gap = 45 - emergency.Params().body_radius - emergency.State().x;
  CHECK(emergency.Status() == CarBlocked);
  CHECK(emergency.State().forward < 0.1 && gap > 0.25);
  D_COUT("TCarStatusName=" << TCarStatusName(emergency.Status()) <<);
  initial.forward = 300;
  ACarEnv crash(map, route, initial, 7);
  while (!crash.Done()) crash.StepControl(TCarControl());
  CHECK(crash.Status() == CarCollision && crash.State().forward == 0);
  CHECK(crash.State().x <= 45 - crash.Params().body_radius);
  AVehiclePlanner* planner = new AVehiclePlanner;
  CHECK(!planner->Plan(map, 12.5, 32.5, 108.5, 32.5, 6, route));
  CHECK(route.count == 0);
  ACarEnv blocked(map, route, initial, 7);
  CHECK(blocked.Status() == CarBlocked);
  delete planner;
  route.count = 2; route.points[0] = {12.5, 32.5}; route.points[1] = {108.5, NAN};
  initial.forward = 8;
  ACarEnv bad_route(map, route, initial, 7);
  CHECK(bad_route.Status() == CarInvalid);
  for (ISC i = 0; i < ACarEnv::ObsLength; ++i) CHECK(isfinite(bad_route.Obs()[i]));
  route.points[1].y = 32.5; initial.x = 1e300;
  ACarEnv bad_pose(map, route, initial, 7);
  CHECK(bad_pose.Status() == CarInvalid);
}
static void NativeGym() {
#ifdef CRABS_CARWORLD_GYM_HXX
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {12.5, 32.5}; route.points[1] = {108.5, 32.5};
  TCarState initial; initial.x = 12.5; initial.y = 32.5; initial.forward = 6;
  ACarGym gym(map, route, initial, 4, 7);
  Gym& native = gym; CHECK(native.EnvCount() == 4);
  FPC first = native.Observations()[4];
  native.StepBatch(); CHECK(gym.Car(0).State().time > 0);
  native.ResetBatch(); CHECK(native.Observations()[4] == first);
  ISC terminals = 0;
  for (ISC step = 0; step < 1000; ++step) {
    native.StepBatch();
    for (ISC i = 0; i < native.EnvCount(); ++i) {
      CHECK(isfinite(native.Rewards()[i]));
      if (native.Terminals()[i]) {
        ++terminals; CHECK(gym.LastStatus(i) == CarGoal);
        CHECK(native.Rewards()[i] > 9);
        CHECK(gym.Car(i).State().time == 0 && !gym.Car(i).Done());
        CHECK(native.Observations()[i * ACarEnv::ObsLength + 21] == 0);
      }
    }
  }
  CHECK(terminals == 4);
  ACarGym raw(map, route, initial, 2, 11, CarActuatorActions);
  CHECK(raw.ActionCount() == 9);
  raw.Actions()[0] = 0; raw.Actions()[1] = 2;
  raw.StepBatch();
  CHECK(raw.Car(0).State().steer < -0.01 && raw.Car(1).State().steer > 0.01);
  raw.Actions()[0] = 9; raw.StepBatch();
  CHECK(raw.Terminals()[0] == 1 && raw.LastStatus(0) == CarInvalid);
  CHECK(raw.Rewards()[0] == -10 && raw.Car(0).State().time == 0);
#else
  CHECK(false && "native Gym driver/raw actuator interfaces missing");
#endif
}
// Runs the full headless CarWorld suite. Returns the failure count (0 = pass).
// Converted from a standalone ISC main() + D_COUT — now a callable the seam
// unit invokes (no C/C++ standard library; uses ASCIICrabs StdOut).
ISC RunAllHeadless() {
  FreeCoast();
  ForceAndTurning();
  InputRejection();
  GripAndConvergence();
  RouteFollower();
  CarScenarioRoutes();
  SpeedStops();
  CarFailurePaths();
  NativeGym();
  StdOut() << "CARWORLD_TESTS failures=" << cw_failures << '\n';
  return cw_failures;
}
