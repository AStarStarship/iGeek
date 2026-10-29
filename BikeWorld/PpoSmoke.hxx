// Copyright AStarship <https://astarship.net>.
// Integration smoke only: this does not train or validate learned balance.
#include "TileRuntime.hxx"
#include "BikeGym.hxx"
#include "../PPO.hxx"
#include <stdio.h>
using namespace _;
// PPO training smoke test. Converted from a standalone ISC main() — now a
// callable the seam unit invokes. Returns 0 on success, 1 on failure.
// (D_COUT kept as legacy debt, same as CarWorld/HeadlessTests.hxx.)
ISC RunAllPpoSmoke() {
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; route.count = 2;
  route.points[0] = {8.5, 32.5}; route.points[1] = {108.5, 32.5};
  TBikeState initial; initial.x = 8.5; initial.y = 32.5; initial.forward = 3;
  ABikeGym gym(map, route, initial, GymEnvMax, 7);
  TLinearPolicy policy(ABikeEnv::ObsLength, BikeActionCount, 11);
  policy.SetLearnRate(0.001f);
  TPPO trainer(gym, policy, 32, 2);
  TPPOStats stats = {};
  for (ISC update = 0; update < 10; ++update) {
    stats = trainer.TrainStep();
    if (!isfinite(stats.policy_loss_) || !isfinite(stats.value_loss_) ||
        !isfinite(stats.entropy_) || !isfinite(stats.approx_kl_) ||
        !isfinite(stats.explained_var_)) {
      D_COUT("PPO_SMOKE failed update=" << update << "\n"); return 1;
    }
  }
  D_COUT("PPO_SMOKE passed updates=10 envs=" << gym.EnvCount() << " horizon=32 "
      "policy_loss=%.6f value_loss=%.6f entropy=%.6f\n");
  return 0;
}
