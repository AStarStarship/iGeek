// Copyright AStarship <https://astarship.net>.
//
// AUTO-GENERATED from __Seams.yaml (iGeek PacWorld).
// PacWorld core seam unit: the unit tests for the deterministic parts of the
// PacWorld engine. Gated on IGEEK_PACWORLD_CORE (the first / default seam).
#if SEAM >= IGEEK_PACWORLD_CORE
#if SEAM == IGEEK_PACWORLD_CORE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif
namespace PacWorldTest {
using namespace ::_;
using namespace iGeek;

static void Say(const char* s) {
  unsigned long n = 0; while (s[n]) ++n;
  write(1, s, n);
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
  ISC B = gym.EnvCount();
  TTensor obs = TTensorAlloc(B, PacWorldEnv::ObsLength);
  for (ISC i = 0; i < B; ++i)
    for (ISC j = 0; j < PacWorldEnv::ObsLength; ++j)
      obs.At(i, j) = gym.Observations()[i * PacWorldEnv::ObsLength + j];
  TTensor logits = TTensorAlloc(B, PacWorldEnv::ActionCount);
  TTensor values = TTensorAlloc(B, 1);
  pol.Forward(obs, logits, values);
  FPC total = 0.0f;
  for (ISC i = 0; i < B; ++i) {
    ISC best = 0; FPC bv = logits.AtC(i, 0);
    for (ISC a = 1; a < PacWorldEnv::ActionCount; ++a)
      if (logits.AtC(i, a) > bv) { bv = logits.AtC(i, a); best = a; }
    gym.Actions()[i] = best;
  }
  gym.StepBatch();
  for (ISC i = 0; i < B; ++i) total += gym.Rewards()[i];
  TTensorFree(obs); TTensorFree(logits); TTensorFree(values);
  return total / (FPC)B;
}
inline const CHA* Core(const CHA* args) {
#if SEAM >= IGEEK_PACWORLD_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing PacWorld (maze + ghosts + pellets)..."));

  int fails = 0;

  // --- Check 1: TLinearPolicy learns at envs=8 ---
  {
    PacWorldGym gym(8, 11, 11, 7);
    TLinearPolicy pol(PacWorldEnv::ObsLength, PacWorldEnv::ActionCount, 11);
    pol.SetLearnRate(0.1f);
    TPPO ppo(gym, pol, 4, 4);
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

  // --- Check 2: TTransformer runs clean on PacWorldGym (block biases) ---
  {
    PacWorldGym gym(8, 11, 11, 7);
    TTransformer pol(PacWorldEnv::ObsLength, PacWorldEnv::ActionCount,
                     /*dim=*/16, /*layers=*/2);
    pol.SetLearnRate(0.05f);
    TPPO ppo(gym, pol, 4, 2);
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
  return NILP;
}

}  //< namespace PacWorldTest
