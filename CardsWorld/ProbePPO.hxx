// Copyright AStarship <https://astarship.net>.
// probe: run just the PPO toy loop under gdb to get a clean crash backtrace.
#include <_Config.h>
#include <_Package.hxx>
#include "../Tensor.h"
#include "../Tensor.hxx"
#include "../Policy.h"
#include "../PolicyNet.hxx"
#include "../Gym.hxx"
#include "../LinearPolicy.hxx"
#include "../PPO.hxx"
using namespace _;

struct TToyGym : public Gym {
  ISC targets_[8];
  TToyGym(ISC b) : Gym("toy") {
    env_count_ = b < 1 ? 1 : (b > GymEnvMax ? GymEnvMax : b);
    obs_len_ = 2; action_count_ = 2;
    observations_ = new FPC[env_count_ * obs_len_]();
    rewards_ = new FPC[env_count_]();
    terminals_ = new FPC[env_count_]();
    action_mask_ = new FPC[env_count_ * action_count_]();
    for (ISC i = 0; i < env_count_; ++i) targets_[i] = (i % 2);
    ResetBatch();
  }
  ~TToyGym() override {
    delete[] observations_; delete[] rewards_;
    delete[] terminals_; delete[] action_mask_;
  }
  void ResetBatch() override {
    for (ISC i = 0; i < env_count_; ++i) {
      observations_[i * obs_len_ + 0] = (FPC)targets_[i];
      observations_[i * obs_len_ + 1] = 0.0f;
      rewards_[i] = 0.0f; terminals_[i] = 1.0f;
      action_mask_[i * action_count_ + 0] = 1.0f;
      action_mask_[i * action_count_ + 1] = 1.0f;
    }
  }
  void StepBatch() override {
    for (ISC i = 0; i < env_count_; ++i) {
      ISC a = Actions()[i];
      rewards_[i] = (a == targets_[i]) ? 1.0f : -1.0f;
      terminals_[i] = 1.0f;
    }
  }
};

int main() {
  const ISC B = 16, OBS = 2, A = 2, HORIZON = 1, EPOCHS = 4;
  TToyGym gym(B);
  StdOut() << "gym envs=" << gym.EnvCount() << " obs=" << (ISC)gym.Observations()[0] << "\n";
  TLinearPolicy policy(OBS, A, 777);
  policy.SetLearnRate(0.3f);
  TPPO ppo(gym, policy, HORIZON, EPOCHS);
  ppo.SetGamma(0.9f); ppo.SetLambda(0.9f); ppo.SetClipEps(0.2f);
  StdOut() << "starting train\n";
  for (ISC s = 0; s < 300; ++s) {
    if (s % 50 == 0) StdOut() << "step " << s << "\n";
    ppo.TrainStep();
  }
  StdOut() << "done training, no crash\n";
  return 0;
}
