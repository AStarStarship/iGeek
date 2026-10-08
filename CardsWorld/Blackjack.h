// Copyright AStarship <https://astarship.net>.

#pragma once
#ifndef IGEEK_CARDSBLACKJACK_DECL
#define IGEEK_CARDSBLACKJACK_DECL
#include <_Config.h>
#if SEAM >= IGEEK_BLACKJACK
#include "Dealer.h"
namespace _ {
namespace CardsWorld {

/* The player's action in a round of Blackjack. */
enum class Action : ISC {
  kHit = 0,   //< Take another card.
  kStand = 1, //< Stop taking cards.
  kInvalid = -1,
};

/* The outcome of a round of Blackjack. */
enum class Outcome : ISC {
  kPlayerBlackjack = 0,  //< Player got a natural 21.
  kPlayerWin = 1,        //< Player beats the dealer without busting.
  kPlayerPush = 2,       //< Player ties the dealer.
  kPlayerBust = 3,       //< Player went over 21.
  kDealerWin = 4,        //< Dealer beats the player.
  kDealerBlackjack = 5,  //< Dealer got a natural 21.
  kInvalid = -1,
};

/* The reward the player receives for a round outcome. */
enum {
  RewardBlackjack = 3,   //< Natural 21 pays 3:2, we use 3.
  RewardWin = 2,         //< Normal win.
  RewardPush = 1,        //< Push: no loss, no gain (use 1 as neutral).
  RewardBust = -2,       //< Player busted.
  RewardLose = -2,       //< Dealer won.
};

/* Blackjack game state and rules.
This is the core game engine: it holds the dealer, the player's current hand,
the dealer's current hand, and the round outcome. It implements standard
blackjack rules: player hits/stands, dealer draws to 17, aces are 1 or 11,
face cards are 10, and a natural 21 (blackjack) pays 3:2. */
class Blackjack {
  Dealer dealer_;        //< The dealer (owns deck + stock + players).
  Player* player_;       //< The player we're playing against the dealer.
  Hand dealer_hand_;     //< The dealer's current hand.
  BOL player_turn_;      //< True while the player is taking turns.
  BOL round_over_;       //< True once the round is decided.
  Outcome outcome_;      //< The round outcome.
  ISC player_bet_;       //< The player's current bet.

 public:
  /* Constructor. Sets up a 1-deck, aces-high, French blackjack table.
  @param player The player to seat at the table. */
  explicit Blackjack(Player* player);

  /* Returns the dealer. */
  Dealer& GetDealer() { return dealer_; }

  /* Returns the player. */
  Player* GetPlayer() { return player_; }

  /* Returns the player's current hand. */
  Hand& PlayerHand() { return player_->GetHand(); }

  /* Returns the dealer's current hand. */
  Hand& DealerHand() { return dealer_hand_; }

  /* Returns the total point value of a hand, optimally counting aces.
  Aces count as 11 while the total is <= 21, then drop to 1. */
  ISC HandValue(const Hand& hand) const;

  /* Returns true if the hand is a natural blackjack (2 cards totaling 21). */
  BOL IsBlackjack(const Hand& hand) const;

  /* Returns true if the hand busts (> 21). */
  BOL IsBust(const Hand& hand) const { return HandValue(hand) > 21; }

  /* Returns true if the player's turn is active. */
  BOL PlayerTurn() const { return player_turn_; }

  /* Returns true if the round is over. */
  BOL RoundOver() const { return round_over_; }

  /* Returns the round outcome. */
  Outcome GetOutcome() const { return outcome_; }

  /* Starts a new round: resets hands, deals 2 cards to player and dealer
  (dealer's second card face-down), and checks for naturals. */
  void NewRound();

  /* The player takes a hit. @return false if the player busted. */
  BOL Hit();

  /* The player stands: dealer draws to 17, round is decided.
  @return the round outcome. */
  Outcome Stand();

  /* Applies the round outcome to the player's credit and win count.
  @return the reward for the outcome. */
  ISC SettleRound();
};

}  //< namespace CardsWorld
}  //< namespace _
#endif
#endif
