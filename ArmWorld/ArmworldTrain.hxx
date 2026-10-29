// Copyright AStarship <https://astarship.net>.
// ArmWorld PPO training: learns a policy to reach targets with the 2-link arm.
// The TPPO loop resets the Gym at each rollout, so this is a short-horizon
// learning signal, not full-episode reaching competence.
#include "ArmGym.hxx"
#include "../PPO.hxx"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
using namespace _;
static FPD Seconds() {
  timespec time;
  if (clock_gettime(CLOCK_MONOTONIC, &time) != 0) return 0;
  return FPD(time.tv_sec) + FPD(time.tv_nsec) * 1e-9;
}
static BOL ParseUInt(const CHA* text, IUD& value) {
  if (text == NILP || *text == 0) return false;
  for (const CHA* p = text; *p; ++p) if (*p < '0' || *p > '9') return false;
  value = IUD(strtoull(text, NILP, 10));
  return value > 0;
}
static void Help() {
  D_COUT(
      "  --updates N     PPO training updates (default 500)\n"
      "  --horizon N     Steps per rollout (default 128)\n"
      "  --envs N        Parallel environments (default 4, max 8)\n"
      "  --seed N        RNG seed (default 7)\n"
      "  --lr N          Learning rate (default 0.005)\n"
      "  --epochs N      PPO epochs per rollout (default 4)\n"
      "  --report N      Print stats every N updates (default 25)\n"
      "  --help\n");
}
/** Evaluate: run the policy greedily (argmax) for K episodes, count reaches. */
static void Evaluate(const TArmParams& params, ISC count, IUD seed,
    TLinearPolicy& policy, ISC episodes, FPC& mean_reward, IUD& reaches) {
  AArmGym gym(params, count, seed);
  FPC total_reward = 0;
  reaches = 0;
  ISC A = gym.ActionCount();
  TTensor obs = TTensorAlloc(count, AArmEnv::ObsLength);
  TTensor logits = TTensorAlloc(count, A);
  TTensor values = TTensorAlloc(count, 1);
  for (ISC ep = 0; ep < episodes; ++ep) {
    FPC ep_reward = 0;
    gym.ResetBatch();
    for (ISC step = 0; step < AArmEnv::MaxFrames; ++step) {
      BOL all_done = true;
      for (ISC i = 0; i < count; ++i)
        if (gym.Terminals()[i] == 0) all_done = false;
      if (all_done) break;
      for (ISC i = 0; i < count; ++i)
        for (ISC j = 0; j < AArmEnv::ObsLength; ++j)
          obs.At(i, j) = gym.Observations()[i * AArmEnv::ObsLength + j];
      policy.Forward(obs, logits, values);
      for (ISC i = 0; i < count; ++i) {
        ISC best = 0; FPC best_logit = logits.AtC(i, 0);
        for (ISC a = 1; a < A; ++a)
          if (logits.AtC(i, a) > best_logit) { best_logit = logits.AtC(i, a); best = a; }
        gym.Actions()[i] = best;
      }
      gym.StepBatch();
      for (ISC i = 0; i < count; ++i) ep_reward += gym.Rewards()[i];
    }
    total_reward += ep_reward;
    for (ISC i = 0; i < count; ++i)
      if (gym.LastStatus(i) == ArmReach) ++reaches;
  }
  TTensorFree(obs); TTensorFree(logits); TTensorFree(values);
  mean_reward = total_reward / FPC(episodes);
}
// CLI PPO training run for the 2-link arm (converted from a standalone ISC
// main() — now a callable). Not pulled by the test seam (the test suite has
// its own PpoSmoke()); kept available for a future train-tool seam. Returns 0
// on success, 2 on a bad flag. (D_COUT/fD_COUT kept as legacy debt.)
ISC RunTrain(ISC argc, CHA** argv) {
  ISC updates = 500, horizon = 128, envs = 4, epochs = 4, report = 25;
  IUD seed = 7;
  FPC lr = 0.005f;
  for (ISC i = 1; i < argc; ++i) {
    const CHA* flag = argv[i];
    if (strcmp(flag, "--help") == 0) { Help(); return 0; }
    if (i + 1 >= argc) { fD_COUT(stderr,  << flag << ); return 2; }
    const CHA* value = argv[++i];
    if (strcmp(flag, "--updates") == 0) {
      IUD v; if (!ParseUInt(value, v) || v > 100000) return 2; updates = ISC(v);
    } else if (strcmp(flag, "--horizon") == 0) {
      IUD v; if (!ParseUInt(value, v) || v < 4 || v > 512) return 2; horizon = ISC(v);
    } else if (strcmp(flag, "--envs") == 0) {
      IUD v; if (!ParseUInt(value, v) || v < 1 || v > GymEnvMax) return 2; envs = ISC(v);
    } else if (strcmp(flag, "--seed") == 0) {
      if (!ParseUInt(value, seed)) return 2;
    } else if (strcmp(flag, "--lr") == 0) {
      lr = FPC(strtof(value, NILP));
      if (lr <= 0 || lr > 0.1f) return 2;
    } else if (strcmp(flag, "--epochs") == 0) {
      IUD v; if (!ParseUInt(value, v) || v > 32) return 2; epochs = ISC(v);
    } else if (strcmp(flag, "--report") == 0) {
      IUD v; if (!ParseUInt(value, v)) return 2; report = ISC(v);
    } else { fD_COUT(stderr, , flag); return 2; }
  }
  TArmParams params;
  D_COUT("updates=" << updates <<  << horizon <<  << envs <<  << epochs <<  << (unsigned long)seed <<);
  AArmGym gym(params, envs, seed);
  TLinearPolicy policy(AArmEnv::ObsLength, gym.ActionCount(), seed);
  policy.SetLearnRate(lr);
  TPPO trainer(gym, policy, horizon, epochs);
  // Baseline evaluation.
  FPC base_reward = 0; IUD base_reaches = 0;
  Evaluate(params, envs, seed, policy, 4, base_reward, base_reaches);
  D_COUT("base_reward=" << (double)base_reward <<  << (int)base_reaches <<);
  FPD started = Seconds();
  TPPOStats stats = {};
  FPC best_reward = base_reward;
  IUD best_reaches = base_reaches;
  for (ISC update = 1; update <= updates; ++update) {
    stats = trainer.TrainStep();
    if (!isfinite(stats.policy_loss_) || !isfinite(stats.value_loss_) ||
        !isfinite(stats.entropy_)) {
      fD_COUT(stderr, , update);
      return 1;
    }
    if (update % report == 0 || update == updates) {
      FPC eval_reward = 0; IUD eval_reaches = 0;
      Evaluate(params, envs, seed + IUD(update) * 7919, policy, 4,
          eval_reward, eval_reaches);
      if (eval_reward > best_reward) {
        best_reward = eval_reward;
        best_reaches = eval_reaches;
      }
      FPD wall = Seconds() - started;
      D_COUT("update=" << update << " updates=" << updates << " eval_reward=" << (double)stats.policy_loss_ << " reaches=" << (double)stats.value_loss_ << " best=" << (double)stats.policy_loss_ << " wall_s=" << (double)stats.value_loss_ << " fps=" << 1.0 << "\n");
      fflush(stdout);
    }
  }
  FPD wall = Seconds() - started;
  FPC final_reward = 0; IUD final_reaches = 0;
  Evaluate(params, envs, seed + IUD(updates) * 7919, policy, 8,
      final_reward, final_reaches);
  D_COUT("final_reward=" << (double)final_reward <<  << (int)final_reaches <<  << (int)(8 * envs) <<);
  D_COUT("final_reaches=" << final_reaches > 0 ? 1 : 0 <<);
  return 0;
}
