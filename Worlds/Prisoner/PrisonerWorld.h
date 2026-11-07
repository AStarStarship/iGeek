/// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekPrisonerWorldH
#define _iGeekPrisonerWorldH

#include "Environment.h"
#include "EnvironmentGoal.h"
#include "Gym.h"
#include "Stringf.hpp"
#if _Seam >= _iGeekPrisonerWorldCore
namespace iGeek {
/* Prisoner's Dilemma repeated-game environment for iGeek.
 * Two agents simultaneously choose Cooperate (0) or Defect (1).
 * Payoffs: T > R > P > S (Temptation > Reward > Punishment > Sucker).
 * Typical values: T=5, R=3, P=1, S=0.
 *
 * The iGeek Env contract:
 *   - Step(action) computes the per-step reward and stores it.
 *   - ComputeReward() is an interface hook that returns the last computed reward.
 *   - achieved_goal/desired_goal are set by Step() as IUD datums (0=C, 1=D).
 */
class PrisonerEnvironment : public Environment {
 public:
  enum { NumAgents = 2, MaxHistory = 64 };

  PrisonerEnvironment(const CHA* env_name = "prisoner", ISC state_count = 2)
      : Environment(env_name, state_count),
        round_(0),
        payoff_T_(5), payoff_R_(3), payoff_P_(1), payoff_S_(0),
        step_reward_(0) {
    /* The manifest is the in-Room contract a joining agent reads before it
       plays: what the env is, and the observation/action/reward shape. */
    TSPrint(manifest_.name, ISW(ManifestNameLen), "prisoner");
    TSPrint(manifest_.version, ISW(ManifestVersionLen), "1.0");
    manifest_.kind = ManifestKindEnv;
    manifest_.domain = ManifestDomainGame;
    TSPrint(manifest_.description, ISW(ManifestDescriptionLen),
            "Repeated Prisoner's Dilemma; two agents choose Cooperate or Defect.");
    TSPrint(manifest_.modality, ISW(ManifestModalityLen), "vector");
    /* Rewards run from the sucker payoff up to the temptation payoff. */
    manifest_.reward_low = FPC(payoff_S_);
    manifest_.reward_high = FPC(payoff_T_);
    manifest_.reward_higher_is_better = true;
    manifest_.determinism = ManifestDeterminismSeeded;
    manifest_.observation_count = ISC(NumAgents);  // last action per agent.
    manifest_.action_count = ISC(2);               // Cooperate, Defect.
    manifest_.max_episode_steps = ISC(MaxHistory);
    manifest_.environment_count_max = ISC(GymEnvMax);
    TSPrint(manifest_.world_name, ISW(ManifestWorldLen), "PrisonerWorld");
    manifest_.seam_number = _iGeekPrisonerWorldCore;
    manifest_.manifest_version = ISC(1);
  }

  /* Get the scalar reward for this step.
   * @param crabs       The Crabs to read/write the reward to. May be _NilP.
   * @param achieved    What the agent actually did this step (IUD: 0=Cooperate, 1=Defect).
   * @param desired     What we want the agent to do (IUD: 0=Cooperate, 1=Defect).
   * @param info        Human-readable info string. May be _NilP.
   * @return            The scalar reward for this step (FPC). */
  FPC ComputeReward(Crabs* crabs, EnvironmentGoal achieved_goal,
                    EnvironmentGoal desired_goal, const CHA* info) override {
    // Interface hook: return the last step's reward
    (void)crabs; (void)achieved_goal; (void)desired_goal; (void)info;
    return step_reward_;
  }

  /* Reset the environment to a fresh episode. */
  void Reset() override {
    round_ = 0;
    for (ISC i = 0; i < NumAgents; ++i) {
      agent_history_[i] = 0;
      agent_desired_[i] = 0;  // default: Cooperate
    }
    step_reward_ = 0;
  }

  /* True when the current episode is over. */
  BOL Done() override { return round_ >= MaxHistory; }

  /* Step the environment forward one round.
   * @param actions  Array of IUD actions from each agent (0=Cooperate, 1=Defect).
   * @param rewards  Output array of FPC rewards for each agent (filled with step_reward_).
   * @param terminals Output array of terminal flags for each agent.
   */
  void Step(const IUD* actions, FPC* rewards, BOL* terminals) {
    // Compute rewards for both agents based on prisoner's dilemma matrix
    for (ISC i = 0; i < NumAgents; ++i) {
      IUD a = agent_history_[i];   // achieved action
      IUD d = agent_desired_[i];   // desired action
      FPC reward = payoff_R_;      // default: both Cooperate

      if (a == 0 && d == 0) reward = payoff_R_;  // Both Cooperate
      else if (a == 1 && d == 1) reward = payoff_P_;  // Both Defect
      else if (a == 0 && d == 1) reward = payoff_S_;  // I cooperate, you defect
      else if (a == 1 && d == 0) reward = payoff_T_;  // I defect, you cooperate

      rewards[i] = reward;
    }
    step_reward_ = rewards[0];  // use agent 0's reward as the shared step reward

    // Record action in history (bit-packed: bit 0 = most recent)
    for (ISC i = 0; i < NumAgents; ++i) {
      agent_history_[i] = (agent_history_[i] << 1) | actions[i];
    }
    // Set desired for next step (default: Cooperate)
    for (ISC i = 0; i < NumAgents; ++i) {
      agent_desired_[i] = 0;
    }
    ++round_;
  }

  /* Set the prisoner's dilemma payoff parameters.
   * T=temptation, R=reward, P=punishment, S=sucker. */
  void SetPayoffs(IUD T, IUD R, IUD P, IUD S) {
    payoff_T_ = T; payoff_R_ = R; payoff_P_ = P; payoff_S_ = S;
    /* Keep the manifest's reward range in step with the payoff matrix. */
    manifest_.reward_low = FPC(payoff_S_);
    manifest_.reward_high = FPC(payoff_T_);
  }

  /* Get the current round number. */
  ISC Round() { return round_; }

  /* Get the total number of agents. */
  ISC AgentCount() { return NumAgents; }

 private:
  ISC round_;
  IUD payoff_T_, payoff_R_, payoff_P_, payoff_S_;
  FPC step_reward_;
  IUD agent_history_[NumAgents];   // bit-packed choice history per agent
  IUD agent_desired_[NumAgents];   // desired action per agent per step
};

/* Exported symbol: the one-and-only Prisoner environment instance.
 * Defined in PrisonerWorld.cpp. Other translation units can access it
 * as a variable: PrisonerEnvironmentironmentPtr.
 */
extern PrisonerEnvironment* PrisonerEnvironmentironmentPtr;

} //< namespace iGeek
#endif  //< _Seam >= _iGeekPrisonerWorldCore
#endif  //< _iGeekPrisonerWorldH