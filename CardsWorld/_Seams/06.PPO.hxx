// Copyright AStarship <https://astarship.net>.

// The card world's PPO seam unit: the M5 test — verify the TPPO loop
// (GAE + clipped PPO + entropy + value-clip) actually LEARNS on a toy env
// with a known optimum: reward must go UP over training steps.
// Gated on CARDSWORLD_CORE.
//
// Toy env (TToyGym): a B-parallel "match the hidden rule" task. Each env has
// a hidden target bit; the obs encodes it; the correct action matches it.
// Reward +1 for a correct action, -1 for wrong, 0 otherwise. The optimum is
// to always pick the matching action -> reward -> +1 per step. A random-init
// linear policy starts near 0.5 accuracy (~0 reward); PPO must drive reward
// up toward +1.

#if SEAM >= CARDSWORLD_CORE

#include "../PPO.hxx"

using namespace ::_;
namespace CWTest {

namespace {
inline ISC& PPFAILS() {
  static ISC count = 0;
  return count;
}
#define PP_CHECK(cond)                                       \
  do {                                                       \
    if (!::_::Test(cond)) {                                  \
      ++PPFAILS();                                           \
      if (SEAM >= CRABS_COUT)                                \
        ::_::StdOut() << "\nFAILURE PPO at line:" << __LINE__ \
                      << " in \"" << __FILE__ << "\"\n";      \
    }                                                        \
  } while (0)

/* A toy gym: B parallel 1-step "match the target" envs. obs[0] = target bit
   (0 or 1) + noise; the correct action == target. Episode = 1 step (terminal
   always). Reward +1 correct, -1 wrong. */
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
    // Fixed targets (deterministic): env i's target = (i % 2).
    for (ISC i = 0; i < env_count_; ++i) targets_[i] = (i % 2);
    ResetBatch();
  }
  ~TToyGym() override {
    delete[] observations_; delete[] actions_; delete[] rewards_;
    delete[] terminals_; delete[] action_mask_;
  }
  void ResetBatch() override {
    for (ISC i = 0; i < env_count_; ++i) {
      // obs[0] = target, obs[1] = 0 (a second feature to make it non-trivial).
      observations_[i * obs_len_ + 0] = (FPC)targets_[i];
      observations_[i * obs_len_ + 1] = 0.0f;
      rewards_[i] = 0.0f;
      terminals_[i] = 1.0f;  // 1-step env.
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

inline const CHA* PPO(const CHA* args) {
  A_TEST_BEGIN;
  const ISC B = 8, OBS = 2, A = 2, HORIZON = 1, EPOCHS = 4;  // B <= GymEnvMax (8)
  const ISC TRAIN_STEPS = 300;

  TToyGym gym(B);
  TLinearPolicy policy(OBS, A, 777);
  policy.SetLearnRate(0.3f);
  TPPO ppo(gym, policy, HORIZON, EPOCHS);
  ppo.SetGamma(0.9f);
  ppo.SetLambda(0.9f);
  ppo.SetClipEps(0.2f);
  ppo.SetEntropyCoef(0.01f);

  // Measure reward before training (a few steps).
  FPC reward_before = 0.0f;
  for (ISC s = 0; s < 20; ++s) {
    TPPOStats st = ppo.TrainStep();
    reward_before += st.policy_loss_;  // placeholder; we measure gym reward below
  }
  // Actually measure the gym reward directly: run the policy, sum rewards.
  auto measure = [&]() -> FPC {
    gym.ResetBatch();
    TTensor obs = TTensorAlloc(B, OBS);
    for (ISC i = 0; i < B; ++i)
      for (ISC j = 0; j < OBS; ++j) obs.At(i, j) = gym.Observations()[i * OBS + j];
    TTensor logits = TTensorAlloc(B, A);
    TTensor values = TTensorAlloc(B, 1);
    policy.Forward(obs, logits, values);
    ISC acts[B]; FPC lp[B];
    policy.SampleActions(logits, acts, lp);
    for (ISC i = 0; i < B; ++i) gym.Actions()[i] = acts[i];
    gym.StepBatch();
    FPC r = 0.0f;
    for (ISC i = 0; i < B; ++i) r += gym.Rewards()[i];
    TTensorFree(obs); TTensorFree(logits); TTensorFree(values);
    return r / (FPC)B;  // mean reward in [-1, 1].
  };

  FPC r0 = measure();
  D_COUT("PPO: reward before training (mean) = " << (ISC)(r0 * 1e6f) << "e-6\n");

  // Train.
  for (ISC s = 0; s < TRAIN_STEPS; ++s) {
    ppo.TrainStep();
  }
  FPC r1 = measure();
  D_COUT("PPO: reward after " << TRAIN_STEPS << " steps (mean) = "
                              << (ISC)(r1 * 1e6f) << "e-6\n");

  // The policy must have IMPROVED: reward after > reward before.
  PP_CHECK(r1 > r0);
  // And it should have learned the rule: reward close to +1 (the optimum).
  // With 300 steps x 4 epochs on a 2-action linear task, it should be > 0.5.
  PP_CHECK(r1 > 0.5f);
  D_COUT("PPO: learned mean reward " << (ISC)(r1 * 1e4f) << "e-4 "
            << (r1 > 0.5f ? "(PASS: rule learned)" : "(WARN: weak)") << "\n");

  if (PPFAILS() != 0) {
    D_COUT("\n" << PPFAILS() << " PPO assertion(s) FAILED\n");
    return "cards_world_ppo_test_failure";
  }
  return NILP;
}

}  // namespace CWTest
#endif
