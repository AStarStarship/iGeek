/// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_PRISONERWORLD_H
#define CRABS_PRISONERWORLD_H

#include "Env.h"
#include "EnvGoal.h"
#if SEAM >= CRABS_PRISONERWORLD
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
class PrisonerEnv : public Env {
 public:
  enum { NumAgents = 2, MaxHistory = 64 };

  PrisonerEnv(const CHA* env_name = "prisoner", ISC state_count = 2)
      : Env(env_name, state_count),
        round_(0),
        payoff_T_(5), payoff_R_(3), payoff_P_(1), payoff_S_(0),
        step_reward_(0) {}

  /* Get the scalar reward for this step.
   * @param crabs       The Crabs to read/write the reward to. May be NILP.
   * @param achieved    What the agent actually did this step (IUD: 0=Cooperate, 1=Defect).
   * @param desired     What we want the agent to do (IUD: 0=Cooperate, 1=Defect).
   * @param info        Human-readable info string. May be NILP.
   * @return            The scalar reward for this step (FPC). */
  FPC ComputeReward(Crabs* crabs, EnvGoal achieved_goal,
                    EnvGoal desired_goal, const CHA* info) override {
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
  }

  /* Get the current round number. */
  ISC Round() const { return round_; }

  /* Get the total number of agents. */
  ISC AgentCount() const { return NumAgents; }

 private:
  ISC round_;
  IUD payoff_T_, payoff_R_, payoff_P_, payoff_S_;
  FPC step_reward_;
  IUD agent_history_[NumAgents];   // bit-packed choice history per agent
  IUD agent_desired_[NumAgents];   // desired action per agent per step
};

/* Exported symbol: the one-and-only Prisoner environment instance.
 * Defined in PrisonerWorld.cpp. Other translation units can access it
 * as a variable: PrisonerEnvPtr.
 */
extern PrisonerEnv* PrisonerEnvPtr;

} //< namespace iGeek
#endif  // SEAM >= CRABS_PRISONERWORLD
#endif  // CRABS_PRISONERWORLD_H