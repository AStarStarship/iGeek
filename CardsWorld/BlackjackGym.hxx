// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekCardsWorldBlackjackGymHXX
#define _iGeekCardsWorldBlackjackGymHXX
#include "BlackjackGym.h"
#if _Seam >= _iGeekCardsWorldBlackjackGym
namespace iGeek {
namespace CardsWorld {
using namespace ::_;

BlackjackGym::BlackjackGym(ISC tables)
    : table_count_(tables < 1 ? 1 : (tables > TableMax ? TableMax : tables)),
      observations_(_NilP), rewards_(_NilP), terminals_(_NilP),
      action_mask_(_NilP), last_batch_reward_(0.0f) {
  // Preallocate the flat batch arrays (preallocate at ctor, free at dtor).
  observations_ = new FPC[table_count_ * ObsLength]();
  rewards_ = new FPC[table_count_]();
  terminals_ = new FPC[table_count_]();
  action_mask_ = new FPC[table_count_ * ActionCount]();
  // Create one agent + game per table.
  for (ISC i = 0; i < table_count_; ++i) {
    agents_[i] = new Player("Agent", 100);
    tables_[i] = new Blackjack(agents_[i]);
  }
  ResetBatch();
}

BlackjackGym::~BlackjackGym() {
  for (ISC i = 0; i < table_count_; ++i) {
    if (tables_[i] != _NilP) delete tables_[i];
    if (agents_[i] != _NilP) delete agents_[i];
  }
  delete[] observations_;
  delete[] rewards_;
  delete[] terminals_;
  delete[] action_mask_;
  observations_ = rewards_ = terminals_ = action_mask_ = _NilP;
}

void BlackjackGym::ResetBatch() {
  for (ISC i = 0; i < table_count_; ++i) {
    tables_[i]->NewRound();
    // Encode the fresh observation.
    Hand& ph = tables_[i]->PlayerHand();
    Hand& dh = tables_[i]->DealerHand();
    ISC pval = tables_[i]->HandValue(ph);
    ISC dup = 0;
    if (dh.CardCount() > 0) {
      Card* up = dh.PeekCard(0);
      if (up != _NilP) dup = up->PointValue();
    }
    ISC hidden = dh.CardCount() - 1;
    if (hidden < 0) hidden = 0;
    observations_[i * ObsLength + 0] = (FPC)pval;
    observations_[i * ObsLength + 1] = (FPC)dup;
    observations_[i * ObsLength + 2] = (FPC)hidden;
    observations_[i * ObsLength + 3] = tables_[i]->RoundOver() ? 1.0f : 0.0f;
    rewards_[i] = 0.0f;
    terminals_[i] = tables_[i]->RoundOver() ? 1.0f : 0.0f;
    // Both actions legal while the round is running; only stand "legal" once
    // over (a done table has no real action, mask stand=1).
    action_mask_[i * ActionCount + 0] =
        tables_[i]->RoundOver() ? 0.0f : 1.0f;  // hit
    action_mask_[i * ActionCount + 1] = 1.0f;  // stand
  }
  last_batch_reward_ = 0.0f;
}

void BlackjackGym::StepBatch(const ISC* actions) {
  FPC batch_reward = 0.0f;
  for (ISC i = 0; i < table_count_; ++i) {
    if (tables_[i]->RoundOver()) {
      // Done table: no-op, stays done.
      rewards_[i] = 0.0f;
      terminals_[i] = 1.0f;
      continue;
    }
    ISC action = (actions != _NilP) ? actions[i] : 0;
    if (action == static_cast<ISC>(Action::kHit)) {
      tables_[i]->Hit();
    } else {
      tables_[i]->Stand();
    }
    if (tables_[i]->RoundOver()) {
      ISC reward = tables_[i]->SettleRound();
      rewards_[i] = (FPC)reward;
      batch_reward += (FPC)reward;
      terminals_[i] = 1.0f;
    } else {
      rewards_[i] = 0.0f;
      terminals_[i] = 0.0f;
    }
    // Re-encode the observation.
    Hand& ph = tables_[i]->PlayerHand();
    Hand& dh = tables_[i]->DealerHand();
    ISC pval = tables_[i]->HandValue(ph);
    ISC dup = 0;
    if (dh.CardCount() > 0) {
      Card* up = dh.PeekCard(0);
      if (up != _NilP) dup = up->PointValue();
    }
    ISC hidden = dh.CardCount() - 1;
    if (hidden < 0) hidden = 0;
    observations_[i * ObsLength + 0] = (FPC)pval;
    observations_[i * ObsLength + 1] = (FPC)dup;
    observations_[i * ObsLength + 2] = (FPC)hidden;
    observations_[i * ObsLength + 3] = tables_[i]->RoundOver() ? 1.0f : 0.0f;
    action_mask_[i * ActionCount + 0] =
        tables_[i]->RoundOver() ? 0.0f : 1.0f;
    action_mask_[i * ActionCount + 1] = 1.0f;
  }
  last_batch_reward_ = batch_reward;
}

}       //< namespace CardsWorld
}       //< namespace iGeek
#endif  //< _Seam >= _iGeekCardsWorldBlackjackGym
#endif  //< _iGeekCardsWorldBlackjackGymHXX