// Copyright AStarship <https://astarship.net>.

#pragma once
#ifndef IGEEK_CARDSBLACKJACKENV_DECL
#define IGEEK_CARDSBLACKJACKENV_DECL
#include <_Config.h>
#if SEAM >= IGEEK_BLACKJACKENV
#include "Blackjack.h"
namespace _ {

/* A Blackjack environment for training agents.
A BlackjackEnv wraps the CardsWorld::Blackjack game engine and exposes the
iGeek agent-training interface:
  - Reset()           starts a new round.
  - Observe()         returns a compact text percept of the table state.
  - Step(action)      applies an action (hit/stand) and returns the reward.
  - Done()            reports whether the round is over.
  - ComputeReward()   the iGeek Env interface, scores achieved vs desired.
The percept is a nil-terminated CHA string the agent can tokenize.

Note: this Env is standalone (not a TRoom) so it builds on the current
ASCIICrabs core without depending on the TRoom template's Main()/Star()
methods. It can be re-wrapped in a TRoom once those are stabilized.
*/
class BlackjackEnv {
 public:
  enum {
    PerceptBufferMax = 256,  //< Max length of the observation string.
  };

  /* Constructor. Creates a new blackjack table for a single agent. */
  BlackjackEnv();

  /* Destructor. */
  ~BlackjackEnv();

  /* iGeek Env interface: scores the achieved goal against the desired goal.
  @param crabs The Crabs expression (nil for console).
  @param achieved_goal The goal state achieved this step.
  @param desired_goal The goal state the agent was aiming for.
  @param info A pointer to a CHA buffer for diagnostics. */
  void ComputeReward(Crabs* crabs, FPD achieved_goal, FPD desired_goal,
                     CHA* info);

  /* Starts a new round of blackjack. */
  void Reset();

  /* Returns a compact text observation of the table state.
  The string is nil-terminated and suitable for tokenization. */
  const CHA* Observe();

  /* Applies an action to the environment.
  @param action Action::kHit to take a card, Action::kStand to stop.
  @return The reward for this step (0 while the round is still in progress,
  non-zero once the round settles). */
  ISC Step(ISC action);

  /* Returns true if the current round is over. */
  BOL Done() const;

  /* Returns the last round's outcome. */
  CardsWorld::Outcome LastOutcome() const;

  /* Access the underlying game for diagnostics. */
  CardsWorld::Blackjack& Game() { return *game_; }

  /* The agent's credit balance. */
  ISC Credit() const;

  /* Plays N rounds with the given agent policy.
  @param policy A function pointer: (const CHA* percept) -> action code.
  @param rounds The number of rounds to play.
  @return The total reward accumulated (integer points). */
  ISC RunPolicy(ISC (*policy)(const CHA*), ISC rounds);

 private:
  CardsWorld::Blackjack* game_;   //< The blackjack game engine.
  CardsWorld::Player* agent_;     //< The agent's player.
  CHA percept_[PerceptBufferMax];  //< The observation buffer.
  ISC total_reward_;               //< Accumulated reward (integer points).
  ISC rounds_played_;              //< Number of rounds completed.
  BOL on_;                         //< Whether the env is running.
};

}  //< namespace _
#endif
#endif
