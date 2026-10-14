// Copyright AStarship <https://astarship.net>.
#include "BlackjackDemo.hxx"
#include <stdio.h>
using namespace _;
/** CSV is deliberately stdout-only; redirect bulk results outside git. */
ISC main() {
  printf("policy_seed,kind,updates,train_episodes,eval_seed,episodes,mean,se\n");
  for (IUD seed = 777; seed < 780; ++seed) {
    TBlackjackDemoGym gym(12345 + seed);
    TLinearPolicy policy(gym.Features, 2, seed);
    policy.Build(seed);
    policy.SetLearnRate(0.1f);
    TPPO trainer(gym, policy, 32, 1);
    auto emit = [&](const CHA* kind, ISC updates, IUD eval_seed, ISC mode,
                    ISC batches) {
      TBlackjackDemoScore score = TBlackjackDemoEval(policy, eval_seed,
                                                    batches, mode);
      printf("%d,%s,%d,%d,%d,%d,%.8f,%.8f\n",
             ISC(seed), kind, updates, updates * 256, ISC(eval_seed), score.count,
             score.Mean(), score.Error());
    };
    emit("before", 0, 1000001, 0, 12500);
    emit("random", 0, 1000001, 1, 12500);
    emit("hit17", 0, 1000001, 2, 12500);
    for (ISC update = 100; update <= 1000; update += 100) {
      TBlackjackDemoTrain(trainer, 100);
      emit("validation", update, 700001, 0, 1024);
    }
    emit("after", 1000, 1000001, 0, 12500);
  }
  return ferror(stdout) ? 1 : 0;
}
