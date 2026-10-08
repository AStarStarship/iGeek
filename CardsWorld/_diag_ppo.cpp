// Copyright AStarship <https://astarship.net>.
// One-shot diagnostic: replicate the PPO toy task and print per-step reward.
#include <_Config.h>
#define IGEEK_PP_DIAG

#include "../Tensor.h"
#include "../Policy.h"
#include "../Env.h"
#include "../Gym.h"
#include "../PPO.h"
#include "../Tensor.hxx"
#include "../PolicyNet.hxx"
#include "../LinearPolicy.hxx"
#include "../PPO.hxx"
#include "../Gym.hxx"
#include "../../ASCIICrabs/_Package.hxx"

using namespace _;

namespace {
struct TToyGym : public Gym {
  ISC targets_[8];
  TToyGym(ISC b) : Gym("toy") {
    env_count_ = b < 1 ? 1 : (b > GymEnvMax ? GymEnvMax : b);
    obs_len_ = 2;
    action_count_ = 2;
    observations_ = new FPC[env_count_ * obs_len_]();
    actions_ = new ISC[env_count_]();
    rewards_ = new FPC[env_count_]();
    terminals_ = new FPC[env_count_]();
    action_mask_ = new FPC[env_count_ * action_count_]();
    for (ISC i = 0; i < env_count_; ++i) targets_[i] = (i % 2);
    ResetBatch();
  }
  ~TToyGym() override {
    delete[] observations_; delete[] actions_; delete[] rewards_;
    delete[] terminals_; delete[] action_mask_;
  }
  void ResetBatch() override {
    for (ISC i = 0; i < env_count_; ++i) {
      observations_[i * obs_len_ + 0] = (FPC)targets_[i];
      observations_[i * obs_len_ + 1] = 0.0f;
      rewards_[i] = 0.0f;
      terminals_[i] = 1.0f;
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
}  // namespace

int main() {
  const ISC B = 8, OBS = 2, A = 2;
  TToyGym gym(B);
  TLinearPolicy policy(OBS, A, 777);
  policy.SetLearnRate(0.3f);
  TPPO ppo(gym, policy, 1, 1);
  ppo.SetGamma(0.9f);
  ppo.SetLambda(0.9f);

  auto measure = [&]() -> FPC {
    gym.ResetBatch();
    TTensor obs = TTensorAlloc(B, OBS);
    for (ISC i = 0; i < B; ++i)
      for (ISC j = 0; j < OBS; ++j) obs.At(i, j) = gym.Observations()[i * OBS + j];
    TTensor logits = TTensorAlloc(B, A);
    TTensor values = TTensorAlloc(B, 1);
    policy.Forward(obs, logits, values);
    ISC acts[GymEnvMax]; FPC lp[GymEnvMax];
    policy.SampleActions(logits, acts, lp);
    for (ISC i = 0; i < B; ++i) gym.Actions()[i] = acts[i];
    gym.StepBatch();
    FPC r = 0.0f;
    ::_::StdOut() << "  [rewards: ";
    for (ISC i = 0; i < B; ++i) { r += gym.Rewards()[i]; ::_::StdOut() << (ISC)(gym.Rewards()[i]*10000.0f) << " "; }
    ::_::StdOut() << "e-4]  [acts: ";
    for (ISC i = 0; i < B; ++i) ::_::StdOut() << acts[i] << " ";
    ::_::StdOut() << "] [obs0: ";
    for (ISC i = 0; i < B; ++i) ::_::StdOut() << (ISC)(gym.Observations()[i*OBS]);
    ::_::StdOut() << "]\n";
    TTensorFree(obs); TTensorFree(logits); TTensorFree(values);
    return r / (FPC)B;
  };

  // Force Build (allocates W_) before reading weights.
  TTensor obs1 = TTensorAlloc(1, OBS);
  TTensor lg1 = TTensorAlloc(1, A);
  TTensor vl1 = TTensorAlloc(1, 1);
  policy.Forward(obs1, lg1, vl1);
  TTensorFree(obs1); TTensorFree(lg1); TTensorFree(vl1);
  TTensor W = policy.W();
  auto wdump = [&]() {
    ::_::StdOut() << "W00=" << (ISC)(W.AtC(0,0)*10000.0f);
  };
  ::_::StdOut() << "before: "; wdump();
  ::_::StdOut() << "  reward=" << (ISC)(measure() * 1000000.0f) << "e-6\n";

  for (ISC s = 1; s <= 5; ++s) {
    ppo.TrainStep();
    ::_::StdOut() << "after " << s << ": "; wdump();
    ::_::StdOut() << "  reward=" << (ISC)(measure() * 1000000.0f) << "e-6\n";
  }
  return 0;
}
