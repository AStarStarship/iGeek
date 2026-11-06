// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekCardsWorldCore
#include "../PPO.hxx"
namespace CardsWorldTest {
using namespace ::_;
using namespace iGeek; 

inline ISC& PPFAILS() {
  static ISC count = 0;
  return count;
}
#define PP_CHECK(cond)                                       \
  do {                                                       \
    if (!::_::Test(cond)) {                                  \
      ++PPFAILS();                                           \
      if (_Seam >= CRABS_COUT)                                \
        ::_::RefOut() << "\nFAILURE PPO at line:" << __LINE__ \
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

inline const CHA* PPO(const CHA* args) {
  _ATestBegin;
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
  _DROut("PPO: reward before training (mean) = " << (ISC)(r0 * 1e6f) << "e-6\n");

  // Train.
  for (ISC s = 0; s < TRAIN_STEPS; ++s) {
    ppo.TrainStep();
  }
  FPC r1 = measure();
  _DROut("PPO: reward after " << TRAIN_STEPS << " steps (mean) = "
                              << (ISC)(r1 * 1e6f) << "e-6\n");

  // The policy must have IMPROVED: reward after > reward before.
  PP_CHECK(r1 > r0);
  // And it should have learned the rule: reward close to +1 (the optimum).
  // With 300 steps x 4 epochs on a 2-action linear task, it should be > 0.5.
  PP_CHECK(r1 > 0.5f);
  _DROut("PPO: learned mean reward " << (ISC)(r1 * 1e4f) << "e-4 "
            << (r1 > 0.5f ? "(PASS: rule learned)" : "(WARN: weak)") << "\n");

  if (PPFAILS() != 0) {
    _DROut("\n" << PPFAILS() << " PPO assertion(s) FAILED\n");
    return "cards_world_ppo_test_failure";
  }
  return _NilP;
}

}  //< namespace CardsWorldTest
#endif
