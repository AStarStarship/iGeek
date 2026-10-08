// Copyright AStarship <https://astarship.net>.

#include "Blackjack.h"
#if SEAM >= IGEEK_BLACKJACK
namespace _ {
namespace CardsWorld {

Blackjack::Blackjack(Player* player)
    : dealer_(Dealer()),
      player_(player),
      dealer_hand_(2, 54),
      player_turn_(false),
      round_over_(false),
      outcome_(Outcome::kInvalid),
      player_bet_(0) {
  if (player != NILP) dealer_.AddPlayer(player);
}

ISC Blackjack::HandValue(const Hand& hand) const {
  // Total the point values, counting aces as 11 then demoting to 1 as needed.
  ISC total = 0;
  ISC aces = 0;
  for (ISC i = 0; i < hand.CardCount(); ++i) {
    const Card* c = hand.PeekCard(i);
    if (c == NILP) continue;
    ISC v = c->PointValue();
    if (c->PipValue() == 1) {
      // Ace: count as 11 tentatively.
      v = 11;
      ++aces;
    }
    total += v;
  }
  // Demote aces from 11 to 1 while over 21.
  while (total > 21 && aces > 0) {
    total -= 10;
    --aces;
  }
  return total;
}

BOL Blackjack::IsBlackjack(const Hand& hand) const {
  return hand.CardCount() == 2 && HandValue(hand) == 21;
}

void Blackjack::NewRound() {
  // Reshuffle the stock if it's running low (less than 15 cards left).
  if (dealer_.GetStock().CardCount() < 15) {
    dealer_.GetStock() = dealer_.GetDeck().Stock();
    dealer_.GetStock().Shuffle();
  }

  // Reset.
  player_->GetHand() = Hand(2, 54);
  dealer_hand_ = Hand(2, 54);
  player_turn_ = true;
  round_over_ = false;
  outcome_ = Outcome::kInvalid;

  // Deal 2 cards to the player.
  player_->GetHand().DealTwoCards(dealer_.GetStock());

  // Deal 2 cards to the dealer (second card face-down, but we track it).
  dealer_hand_.DealTwoCards(dealer_.GetStock());

  // Check for naturals first.
  if (IsBlackjack(player_->GetHand()) && IsBlackjack(dealer_hand_)) {
    outcome_ = Outcome::kPlayerPush;
    round_over_ = true;
    player_turn_ = false;
  } else if (IsBlackjack(player_->GetHand())) {
    outcome_ = Outcome::kPlayerBlackjack;
    round_over_ = true;
    player_turn_ = false;
  } else if (IsBlackjack(dealer_hand_)) {
    outcome_ = Outcome::kDealerBlackjack;
    round_over_ = true;
    player_turn_ = false;
  }
}

BOL Blackjack::Hit() {
  if (!player_turn_ || round_over_) return false;
  Card* next = dealer_.GetStock().TakeNextCard();
  if (next == NILP) {
    // Stock exhausted: force-stand to avoid an infinite loop.
    Stand();
    return false;
  }
  player_->GetHand().AddCard(next);
  if (IsBust(player_->GetHand())) {
    outcome_ = Outcome::kPlayerBust;
    round_over_ = true;
    player_turn_ = false;
    return false;
  }
  // A 21 auto-stands.
  if (HandValue(player_->GetHand()) == 21) {
    Stand();
    return false;
  }
  return true;  // Still the player's turn.
}

Outcome Blackjack::Stand() {
  if (!player_turn_ || round_over_) return outcome_;
  player_turn_ = false;

  // Dealer draws to 17 (stands on all 17s). Guard against stock exhaustion.
  while (HandValue(dealer_hand_) < 17) {
    Card* next = dealer_.GetStock().TakeNextCard();
    if (next == NILP) break;  // Stock exhausted; dealer stands.
    dealer_hand_.AddCard(next);
  }

  ISC player_val = HandValue(player_->GetHand());
  ISC dealer_val = HandValue(dealer_hand_);

  if (dealer_val > 21) {
    outcome_ = Outcome::kPlayerWin;  // Dealer busted.
  } else if (player_val > dealer_val) {
    outcome_ = Outcome::kPlayerWin;
  } else if (player_val < dealer_val) {
    outcome_ = Outcome::kDealerWin;
  } else {
    outcome_ = Outcome::kPlayerPush;
  }
  round_over_ = true;
  return outcome_;
}

ISC Blackjack::SettleRound() {
  if (!round_over_) return 0;
  ISC reward = 0;
  switch (outcome_) {
    case Outcome::kPlayerBlackjack:
      reward = RewardBlackjack;  // +3 (natural 21 pays 3:2)
      player_->WinAdd();
      break;
    case Outcome::kPlayerWin:
      reward = RewardWin;  // +2
      player_->WinAdd();
      break;
    case Outcome::kPlayerPush:
      reward = 0;  // push: no change
      break;
    case Outcome::kPlayerBust:
      reward = RewardBust;  // -2
      break;
    case Outcome::kDealerWin:
      reward = RewardLose;  // -2
      break;
    case Outcome::kDealerBlackjack:
      reward = RewardLose;  // -2
      break;
    default:
      reward = 0;
      break;
  }
  // Apply the reward to the player's credit balance (positive = win,
  // negative = loss). This is the running total the agent observes.
  player_->PointsAdd(reward);
  return reward;
}

}  //< namespace CardsWorld
}  //< namespace _
#endif
