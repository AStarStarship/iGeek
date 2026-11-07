// Copyright AStarship <https://astarship.net>.
// Integration/finite-update smoke, NOT learned route competence.
#include "../../Worlds/Bike/TileRuntime.hxx"
#include "../../../ASCIICrabs/Math.hxx"
#include "CarGym.hxx"
#include "../../PPO.hxx"
#include <stdio.h>
#include <time.h>
using namespace _;
static FPD Seconds() {
  timespec time;
  if (clock_gettime(CLOCK_MONOTONIC, &time) != 0) return 0;
  return FPD(time.tv_sec) + FPD(time.tv_nsec) * 1e-9;
}
ISC main() {
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {12.5, 32.5}; route.points[1] = {108.5, 32.5};
  TCarState initial; initial.x = 12.5; initial.y = 32.5; initial.forward = 6;
  for (ISC mode = CarDriverActions; mode <= CarActuatorActions; ++mode) {
    ACarGym gym(map, route, initial, GymEnvMax, 7, mode);
    TLinearPolicy policy(ACarEnv::ObservationLength, gym.ActionCount(), 11);
    policy.SetLearnRate(0.001f);
    PPO trainer(gym, policy, 64, 2);
    PPOStats stats = {}; FPD started = Seconds();
    for (ISC update = 0; update < 10; ++update) {
      stats = trainer.TrainStep();
      if (!iGeek::IsFinite(stats.policy_loss_) || !iGeek::IsFinite(stats.value_loss_) ||
          !iGeek::IsFinite(stats.entropy_) || !iGeek::IsFinite(stats.approx_kl_) ||
          !iGeek::IsFinite(stats.explained_var_)) {
        _DROut("CAR_PPO_SMOKE failed mode=" << mode << " update=" << update << "\n"); return 1;
      }
    }
    FPD wall = Seconds() - started;
    _DROut("CAR_PPO_SMOKE passed mode=" << mode == CarActuatorActions ? "raw" : "driver" << " updates=10 envs=" << gym.EnvironmentCount() << " horizon=64 "
        "policy_loss=%.6f value_loss=%.6f entropy=%.6f wall_s=%.6f\n");
  }
  return 0;
}
