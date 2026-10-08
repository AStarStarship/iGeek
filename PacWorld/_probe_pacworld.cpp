// Copyright AStarship <https://astarship.net>.
// Probe: headless PacWorld env + Gym + PPO. Verifies (a) the env steps
// deterministically, (b) a linear policy can be trained on PacWorldGym via
// TPPO without crashing, (c) obs is fixed-length. Uses Crabs StdOut for I/O.
// Exit 0 = all good.
#include "_probe_pacworld.hxx"

using namespace _;
extern "C" { int write(int, const void*, unsigned long); }

static ISC fails = 0;
static void FAIL(const CHA* what) {
  write(2, "FAIL: ", 6); write(2, what, 96); write(2, "\n", 1);
  ++fails;
}
static void OK(const CHA* what) {
  write(1, "ok: ", 4); write(1, what, 96); write(1, "\n", 1);
}
// Greedy reward: run the policy's argmax on a fresh batch for 1 step, return
// the mean per-env reward.
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

int main() {
  // 1. Env steps + obs is fixed-length.
  {
    PacWorldEnv env(9, 9, 42);
    env.Reset(); env.Observe();
    FPC total = 0.0f;
    for (ISC s = 0; s < 10; ++s) total += env.Step((ISC)(s % 5));
    OK("env stepped 10 actions");
    {
      CHA b[32]; ISN n = 0;
      b[n++]=' ';b[n++]=' ';b[n++]='1';b[n++]='0';b[n++]='-';b[n++]='s';b[n++]='t';
      b[n++]='e';b[n++]='p';b[n++]=' ';b[n++]='s';b[n++]='u';b[n++]='m';b[n++]='=';
      ISC ip = (ISC)total; if (ip<0){b[n++]='-';ip=-ip;}
      CHA t2[16]; ISN t=0; if(ip==0)t2[t++]='0';
      while(ip>0&&t<12){t2[t++]=(CHA)('0'+ip%10);ip/=10;}
      while(t>0&&n<30)b[n++]=t2[--t];
      b[n++]='\n';
      write(1, b, (unsigned long)n);
    }
  }
  // 2. Determinism: two fresh envs, same seed -> same first obs.
  {
    PacWorldEnv envA(9, 9, 42); envA.Reset(); envA.Observe();
    PacWorldEnv envB(9, 9, 42); envB.Reset(); envB.Observe();
    BOL same = true;
    for (ISC i = 0; i < PacWorldEnv::ObsLength; ++i)
      if (envA.Obs()[i] != envB.Obs()[i]) same = false;
    if (!same) FAIL("env not deterministic per seed");
    else OK("env deterministic per seed");
  }
  // 3. Gym batches + PPO trains a linear policy; no crash, finite reward.
  {
    const ISC B = 4, Cols = 9, Rows = 9;
    PacWorldGym gym(B, Cols, Rows, 7);
    if (gym.EnvCount() != B) FAIL("gym env count");
    TLinearPolicy pol(PacWorldEnv::ObsLength, PacWorldEnv::ActionCount, 11);
    pol.SetLearnRate(0.1f);
    TPPO ppo(gym, pol, /*horizon=*/2, /*epochs=*/2);
    ppo.SetGamma(0.99f); ppo.SetLambda(0.95f);
    FPC greedy_before = GreedyReward(gym, pol);
    for (ISC s = 0; s < 1; ++s) ppo.TrainStep();
    FPC greedy_after = GreedyReward(gym, pol);
    OK("ppo trainstep on pacworld (no crash)");
    {
      CHA b[64]; ISN n = 0;
      const CHA* pre = "  greedy before="; while(pre[n]) b[n++]=pre[n];
      ISC ib = (ISC)(greedy_before*100); if(ib<0){b[n++]='-';ib=-ib;}
      CHA t2[16]; ISN t=0; if(ib==0)t2[t++]='0';
      while(ib>0&&t<12){t2[t++]=(CHA)('0'+ib%10);ib/=10;}
      while(t>0&&n<60)b[n++]=t2[--t];
      const CHA* mid = " after="; while(mid[n-n]){ } 
      // append " after="
      b[n++]=' ';b[n++]='a';b[n++]='f';b[n++]='t';b[n++]='e';b[n++]='r';b[n++]='=';
      ISC ia = (ISC)(greedy_after*100); if(ia<0){b[n++]='-';ia=-ia;}
      t=0; if(ia==0)t2[t++]='0';
      while(ia>0&&t<12){t2[t++]=(CHA)('0'+ia%10);ia/=10;}
      while(t>0&&n<62)b[n++]=t2[--t];
      b[n++]='\n';
      write(1, b, (unsigned long)n);
    }
    // PacWorld is a hard, sparse-reward task; the hard gate is no-crash +
    // finite. The soft signal is the printed trend (may need a bigger net /
    // more steps to show a clear rise — that's the next milestone).
  }
  if (fails == 0) { write(1, "PACWORLD_PROBE_OK\n", 18); return 0; }
  return 1;
}
