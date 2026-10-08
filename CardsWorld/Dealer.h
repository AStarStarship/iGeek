// Copyright AStarship <https://astarship.net>.

#pragma once
#ifndef IGEEK_CARDSDEALER_DECL
#define IGEEK_CARDSDEALER_DECL
#include <_Config.h>
#if SEAM >= IGEEK_DEALER
#include "Deck.h"
#include "Hand.h"
#include "Player.h"
namespace _ {
namespace CardsWorld {

/* A dealer in a card game.
The Dealer owns the Deck, the shuffled stock (the pile players draw from),
and a fixed array of up to 8 players. The dealer manages the ante, the pot,
the bets, and the dealing. */
class Dealer {
 public:
  enum {
    MaxPlayers = 8,      //< Max simultaneous players.
    AnteDefault = 5,     //< Default ante.
    MinBetDefault = 1,   //< Default minimum bet.
    MinCardsDefault = 2, //< Default min cards per hand.
    MaxCardsDefault = 54,//< Default max cards per hand.
  };

 private:
  ISC ante_;              //< The current ante.
  ISC min_bet_;           //< The minimum bet.
  ISC min_cards_per_hand_;  //< Min cards per hand.
  ISC max_cards_per_hand_;  //< Max cards per hand.
  ISC pot_total_;         //< Points in the pot.
  ISC player_count_;      //< Number of players in this game.
  ISC current_player_;    //< Index of the current player's turn.
  Deck deck_;             //< The deck (owns the Card objects).
  CardStack stock_;       //< The shuffled stock players draw from.
  Player* players_[MaxPlayers];  //< The players.
  ISC players_owned_[MaxPlayers];  //< 1 if we own the player, 0 if borrowed.

 public:
  /* Constructor. @param ante The starting ante.
  @param min_bet The minimum bet.
  @param min_cards The min cards per hand.
  @param max_cards The max cards per hand. */
  Dealer(ISC ante = AnteDefault, ISC min_bet = MinBetDefault,
         ISC min_cards = MinCardsDefault, ISC max_cards = MaxCardsDefault);

  /* Destructor. Deletes owned players. */
  virtual ~Dealer();

  /* Adds a player to the game. @return the player index, or -1 if full. */
  ISC AddPlayer(Player* new_player);

  /* Returns the number of players. */
  ISC PlayerCount() const { return player_count_; }

  /* Returns a pointer to the player at index, or nil if out of range. */
  Player* GetPlayer(ISC index) const {
    if (index < 0 || index >= player_count_) return NILP;
    return players_[index];
  }

  /* Returns the deck. */
  Deck& GetDeck() { return deck_; }

  /* Returns the stock. */
  CardStack& GetStock() { return stock_; }

  /* Returns the current ante. */
  ISC GetAnte() const { return ante_; }
  void SetAnte(ISC ante) { ante_ = ante < 0 ? 0 : ante; }

  /* Returns the minimum bet. */
  ISC GetMinBet() const { return min_bet_; }
  void SetMinBet(ISC value) { min_bet_ = value < 1 ? 1 : value; }

  /* Returns the pot total. */
  ISC GetPotTotal() const { return pot_total_; }
  void SetPotTotal(ISC total) { pot_total_ = total < 0 ? 0 : total; }
  void AddToPot(ISC points) { pot_total_ += points; }

  /* Shuffles the stock and resets the game. */
  virtual void StartNewGame();

  /* Resets all players' hands and deals a fresh 2 cards to each. */
  virtual void Redeal();

  /* Advances to the next player's turn. @return the new current player index. */
  ISC NextPlayer();

  /* Returns the current player's turn index. */
  ISC CurrentPlayer() const { return current_player_; }
};

}  //< namespace CardsWorld
}  //< namespace _
#endif
#endif
