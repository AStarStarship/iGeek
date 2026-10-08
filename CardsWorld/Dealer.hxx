// Copyright AStarship <https://astarship.net>.

#include "Dealer.h"
#if SEAM >= IGEEK_DEALER
namespace _ {
namespace CardsWorld {

Dealer::Dealer(ISC ante, ISC min_bet, ISC min_cards, ISC max_cards)
    : ante_(ante < 0 ? 0 : ante),
      min_bet_(min_bet < 1 ? 1 : min_bet),
      min_cards_per_hand_(min_cards < 1 ? 1 : min_cards),
      max_cards_per_hand_(max_cards < 1 ? 1 : max_cards),
      pot_total_(0),
      player_count_(0),
      current_player_(0),
      deck_(),
      stock_() {
  for (ISC i = 0; i < MaxPlayers; ++i) {
    players_[i] = NILP;
    players_owned_[i] = 0;
  }
}

Dealer::~Dealer() {
  for (ISC i = 0; i < MaxPlayers; ++i) {
    if (players_owned_[i] && players_[i] != NILP) delete players_[i];
  }
}

ISC Dealer::AddPlayer(Player* new_player) {
  if (new_player == NILP) return -1;
  if (player_count_ >= MaxPlayers) return -1;
  ISC index = player_count_++;
  players_[index] = new_player;
  players_owned_[index] = 0;  // borrowed; caller owns it.
  return index;
}

void Dealer::StartNewGame() {
  // Rebuild the stock from the deck and shuffle.
  stock_ = deck_.Stock();
  stock_.Shuffle();
  pot_total_ = 0;
  current_player_ = 0;
  Redeal();
}

void Dealer::Redeal() {
  for (ISC i = 0; i < player_count_; ++i) {
    if (players_[i] != NILP) {
      Hand fresh;
      players_[i]->GetHand() = fresh;
      players_[i]->GetHand().DealTwoCards(stock_);
    }
  }
}

ISC Dealer::NextPlayer() {
  if (player_count_ == 0) return -1;
  current_player_ = (current_player_ + 1) % player_count_;
  return current_player_;
}

}  //< namespace CardsWorld
}  //< namespace _
#endif
