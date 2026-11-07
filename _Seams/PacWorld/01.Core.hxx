// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekPacWorldCore
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace PacWorldTest {
using namespace ::_;
using namespace iGeek;

static void Say(const CHA* s) {
  RefOut() << s;
}

// Print a float as "SSSS.d" (1 decimal) — clean, no UB, no stdlib.
static void SayFPC(FPC v) {
  ISC neg = v < 0.0f ? 1 : 0;
  FPC a = neg ? -v : v;
  ISC ip = ISC(a); ISC dec = ISC((a - FPC(ip)) * 10.0);
  if (neg) write(1, "-", 1);
  CHA t[16]; int k = 0;
  if (ip == 0) t[k++] = '0';
  while (ip > 0 && k < 15) { t[k++] = (CHA)('0' + ip % 10); ip /= 10; }
  while (k > 0) write(1, &t[--k], 1);
  write(1, ".", 1);
  CHA d = (CHA)('0' + dec);
  write(1, &d, 1);
  write(1, "\n", 1);
}

template <class POL>
static FPC GreedyReward(PacWorldGym& gym, POL& pol) {
  gym.ResetBatch();
  ISC B = gym.EnvironmentCount();
  TensorFPC observation = TensorFPCAlloc(B, PacWorldEnv::ObservationLength);
  for (ISC i = 0; i < B; ++i)
    for (ISC j = 0; j < PacWorldEnv::ObservationLength; ++j)
      observation.At(i, j) = gym.Observations()[i * PacWorldEnv::ObservationLength + j];
  TensorFPC logits = TensorFPCAlloc(B, PacWorldEnv::ActionCount);
  TensorFPC values = TensorFPCAlloc(B, 1);
  pol.Forward(observation, logits, values);
  FPC total = 0.0f;
  for (ISC i = 0; i < B; ++i) {
    ISC best = 0; FPC bv = logits.At(i, 0);
    for (ISC a = 1; a < PacWorldEnv::ActionCount; ++a)
      if (logits.At(i, a) > bv) { bv = logits.At(i, a); best = a; }
    gym.Actions()[i] = best;
  }
  gym.StepBatch();
  for (ISC i = 0; i < B; ++i) total += gym.Rewards()[i];
  TensorFPCFree(observation); TensorFPCFree(logits); TensorFPCFree(values);
  return total / (FPC)B;
}
inline const CHA* Core(const CHA* args) {
#if _Seam >= _iGeekPacWorldCore
  _ATestBegin;
  _DROut(Headingf("Testing PacWorld (maze + ghosts + pellets)..."));

  int fails = 0;

  // --- Check 1: TLinearPolicy learns at envs=8 ---
  {
    PacWorldGym gym(8, 11, 11, 7);
    /* Manifest contract: the in-Room contract for a joining agent. */
    _DCheck(EnvironmentManifestValidate(&gym.Env(0).Manifest()),
            "PacWorld manifest validates");
    _DCheck(TSEquals(gym.Env(0).Manifest().name, "pacworld") != _NilP,
            "PacWorld manifest name == pacworld");
    TLinearPolicy pol(PacWorldEnv::ObservationLength, PacWorldEnv::ActionCount, 11);
    pol.SetLearnRate(0.1f);
    PPO ppo(gym, pol, 4, 4);
    ppo.SetGamma(0.99f); ppo.SetLambda(0.95f);
    FPC before = GreedyReward(gym, pol);
    for (ISC s = 0; s < 1500; ++s) ppo.TrainStep();
    FPC after = GreedyReward(gym, pol);
    Say(after > before ? "PASS linear-learns (envs=8) before=" : "FAIL linear-learns (envs=8) before=");
    SayFPC(before);
    Say("  after=");
    SayFPC(after);
    if (after <= before) ++fails;
  }

  // --- Check 2: TransformerPolicy runs clean on PacWorldGym (block biases) ---
  {
    PacWorldGym gym(8, 11, 11, 7);
    TransformerPolicy pol(PacWorldEnv::ObservationLength, PacWorldEnv::ActionCount,
                     /*dim=*/16, /*layers=*/2);
    pol.SetLearnRate(0.05f);
    PPO ppo(gym, pol, 4, 2);
    ppo.SetGamma(0.99f); ppo.SetLambda(0.95f);
    FPC before = GreedyReward(gym, pol);
    for (ISC s = 0; s < 300; ++s) ppo.TrainStep();
    FPC after = GreedyReward(gym, pol);
    Say(after >= before - 1.0f
           ? "PASS transformer-runs (envs=8) before="
           : "WARN transformer (no crash, reward flat/down) before=");
    SayFPC(before);
    Say("  after=");
    SayFPC(after);
  }

  Say(fails == 0 ? "VERIFY_TRAIN_PASS\n" : "VERIFY_TRAIN_FAILED\n");

#endif
  return _NilP;
}

}  //< namespace PacWorldTest
