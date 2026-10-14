// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_BLACKJACK_DEMO_TEST_HXX
#define CRABS_BLACKJACK_DEMO_TEST_HXX
#if SEAM >= CARDSWORLD_CORE
namespace _ {
namespace CWTest {
inline const CHA* Demo(const CHA* args) {
  A_TEST_BEGIN;
  TBlackjackDemoGym gym(123), same(123), other(456);
  ISC nonzero = 0, different = 0;
  for (ISC i = 0; i < gym.Batch * gym.Features; ++i) {
    if (gym.obs[i] != same.obs[i]) return "seed_replay_failed";
    nonzero += gym.obs[i] != 0;
    different += gym.obs[i] != other.obs[i];
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
      if (gym.obs[i] != same.obs[i]) return "autoreset_replay_failed";
  }
  TLinearPolicy policy(gym.Features, 2, 777);
  policy.Build(777);
  TBlackjackDemoScore score = TBlackjackDemoEval(policy, 900001, 128, 1);
  TBlackjackDemoScore replay = TBlackjackDemoEval(policy, 900001, 128, 1);
  if (score.count != 1024 || score.sum_sq <= 0 ||
      score.sum != replay.sum || score.sum_sq != replay.sum_sq ||
      !isfinite(score.Mean()) || !isfinite(score.Error()))
    return "heldout_evaluation_missing";
  TBlackjackDemoScore before = TBlackjackDemoEval(policy, 700001, 1024, 0);
  TBlackjackDemoGym training(12345);
  TPPO trainer(training, policy, 32, 1);
  policy.SetLearnRate(0.1f);
  TBlackjackDemoTrain(trainer, 1000);
  TBlackjackDemoScore after = TBlackjackDemoEval(policy, 700001, 1024, 0);
  if (!isfinite(after.Mean()) || after.Mean() < before.Mean() + 0.1)
    return "training_improvement_missing";
  return NILP;
}
}  //< namespace CWTest
}  //< namespace _
#endif
#endif
