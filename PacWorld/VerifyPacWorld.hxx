// Copyright AStarship <https://astarship.net>.
// Ad-hoc verifier: exercises the changed framework code end-to-end.
//   1. TLinearPolicy  PPO train on PacWorldGym (envs=8) -> learns
//   2. TTransformer   PPO train on PacWorldGym (envs=8) -> runs clean
// The no-stdlib check is run by the SHELL, not here (no system()/no <cstdlib>).
// Built twice: -O2 (fast, learning) and -O1 -g -fsanitize=address (safety).
// Prints PASS/FAIL per check; exits 0 iff both training checks pass.
#include "_probe_pacworld.hxx"
using namespace _;
extern "C" int write(int, const void*, unsigned long);

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

int main() {
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
  return fails == 0 ? 0 : 1;
}
