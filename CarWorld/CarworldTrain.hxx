// Copyright AStarship <https://astarship.net>.
// CarWorld PPO training: learns a policy to drive the car through routes.
// Trains for N updates, evaluates periodically, reports reward progression.
// Uses the existing TPPO loop (CollectRollout resets the Gym per rollout,
// so this is a short-horizon learning signal, not full-ride competence).
#include "../BikeWorld/TileRuntime.hxx"
#include "CarGym.hxx"
#include "CarScenarios.hxx"
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
      "  --scenario NAME   straight|curves|obstacle|gravel|ice|uphill|downhill|launch|ice_curves (default straight)\n"
      "  --updates N       PPO training updates (default 200)\n"
      "  --horizon N       Steps per rollout (default 64)\n"
      "  --envs N          Parallel environments (default 4, max 8)\n"
      "  --seed N          RNG seed (default 7)\n"
      "  --lr N            Learning rate (default 0.001)\n"
      "  --epochs N        PPO epochs per rollout (default 4)\n"
      "  --mode driver|raw driver actions (5) or raw actuators (9) (default driver)\n"
      "  --report N        Print stats every N updates (default 10)\n"
      "  --help\n");
}
/** Evaluate: run the policy greedily (argmax) for K episodes, report goals. */
static void Evaluate(const TileMap& map, const TVehicleRoute& route,
    const TCarState& initial, ISC count, IUD seed, ISC mode,
    TLinearPolicy& policy, ISC episodes, FPC& mean_reward, IUD& goals) {
  ACarGym gym(map, route, initial, count, seed, mode);
  FPC total_reward = 0;
  goals = 0;
  ISC A = gym.ActionCount();
  ISC max_steps = ACarEnv::MaxFrames;  // 5000
  // Pre-allocate eval tensors (reuse across steps).
  TTensor obs = TTensorAlloc(count, ACarEnv::ObsLength);
  TTensor logits = TTensorAlloc(count, A);
  TTensor values = TTensorAlloc(count, 1);
  for (ISC ep = 0; ep < episodes; ++ep) {
    FPC ep_reward = 0;
    gym.ResetBatch();
    for (ISC step = 0; step < max_steps; ++step) {
      BOL all_done = true;
      for (ISC i = 0; i < count; ++i)
        if (gym.Terminals()[i] == 0) all_done = false;
      if (all_done) break;
      for (ISC i = 0; i < count; ++i)
        for (ISC j = 0; j < ACarEnv::ObsLength; ++j)
          obs.At(i, j) = gym.Observations()[i * ACarEnv::ObsLength + j];
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
      if (gym.LastStatus(i) == CarGoal) ++goals;
  }
  TTensorFree(obs); TTensorFree(logits); TTensorFree(values);
  mean_reward = total_reward / FPC(episodes);
}
ISC main(ISC argc, CHA** argv) {
  ISC scenario = CarStraight, updates = 200, horizon = 64, envs = 4;
  ISC epochs = 4, report = 10, mode = CarDriverActions;
  IUD seed = 7;
  FPC lr = 0.001f;
  for (ISC i = 1; i < argc; ++i) {
    const CHA* flag = argv[i];
    if (strcmp(flag, "--help") == 0) { Help(); return 0; }
    if (i + 1 >= argc) { fD_COUT(stderr,  << flag << ); return 2; }
    const CHA* value = argv[++i];
    if (strcmp(flag, "--scenario") == 0) {
      scenario = -1;
      for (ISC s = 0; s < CarScenarioCount; ++s)
        if (strcmp(value, TCarScenarioName(s)) == 0) scenario = s;
      if (scenario < 0) { fD_COUT(stderr, , value); return 2; }
    } else if (strcmp(flag, "--updates") == 0) {
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
    } else if (strcmp(flag, "--mode") == 0) {
      mode = strcmp(value, "raw") == 0 ? CarActuatorActions : CarDriverActions;
    } else { fD_COUT(stderr, , flag); return 2; }
  }
  AVehiclePlanner* planner = new AVehiclePlanner;
  TileMap map(1.0f, 120, 64);
  TVehicleRoute route; TCarState initial;
  if (!TCarScenarioBuild(map, scenario, *planner, route, initial)) {
    fD_COUT(stderr, , TCarScenarioName(scenario));
    delete planner; return 1;
  }
  delete planner;
  D_COUT("TCarScenarioName=" << TCarScenarioName(scenario) <<  << updates <<  << horizon <<  << envs <<  << epochs <<  << mode == CarActuatorActions ?  :  <<  << (unsigned long)seed <<);
  ACarGym gym(map, route, initial, envs, seed, mode);
  TLinearPolicy policy(ACarEnv::ObsLength, gym.ActionCount(), seed);
  policy.SetLearnRate(lr);
  TPPO trainer(gym, policy, horizon, epochs);
  // Baseline evaluation.
  FPC base_reward = 0; IUD base_goals = 0;
  Evaluate(map, route, initial, envs, seed, mode, policy, 4, base_reward, base_goals);
  D_COUT("base_reward=" << (double)base_reward <<  << (int)base_goals <<);
  FPD started = Seconds();
  TPPOStats stats = {};
  FPC best_reward = base_reward;
  IUD best_goals = base_goals;
  for (ISC update = 1; update <= updates; ++update) {
    stats = trainer.TrainStep();
    if (!isfinite(stats.policy_loss_) || !isfinite(stats.value_loss_) ||
        !isfinite(stats.entropy_)) {
      fD_COUT(stderr, , update);
      return 1;
    }
    if (update % report == 0 || update == updates) {
      FPC eval_reward = 0; IUD eval_goals = 0;
      Evaluate(map, route, initial, envs, seed + IUD(update) * 7919, mode, policy, 4,
          eval_reward, eval_goals);
      if (eval_reward > best_reward) { best_reward = eval_reward; best_goals = eval_goals; }
      FPD wall = Seconds() - started;
      D_COUT("update=" << update << " updates=" << updates << " eval_reward=" << (double)stats.policy_loss_ << " goals=" << (double)stats.value_loss_ << " best=" << (double)stats.policy_loss_ << " wall_s=" << (double)stats.value_loss_ << " fps=" << 1.0 << "\n");
      fflush(stdout);
    }
  }
  FPD wall = Seconds() - started;
  // Final evaluation with more episodes.
  FPC final_reward = 0; IUD final_goals = 0;
  Evaluate(map, route, initial, envs, seed + IUD(updates) * 7919, mode, policy, 8,
      final_reward, final_goals);
  D_COUT("final_reward=" << (double)final_reward <<  << (int)final_goals <<  << (int)(8 * envs) <<);
  D_COUT("TCarScenarioName=" << TCarScenarioName(scenario) <<  << final_goals > 0 ? 1 : 0 <<);
  return 0;
}
