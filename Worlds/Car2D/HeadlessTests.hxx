// Copyright AStarship <https://astarship.net>.
#include <_Config.h>
#include "../../Worlds/Bike/TileRuntime.hxx"
#include <stdio.h>
#include "../../CrabsTK/Math/Blas.h"
#include "../../../ASCIICrabs/Math.hxx"
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
using namespace iGeek;
static ISC cw_failures = 0;
#define _Check(condition) do { if (!(condition)) { \
  _DROut("FAIL " << __FILE__ << " " << __LINE__ << " " << #condition << '\n'); \
  ++cw_failures; } } while (false)
#define NAN FPC(0.0 / 0.0)

static void FreeCoast() {
#ifdef _iGeekCarWorldPhysicsHXX
  TCarParams params; params.drag_area = 0;
  TCarContacts contacts;
  for (TVehicleSurface& surface : contacts.wheels) surface.rolling = 0;
  TCarState car; car.forward = 8;
  TCarControl go;
  for (ISC i = 0; i < 200; ++i)
    _Check(TCarStep(car, params, go, contacts, 0.005));
  _Check(Absolute(car.x - 8) < 1e-10);
  _Check(car.forward == 8 && car.y == 0 && car.heading == 0);
  _Check(Absolute(car.time - 1) < 1e-12);
#else
  _Check(false && "native 2D car physics missing");
#endif
}
static void ForceAndTurning() {
  TCarParams params; params.drag_area = 0; params.drive_power = 1e12;
  TCarContacts contacts;
  for (TVehicleSurface& surface : contacts.wheels) surface.rolling = 0;
  TCarState drive; drive.forward = 8;
  TCarControl control; control.throttle = 0.2;
  for (ISC i = 0; i < 200; ++i)
    _Check(TCarStep(drive, params, control, contacts, 0.005));
  _Check(Absolute(drive.forward - 9) < 1e-9);
  _Check(Absolute(drive.x - 8.5) < 1e-9);
  _Check(Absolute(drive.yaw_rate) < 1e-12);
  TCarState turn; turn.forward = 8;
  control.throttle = 0; control.steer = 0.1;
  for (ISC i = 0; i < 400; ++i)
    _Check(TCarStep(turn, params, control, contacts, 0.005));
  _Check(turn.y > 1 && turn.heading > 0.3 && turn.yaw_rate > 0.15);
  _Check(Absolute(turn.steer - 0.1) < 1e-5);
  _DROut("(label)");
}
static void InputRejection() {
  TCarState car; car.forward = 8;
  TCarParams params; TCarControl control; TCarContacts contacts;
  TCarState saved = car;
  params.mass = -1;
  _Check(!TCarStep(car, params, control, contacts, 0.005));
  _Check(car.x == saved.x && car.forward == saved.forward && car.time == saved.time);
  params = {}; control.steer = NAN;
  _Check(!TCarStep(car, params, control, contacts, 0.005));
  control = {}; car.distance = NAN;
  _Check(!TCarStep(car, params, control, contacts, 0.005));
  _Check(IsNaN(car.distance) && car.time == 0);
  car = saved; contacts.wheels[0].friction = 0;
  _Check(!TCarStep(car, params, control, contacts, 0.005));
  contacts = {}; params.steer_limit = 2;
  _Check(!TCarStep(car, params, control, contacts, 0.005));
  params = {}; car.x = 1e300;
  _Check(!TCarStep(car, params, control, contacts, 0.005));
  _Check(car.x == 1e300 && car.time == 0);
  car = saved;
  _Check(!TCarStep(car, params, control, contacts, 0));
  _Check(!TCarStep(car, params, control, contacts, 0.02));
  _Check(!TCarStep(car, params, control, contacts, NAN));
}
static void GripAndConvergence() {
  TCarParams params; TCarContacts dry, ice, split;
  for (TVehicleSurface& surface : ice.wheels) surface.friction = 0.1;
  split.wheels[CarFrontLeft].friction = 0.1;
  split.wheels[CarRearLeft].friction = 0.1;
  TCarControl control; control.brake = 1;
  TCarState a, b, c; a.forward = b.forward = c.forward = 10;
  for (ISC i = 0; i < 400; ++i) {
    _Check(TCarStep(a, params, control, dry, 0.005));
    _Check(TCarStep(b, params, control, ice, 0.005));
    _Check(TCarStep(c, params, control, split, 0.005));
  }
  _Check(a.forward < 0.1 && b.forward > 7.5);
  _Check(a.distance < b.distance && Absolute(c.heading) > 0.05);
  FPD fx = 300, fy = 400;
  _Check(Absolute(TBikeTire(fx, fy, 100) - 5) < 1e-12);
  _Check(Absolute(hypot(fx, fy) - 100) < 1e-12);
  TCarState coarse, fine; coarse.forward = fine.forward = 8;
  control = {}; control.steer = 0.1;
  for (ISC i = 0; i < 100; ++i)
    _Check(TCarStep(coarse, params, control, dry, 0.005));
  for (ISC i = 0; i < 200; ++i)
    _Check(TCarStep(fine, params, control, dry, 0.0025));
  _Check(Absolute(coarse.x - fine.x) < 1e-5);
  _Check(Absolute(coarse.y - fine.y) < 1e-5);
  _Check(Absolute(coarse.heading - fine.heading) < 1e-5);
  _DROut(
      "ice_distance=%.6f split_heading=%.6f rk4_y_delta=" << a.forward << "\n");
}
static void RouteFollower() {
#ifdef _iGeekCarWorld2DHXX
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {12.5, 32.5}; route.points[1] = {108.5, 32.5};
  TCarState initial; initial.x = 12.5; initial.y = 32.5; initial.forward = 6;
  ACarEnv env(map, route, initial, 7);
  while (!env.Done()) env.Step();
  _Check(env.Status() == CarGoal);
  _Check(env.GoalDistance() < 1.2 && env.State().forward < 0.1);
  _Check(Absolute(env.State().lateral) < 0.1);
  _Check(env.Metrics().physics_steps == 4 * env.Metrics().frames);
  TCarState first = env.State();
  _Check(env.Step() == 0 && env.State().time == first.time);
  env.Reset();
  while (!env.Done()) env.Step();
  _Check(env.State().x == first.x && env.State().y == first.y);
  _Check(env.State().time == first.time && env.State().heading == first.heading);
  for (ISC i = 0; i < ACarEnv::ObservationLength; ++i) _Check(iGeek::IsFinite(env.Observation()[i]));
  _DROut("TCarStatusName=" << TCarStatusName(env.Status()) << '\n');
#else
  _Check(false && "native car route follower missing");
#endif
}
static void CarScenarioRoutes() {
#ifdef _iGeekCarWorldScenariosHXX
  AVehiclePlanner* planner = new AVehiclePlanner;
  for (ISC scenario = 0; scenario < CarScenarioCount; ++scenario) {
    TileMap map(1.0f, 120, 64);
    TVehicleRoute route; TCarState initial;
    _Check(TCarScenarioBuild(map, scenario, *planner, route, initial));
    ACarEnv env(map, route, initial, 19);
    while (!env.Done()) env.Step();
    _Check(env.Status() == CarGoal);
    _Check(env.GoalDistance() < 1.2 && env.State().forward < 0.1);
    _Check(env.Metrics().max_cross_track < 4);
    _DROut("TCarScenarioName=" << TCarScenarioName(scenario) << " " << TCarStatusName(env.Status()) << '\n');
  }
  delete planner;
#else
  _Check(false && "car terrain/route fixtures missing");
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
    _Check(env.Status() == CarGoal);
    _Check(env.GoalDistance() < 1.2 && env.State().forward < 0.1);
    _DROut("action=" << action << " " << TCarStatusName(env.Status()) << '\n');
  }
}
static void CarFailurePaths() {
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {12.5, 32.5}; route.points[1] = {108.5, 32.5};
  TCarState initial; initial.x = 12.5; initial.y = 32.5; initial.forward = 8;
  ACarEnv stop(map, route, initial, 7);
  while (!stop.Done()) stop.Step(CarStop);
  _Check(stop.Status() == CarStopped && stop.State().forward < 0.1);
  ACarEnv invalid(map, route, initial, 7);
  _Check(invalid.Step(-1) < 0 && invalid.Status() == CarInvalid);
  _Check(invalid.State().time == 0);
  TCarControl bad; bad.steer = NAN;
  ACarEnv bad_control(map, route, initial, 7);
  _Check(bad_control.StepControl(bad) < 0 && bad_control.Status() == CarInvalid);
  _Check(bad_control.State().time == 0);
  ACarEnv emergency(map, route, initial, 7);
  for (ISC y = 0; y < 64; ++y) map.AddTile(45, y, TIntRect(), true, 777);
  while (!emergency.Done()) emergency.Step();
  FPD gap = 45 - emergency.Params().body_radius - emergency.State().x;
  _Check(emergency.Status() == CarBlocked);
  _Check(emergency.State().forward < 0.1 && gap > 0.25);
  _DROut("TCarStatusName=" << TCarStatusName(emergency.Status()) << '\n');
  initial.forward = 300;
  ACarEnv crash(map, route, initial, 7);
  TCarControl crash_control;
  while (!crash.Done()) crash.StepControl(crash_control);
  _Check(crash.Status() == CarCollision && crash.State().forward == 0);
  _Check(crash.State().x <= 45 - crash.Params().body_radius);
  AVehiclePlanner* planner = new AVehiclePlanner;
  _Check(!planner->Plan(map, 12.5, 32.5, 108.5, 32.5, 6, route));
  _Check(route.count == 0);
  ACarEnv blocked(map, route, initial, 7);
  _Check(blocked.Status() == CarBlocked);
  delete planner;
  route.count = 2; route.points[0] = {12.5, 32.5}; route.points[1] = {108.5, NAN};
  initial.forward = 8;
  ACarEnv bad_route(map, route, initial, 7);
  _Check(bad_route.Status() == CarInvalid);
  for (ISC i = 0; i < ACarEnv::ObservationLength; ++i) _Check(iGeek::IsFinite(bad_route.Observation()[i]));
  route.points[1].y = 32.5; initial.x = 1e300;
  ACarEnv bad_pose(map, route, initial, 7);
  _Check(bad_pose.Status() == CarInvalid);
}
static void NativeGym() {
#ifdef _iGeekCarWorldCarGymHXX
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {12.5, 32.5}; route.points[1] = {108.5, 32.5};
  TCarState initial; initial.x = 12.5; initial.y = 32.5; initial.forward = 6;
  ACarGym gym(map, route, initial, 4, 7);
  Gym& native = gym; _Check(native.EnvironmentCount() == 4);
  /* Manifest contract: the in-Room contract for a joining agent. */
  _Check(EnvironmentManifestValidate(&gym.Car(0).Manifest()));
  _Check(TSEquals(gym.Car(0).Manifest().name, "carworld") != _NilP);
  FPC first = native.Observations()[4];
  native.StepBatch(); _Check(gym.Car(0).State().time > 0);
  native.ResetBatch(); _Check(native.Observations()[4] == first);
  ISC terminals = 0;
  for (ISC step = 0; step < 1000; ++step) {
    native.StepBatch();
    for (ISC i = 0; i < native.EnvironmentCount(); ++i) {
      _Check(iGeek::IsFinite(native.Rewards()[i]));
      if (native.Terminals()[i]) {
        ++terminals; _Check(gym.LastStatus(i) == CarGoal);
        _Check(native.Rewards()[i] > 9);
        _Check(gym.Car(i).State().time == 0 && !gym.Car(i).Done());
        _Check(native.Observations()[i * ACarEnv::ObservationLength + 21] == 0);
      }
    }
  }
  _Check(terminals == 4);
  ACarGym raw(map, route, initial, 2, 11, CarActuatorActions);
  _Check(raw.ActionCount() == 9);
  raw.Actions()[0] = 0; raw.Actions()[1] = 2;
  raw.StepBatch();
  _Check(raw.Car(0).State().steer < -0.01 && raw.Car(1).State().steer > 0.01);
  raw.Actions()[0] = 9; raw.StepBatch();
  _Check(raw.Terminals()[0] == 1 && raw.LastStatus(0) == CarInvalid);
  _Check(raw.Rewards()[0] == -10 && raw.Car(0).State().time == 0);
#else
  _Check(false && "native Gym driver/raw actuator interfaces missing");
#endif
}
// Runs the full headless CarWorld suite. Returns the failure count (0 = pass).
// Converted from a standalone ISC main() + _DROut — now a callable the seam
// unit invokes (no C/C++ standard library; uses ASCIICrabs RefOut).
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
  RefOut() << "CARWORLD_TESTS failures=" << cw_failures << '\n';
  return cw_failures;
}
