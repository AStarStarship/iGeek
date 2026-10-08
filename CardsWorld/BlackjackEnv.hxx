// Copyright AStarship <https://astarship.net>.

#include "BlackjackEnv.h"
#if SEAM >= IGEEK_BLACKJACKENV
namespace _ {

/* A simple baseline policy: hit until the hand value is >= 17.
The percept format is "P:<val> D:<dealer_up> H:<hidden?>\n<cards>".
We parse the player value and dealer up-card value from it. */
ISC PolicyHitTo17(const CHA* percept) {
  // Scan for "P:<val>" and "D:<val>".
  ISC player_val = 0, dealer_val = 0;
  const CHA* cursor = percept;
  while (*cursor != 0) {
    if (*cursor == 'P' && cursor[1] == ':') {
      ++cursor;
      while (*cursor == ':') ++cursor;
      while (*cursor >= '0' && *cursor <= '9') {
        player_val = player_val * 10 + (*cursor - '0');
        ++cursor;
      }
      continue;
    }
    if (*cursor == 'D' && cursor[1] == ':') {
      ++cursor;
      while (*cursor == ':') ++cursor;
      while (*cursor >= '0' && *cursor <= '9') {
        dealer_val = dealer_val * 10 + (*cursor - '0');
        ++cursor;
      }
      continue;
    }
    ++cursor;
  }
  // Hit if under 17, else stand.
  return player_val < 17 ? 0 : 1;  // 0=hit, 1=stand.
}

BlackjackEnv::BlackjackEnv()
    : game_(NILP),
      agent_(NILP),
      total_reward_(0),
      rounds_played_(0),
      on_(true) {
  for (ISC i = 0; i < PerceptBufferMax; ++i) percept_[i] = 0;
  // Create the agent and the game.
  agent_ = new CardsWorld::Player("Agent", 100);
  game_ = new CardsWorld::Blackjack(agent_);
  Reset();
}

BlackjackEnv::~BlackjackEnv() {
  if (game_ != NILP) delete game_;
  if (agent_ != NILP) delete agent_;
  game_ = NILP;
  agent_ = NILP;
}

void BlackjackEnv::Reset() {
  if (game_ != NILP) game_->NewRound();
  percept_[0] = 0;
}

const CHA* BlackjackEnv::Observe() {
  if (game_ == NILP) return percept_;
  const CardsWorld::Hand& player_hand = game_->PlayerHand();
  const CardsWorld::Hand& dealer_hand = game_->DealerHand();
  ISC player_val = game_->HandValue(player_hand);
  // Dealer up-card is the first card; the rest are hidden.
  ISC dealer_up = 0;
  if (dealer_hand.CardCount() > 0) {
    const CardsWorld::Card* up = dealer_hand.PeekCard(0);
    if (up != NILP) dealer_up = up->PointValue();
  }
  ISC dealer_hidden = dealer_hand.CardCount() - 1;
  if (dealer_hidden < 0) dealer_hidden = 0;

  // Build the percept: "P:<val> D:<up> H:<hidden> R:<round_over>\n<cards>"
  CHA* cursor = percept_;
  CHA* stop = percept_ + PerceptBufferMax - 1;

  auto emit_num = [&](ISC value) {
    if (cursor >= stop - 2) return;
    CHA digits[12];
    ISC n = 0;
    if (value == 0) digits[n++] = '0';
    while (value > 0 && n < 11) {
      digits[n++] = static_cast<CHA>('0' + (value % 10));
      value /= 10;
    }
    for (ISC i = n - 1; i >= 0 && cursor < stop - 1; --i) *cursor++ = digits[i];
  };

  if (cursor < stop - 1) *cursor++ = 'P';
  if (cursor < stop - 1) *cursor++ = ':';
  emit_num(player_val);
  if (cursor < stop - 1) *cursor++ = ' ';
  if (cursor < stop - 1) *cursor++ = 'D';
  if (cursor < stop - 1) *cursor++ = ':';
  emit_num(dealer_up);
  if (cursor < stop - 1) *cursor++ = ' ';
  if (cursor < stop - 1) *cursor++ = 'H';
  if (cursor < stop - 1) *cursor++ = ':';
  emit_num(dealer_hidden);
  if (cursor < stop - 1) *cursor++ = ' ';
  if (cursor < stop - 1) *cursor++ = 'R';
  if (cursor < stop - 1) *cursor++ = ':';
  emit_num(game_->RoundOver() ? 1 : 0);
  if (cursor < stop - 1) *cursor++ = '\n';

  // Emit the player's cards as "Ac 2d" style tokens.
  for (ISC i = 0; i < player_hand.CardCount() && cursor < stop - 2; ++i) {
    const CardsWorld::Card* c = player_hand.PeekCard(i);
    if (c == NILP) break;
    CHA pip_char = '0';
    switch (c->PipValue()) {
      case 1: pip_char = 'A'; break;
      case 11: pip_char = 'J'; break;
      case 12: pip_char = 'Q'; break;
      case 13: pip_char = 'K'; break;
      default: pip_char = static_cast<CHA>('0' + c->PipValue()); break;
    }
    CHA suit_char = '?';
    switch (c->SuitValue()) {
      case 1: suit_char = 'c'; break;
      case 2: suit_char = 'd'; break;
      case 3: suit_char = 'h'; break;
      case 4: suit_char = 's'; break;
    }
    *cursor++ = pip_char;
    *cursor++ = suit_char;
    if (cursor < stop - 1 && i < player_hand.CardCount() - 1) *cursor++ = ' ';
  }
  *cursor = 0;
  return percept_;
}

ISC BlackjackEnv::Step(ISC action) {
  if (game_ == NILP || game_->RoundOver()) return 0;
  if (action == static_cast<ISC>(CardsWorld::Action::kHit)) {
    game_->Hit();
  } else {
    game_->Stand();
  }
  if (game_->RoundOver()) {
    ISC reward = game_->SettleRound();
    total_reward_ += reward;
    ++rounds_played_;
    return reward;
  }
  return 0;  // Round still in progress.
}

BOL BlackjackEnv::Done() const {
  return game_ == NILP ? true : game_->RoundOver();
}

CardsWorld::Outcome BlackjackEnv::LastOutcome() const {
  return game_ == NILP ? CardsWorld::Outcome::kInvalid
                       : game_->GetOutcome();
}

ISC BlackjackEnv::Credit() const {
  return agent_ == NILP ? 0 : agent_->PointsCount();
}

void BlackjackEnv::ComputeReward(Crabs* crabs, FPD achieved_goal,
                                 FPD desired_goal, CHA* info) {
  (void)crabs;
  // The reward is the difference between what we achieved and what we wanted.
  // A perfect round would hit exactly 21 (desired=21); busting scores low.
  FPD reward = achieved_goal - desired_goal;
  if (info != NILP) {
    // Write a short diagnostic like "a:<achieved> d:<desired> r:<reward>".
    info[0] = 0;
  }
}

ISC BlackjackEnv::RunPolicy(ISC (*policy)(const CHA*), ISC rounds) {
  ISC total = 0;
  for (ISC r = 0; r < rounds; ++r) {
    Reset();
    while (!Done()) {
      const CHA* obs = Observe();
      ISC action = policy(obs);
      total += Step(action);
    }
  }
  return total;
}

}  //< namespace _
#endif
