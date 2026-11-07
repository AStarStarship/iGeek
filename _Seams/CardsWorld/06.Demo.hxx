// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekCardsWorldCore
namespace CardsWorldTest {
using namespace ::_;
using namespace iGeek; 

inline const CHA* Demo(const CHA* args) {
  _ATestBegin;
  TBlackjackDemoGym gym(123), same(123), other(456);
  {
    const EnvironmentManifest& m = gym.Manifest();
    if (!EnvironmentManifestValidate(&m)) return "manifest_invalid";
    if (!TSEquals(m.name, "blackjack-demo")) return "manifest_name_mismatch";
    if (m.observation_count != ISC(gym.Features) || m.action_count != ISC(2))
      return "manifest_shape_mismatch";
    if (m.environment_count_max != ISC(gym.Batch))
      return "manifest_batch_mismatch";
  }
  ISC nonzero = 0, different = 0;
  for (ISC i = 0; i < gym.Batch * gym.Features; ++i) {
    if (gym.observation[i] != same.observation[i]) return "seed_replay_failed";
    nonzero += gym.observation[i] != 0;
    different += gym.observation[i] != other.observation[i];
  }
  if (nonzero != gym.Batch * 3 || different == 0)
    return "seeded_playable_observation_missing";
  for (ISC step = 0; step < 64; ++step) {
    ISC expected[gym.Batch];
    for (ISC i = 0; i < gym.Batch; ++i) {
      gym.action[i] = same.action[i] = (step + i) % 2;
    }
    // Settle the identical seeded engine directly, before the adapter.
    for (ISC i = 0; i < gym.Batch; ++i) {
      expected[i] = same.envs[i].Step(same.action[i]);
      if (!same.envs[i].Done()) expected[i] += same.envs[i].Step(1);
    }
    gym.StepBatch();
    for (ISC i = 0; i < gym.Batch; ++i) {
      if (gym.done[i] != 1 || gym.reward[i] != FPC(expected[i]))
        return "one_decision_settlement_missing";
      same.Deal(i);
    }
    for (ISC i = 0; i < gym.Batch * gym.Features; ++i)
      if (gym.observation[i] != same.observation[i]) return "autoreset_replay_failed";
  }
  TLinearPolicy policy(gym.Features, 2, 777);
  policy.Build(777);
  TBlackjackDemoScore score = TBlackjackDemoEval(policy, 900001, 128, 1);
  TBlackjackDemoScore replay = TBlackjackDemoEval(policy, 900001, 128, 1);
  if (score.count != 1024 || score.sum_sq <= 0 ||
      score.sum != replay.sum || score.sum_sq != replay.sum_sq ||
      !iGeek::IsFinite(score.Mean()) || !iGeek::IsFinite(score.Error()))
    return "heldout_evaluation_missing";
  TBlackjackDemoScore before = TBlackjackDemoEval(policy, 700001, 1024, 0);
  TBlackjackDemoGym training(12345);
  PPO trainer(training, policy, 32, 1);
  policy.SetLearnRate(0.1f);
  TBlackjackDemoTrain(trainer, 1000);
  TBlackjackDemoScore after = TBlackjackDemoEval(policy, 700001, 1024, 0);
  if (!iGeek::IsFinite(after.Mean()) || after.Mean() < before.Mean() + 0.1)
    return "training_improvement_missing";
  return _NilP;
}
}  //< namespace CardsWorldTest
#endif
