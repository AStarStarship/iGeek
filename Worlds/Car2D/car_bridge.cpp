// Copyright AStarship <https://astarship.net>.
// The generated build TU includes the real project headers before this adapter.
namespace iGeek {
struct ACarBrowser
{
  TileMap map;
  TVehicleRoute route;
  TCarState initial;
  ACarEnv* env = _NilP;
  ACarBrowser() : map(1.0f, 120, 64) {}
  ~ACarBrowser() { delete env; }
};
}
extern "C" {
void* CarBrowserCreate(ISC scenario, IUD seed) {
  if (scenario < 0 || scenario >= iGeek::CarScenarioCount) return _NilP;
  auto* car = new iGeek::ACarBrowser;
  auto* planner = new iGeek::AVehiclePlanner;
  BOL ok = iGeek::TCarScenarioBuild(car->map, scenario, *planner, car->route, car->initial);
  delete planner;
  if (!ok) { delete car; return _NilP; }
  car->env = new iGeek::ACarEnv(car->map, car->route, car->initial, seed);
  return car;
}
void CarBrowserDestroy(void* handle) { delete static_cast<iGeek::ACarBrowser*>(handle); }
FPD CarBrowserStep(void* handle, ISC mode, ISC frames, FPD steer, FPD throttle, FPD brake, FPD reverse) {
  auto& env = *static_cast<iGeek::ACarBrowser*>(handle)->env;
  FPD reward = 0;
  iGeek::TCarControl control;
  control.steer = steer; control.throttle = throttle; control.brake = brake;
  control.reverse = reverse;
  for (ISC i = 0; i < frames && !env.Done(); ++i)
    reward += mode == 0 ? env.Step(iGeek::CarCruise) :
              mode == 2 ? env.Step(iGeek::CarStop) : env.StepControl(control);
  return reward;
}
void CarBrowserBarrier(void* handle) {
  auto& car = *static_cast<iGeek::ACarBrowser*>(handle);
  for (ISC y = 0; y < 64; ++y)
    car.map.AddTile(45, y, iGeek::TIntRect(), true, iGeek::VehicleWall);
}
void CarBrowserState(void* handle, FPD* output, FPC* observation) {
  auto& env = *static_cast<iGeek::ACarBrowser*>(handle)->env;
  auto& state = env.State();
  FPD values[] = {state.x, state.y, state.heading, state.forward, state.lateral,
    state.yaw_rate, state.steer, state.time, state.distance, state.longitudinal_accel,
    state.lateral_accel, state.utilization, FPD(env.Status()), env.GoalDistance(),
    FPD(env.Metrics().frames)};
  for (ISC i = 0; i < 15; ++i) output[i] = values[i];
  for (ISC i = 0; i < 22; ++i) observation[i] = env.Observation()[i];
}
ISC CarBrowserMap(void* handle, ISC* tiles, FPD* route) {
  auto& car = *static_cast<iGeek::ACarBrowser*>(handle);
  for (ISC y = 0; y < 64; ++y) for (ISC x = 0; x < 120; ++x) {
    auto* tile = car.map.TileAt(x, y);
    tiles[y * 120 + x] = car.map.CellBlocked(x, y) ? 106 : tile ? tile->GetType() : 100;
  }
  for (ISC i = 0; i < car.route.count; ++i) {
    route[2*i] = car.route.points[i].x; route[2*i+1] = car.route.points[i].y;
  }
  return car.route.count;
}
}
