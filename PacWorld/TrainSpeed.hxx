// Copyright AStarship <https://astarship.net>.
// Headless training-speed probe: N real TPPO::TrainStep() on PacWorldGym @ -O2.
// Reports elapsed + steps/sec. The "fast RL" number.
#include "_probe_pacworld.hxx"
using namespace _;
extern "C" {
  int write(int, const void*, unsigned long);
  struct timespec;  // from <time.h>; wall-clock microsecond precision.
  int clock_gettime(int clk_id, struct timespec* ts);
}
// CLOCK_MONOTONIC = 1 on Linux.
static IUD NowUS() {
  struct timespec ts = {0, 0};
  if (clock_gettime(1, &ts) != 0) return 0;
  return IUD(ts.tv_sec) * 1000000 + IUD(ts.tv_nsec) / 1000;
}

static FPC GreedyReward(PacWorldGym& gym, TPolicy& pol) {
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
// Append an int (base-10) to buf, return new length. Takes IUD (64-bit) so
// large step/sec figures don't overflow a 32-bit ISC.
static ISN Itoa(CHA* buf, ISN n, IUD v) {
  CHA t[24]; ISN k = 0;
  if (v == 0) t[k++] = '0';
  while (v > 0 && k < 22) { t[k++] = (CHA)('0' + (v % 10)); v /= 10; }
  while (k > 0) buf[n++] = t[--k];
  return n;
}
// Append a fixed-point S.CC from a double (using ISC ms = sec*1000).
static ISN Fx(CHA* buf, ISN n, ISC milli) {
  ISC ip = milli / 1000; ISC cc = (milli % 1000) / 10;
  n = Itoa(buf, n, ip);
  buf[n++] = '.';
  buf[n++] = (CHA)('0' + cc / 10);
  buf[n++] = (CHA)('0' + cc % 10);
  return n;
}
static ISN Append(CHA* buf, ISN n, const CHA* s) {
  while (*s) buf[n++] = *s++;
  return n;
}

int main(int argc, char** argv) {
  ISC steps = 50, envs = 4, horizon = 4, epochs = 4;
  if (argc > 1) steps = (ISC)atol(argv[1]);
  if (argc > 2) envs = (ISC)atol(argv[2]);
  if (argc > 3) horizon = (ISC)atol(argv[3]);
  if (argc > 4) epochs = (ISC)atol(argv[4]);

  PacWorldGym gym(envs, 11, 11, 7);
  TLinearPolicy pol(PacWorldEnv::ObsLength, PacWorldEnv::ActionCount, 11);
  pol.SetLearnRate(0.1f);
  TPPO ppo(gym, pol, horizon, epochs);
  ppo.SetGamma(0.99f); ppo.SetLambda(0.95f);
  FPC before = GreedyReward(gym, pol);

  IUD u0 = NowUS();
  for (ISC s = 0; s < steps; ++s) ppo.TrainStep();
  IUD u1 = NowUS();
  FPC after = GreedyReward(gym, pol);

  IUD micro = u1 - u0;
  IUD sps = (micro > 0) ? ((IUD)steps * 1000000) / micro : 0;

  CHA line[160]; ISN n = 0;
  n = Itoa(line, n, (IUD)steps);
  n = Append(line, n, " TrainStep() in ");
  // print as S.SSSs (sec + 3 decimals) from micro.
  {
    IUD s = micro / 1000000; IUD frac = (micro % 1000000) / 1000;
    n = Itoa(line, n, s);
    line[n++] = '.';
    line[n++] = (CHA)('0' + (frac / 100) % 10);
    line[n++] = (CHA)('0' + (frac / 10) % 10);
    line[n++] = (CHA)('0' + frac % 10);
  }
  n = Append(line, n, " s = ");
  n = Itoa(line, n, sps);
  n = Append(line, n, " steps/s | greedy before=");
  if (before < 0.0f) line[n++] = '-';
  n = Itoa(line, n, (before < 0.0f) ? (IUD)(-before * 100.0) : (IUD)(before * 100.0));
  n = Append(line, n, " after=");
  if (after < 0.0f) line[n++] = '-';
  n = Itoa(line, n, (after < 0.0f) ? (IUD)(-after * 100.0) : (IUD)(after * 100.0));
  n = Append(line, n, "\n");
  write(1, line, (unsigned long)n);
  return 0;
}
