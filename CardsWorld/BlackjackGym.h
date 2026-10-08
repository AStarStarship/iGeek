// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CARDSBLACKJACKGYM_DECL
#define IGEEK_CARDSBLACKJACKGYM_DECL
#include <_Config.h>
#if SEAM >= IGEEK_BLACKJACKGYM
#include "Blackjack.h"
#include "../Tensor.h"
namespace _ {
namespace CardsWorld {

/* A vectorized Blackjack environment for PPO training.
   Puffer-informed: batches N parallel blackjack tables and exposes the flat
   obs/actions/rewards/terminals/action-mask arrays the TPPO loop steps in one
   call (the PufferLib `Env*`/`Agent*` analog). The iGeek `Gym` class carries
   the same shape generically; this is the CardsWorld concrete adapter over
   the `Blackjack` engine.

   Observation encoding (4 features, fixed length, Puffer "scalar vector"):
     [0] player hand value (0..21)
     [1] dealer up-card point value (0..10)
     [2] dealer hidden-card count (0..1)
     [3] round-over flag (0.0 running / 1.0 done)
   Action space: 2 (kHit=0, kStand=1). Reward: the engine's round reward
   (ISC, in {-2,+1,+2,+3}), exposed as FPC.
*/
class BlackjackGym {
 public:
  enum {
    ObsLength = 4,       //< Fixed observation feature count.
    ActionCount = 2,     //< hit / stand.
    TableMax = 8,        //< Max parallel tables (fixed, no std vector).
  };

  /* @param tables Number of parallel blackjack tables (1..TableMax). */
  explicit BlackjackGym(ISC tables);
  ~BlackjackGym();

  ISC Tables() const { return table_count_; }

  /* Reset every table to a fresh round (Puffer puf_reset). */
  void ResetBatch();

  /* Step the whole batch by actions_[b]; fills rewards_/terminals_/
     observations_. One call = one batch step (Puffer puf_step).
     @param actions One action (0=hit,1=stand) per table, length tables_. */
  void StepBatch(const ISC* actions);

  /* --- Flat batch arrays (owned, preallocated at init) ---------------- */
  FPC* Observations() { return observations_; }  //< [tables_][ObsLength]
  FPC* Rewards() { return rewards_; }            //< [tables_]
  FPC* Terminals() { return terminals_; }        //< [tables_] 1.0=done
  FPC* ActionMask() { return action_mask_; }     //< [tables_][ActionCount]

  /* Per-table access for diagnostics. */
  Blackjack& Table(ISC i) { return *tables_[i]; }

  /* The total reward summed across the batch for the last step. */
  FPC LastBatchReward() const { return last_batch_reward_; }

 private:
  ISC table_count_;
  Blackjack* tables_[TableMax];   //< Parallel game engines.
  Player* agents_[TableMax];      //< Per-table agents (own their credit).
  FPC* observations_;             //< [table_count_][ObsLength]
  FPC* rewards_;                  //< [table_count_]
  FPC* terminals_;                //< [table_count_]
  FPC* action_mask_;              //< [table_count_][ActionCount]
  FPC last_batch_reward_;
};

}  // namespace CardsWorld
}  // namespace _
#endif
#endif
