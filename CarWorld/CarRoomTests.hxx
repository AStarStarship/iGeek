// Copyright AStarship <https://astarship.net>.
#include "../BikeWorld/TileRuntime.hxx"
#include "../../ASCIICrabs/_Package.hxx"
#include "CarWorld2D.hxx"
#include "CarScenarios.hxx"
#include "../../ASCIICrabs/Room.hpp"
#if __has_include("CarRoom.hxx")
#include "CarRoom.hxx"
#endif

namespace iGeek {
static ISC failures = 0;
#define ROOM_CHECK(value) do { if (!(value)) { \
  StdOut() << "FAIL CarRoom line " << ISC(__LINE__) << ": " << #value << '\n'; \
  ++failures; } } while (false)

static void Takeover() {
#ifdef CRABS_CARWORLD_ROOM_HXX
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route;
  TCarState initial;
  AVehiclePlanner* planner = new AVehiclePlanner;
  ROOM_CHECK(TCarScenarioBuild(map, CarStraight, *planner, route, initial));
  ACarEnv env(map, route, initial, 7);
  ACarEnv reference(map, route, initial, 7);
  ACarRoom room(env);
  Operand& endpoint = room;
  TCarRoomCommand command;
  ROOM_CHECK(room.SetCommand(command));
  const FPD initial_time = env.State().time;
  ROOM_CHECK(endpoint.Star('?', NILP) != NILP);
  ROOM_CHECK(env.State().time == initial_time);
  ROOM_CHECK(endpoint.Star('x', NILP) != NILP);
  ROOM_CHECK(endpoint.Star('t', NILP) == NILP);
  reference.Step();
  ROOM_CHECK(env.State().x == reference.State().x);
  ROOM_CHECK(env.State().y == reference.State().y);
  ROOM_CHECK(room.Reply().frames == 1);
  ROOM_CHECK(room.Reply().status == env.Status());
  command.mode = CarRoomManual;
  command.control = {0.08, 0.2, 0};
  ROOM_CHECK(room.SetCommand(command));
  ROOM_CHECK(endpoint.Star('t', NILP) == NILP);
  reference.StepControl(command.control);
  ROOM_CHECK(env.State().x == reference.State().x);
  ROOM_CHECK(env.State().steer == reference.State().steer);
  ROOM_CHECK(room.Reply().mode == CarRoomManual);
  command.mode = CarRoomAutopilot;
  ROOM_CHECK(room.SetCommand(command));
  ROOM_CHECK(endpoint.Star('t', NILP) == NILP);
  reference.Step();
  ROOM_CHECK(env.State().x == reference.State().x);
  ROOM_CHECK(room.Reply().frames == 3);

  const FPD before = env.State().time;
  command.control.steer = NAN;
  ROOM_CHECK(!room.SetCommand(command));
  ROOM_CHECK(env.State().time == before);
  command.control.steer = 0;
  command.mode = 99;
  ROOM_CHECK(!room.SetCommand(command));
  command.mode = CarRoomManual;
  command.control.throttle = 2;
  ROOM_CHECK(!room.SetCommand(command));
  command.control.throttle = 0;
  ROOM_CHECK(room.SetCommand(command));
  // A corrupted schema must not advance physics or touch driver state.
  TListTypes(room.Floor())[0] = _FPD;
  ROOM_CHECK(endpoint.Star('t', NILP) != NILP);
  ROOM_CHECK(env.State().time == before);
  ROOM_CHECK(room.SetCommand(command));
  auto* version = TPtr<ISC>(TListValue(room.Floor(), ISC(0)));
  *version = 99;
  ROOM_CHECK(endpoint.Star('t', NILP) != NILP);
  ROOM_CHECK(env.State().time == before);
  ROOM_CHECK(room.SetCommand(TCarRoomCommand()));
  TListValuesMap(room.Floor())[3] = -1;
  ROOM_CHECK(endpoint.Star('t', NILP) != NILP);
  ROOM_CHECK(env.State().time == before);
  Crabs interpreter = {};
  ROOM_CHECK(endpoint.Star('t', &interpreter) != NILP);
  ROOM_CHECK(env.State().time == before);
  room.ClearFloor();
  ROOM_CHECK(endpoint.Star('t', NILP) != NILP);
  ROOM_CHECK(env.State().time == before);
  ROOM_CHECK(endpoint.Star('r', NILP) == NILP);
  ROOM_CHECK(env.State().time == 0);
  ROOM_CHECK(room.Reply().frames == 0);
  delete planner;
#else
  ROOM_CHECK(false && "Room-backed CarWorld command adapter missing");
#endif
}
static void TakeoverRoutes() {
#ifdef CRABS_CARWORLD_ROOM_HXX
  AVehiclePlanner* planner = new AVehiclePlanner;
  ISC goals = 0;
  for (ISC scenario = 0; scenario < CarScenarioCount; ++scenario) {
    TileMap map(1.0f, 120, 64);
    TVehicleRoute route;
    TCarState initial;
    ROOM_CHECK(TCarScenarioBuild(map, scenario, *planner, route, initial));
    ACarEnv env(map, route, initial, 7);
    ACarRoom room(env);
    TCarRoomCommand command;
    while (!env.Done()) {
      const IUD frame = env.Metrics().frames;
      command.mode = frame >= 20 && frame < 25 ? CarRoomManual : CarRoomAutopilot;
      command.control = {0, 0, 0.2};
      ROOM_CHECK(room.SetCommand(command));
      if (room.Star('t', NILP) != NILP) {
        ROOM_CHECK(false && "validated Room command rejected");
        break;
      }
    }
    ROOM_CHECK(env.Status() == CarGoal);
    ROOM_CHECK(env.GoalDistance() < 1.2);
    ROOM_CHECK(hypot(env.State().forward, env.State().lateral) < 0.1);
    ROOM_CHECK(room.Reply().frames == env.Metrics().frames);
    if (env.Status() == CarGoal) ++goals;
  }
  StdOut() << "CAR_ROOM takeover_routes=" << goals << '/' << ISC(CarScenarioCount) << '\n';
  ROOM_CHECK(goals == CarScenarioCount);
  delete planner;
#endif
}
}

// Runs the CarRoom takeover suite. Returns the failure count (0 = pass).
// Converted from a standalone ISN main() — now a callable the seam unit invokes.
ISN RunAllRoom() {
  iGeek::Takeover();
  iGeek::TakeoverRoutes();
  _::StdOut() << "CAR_ROOM failures=" << iGeek::failures << '\n';
  return iGeek::failures == 0 ? 0 : 1;
}
