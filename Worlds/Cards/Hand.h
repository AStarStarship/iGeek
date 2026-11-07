// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekCardsWorldCoreHandH
#define _iGeekCardsWorldCoreHandH
#include <_Config.h>
#if _Seam >= _iGeekCardsWorldCore
#include "CardStack.h"
namespace iGeek {
namespace CardsWorld {

/* A hand of cards in a playing card game.
A Hand is a CardStack with a minimum and maximum card count. In Blackjack,
a hand starts with 2 cards and can grow as the player hits. */
class Hand : public CardStack {
 public:
  /* Default constructor. */
  Hand();

  /* Constructor with min/max card counts. */
  Hand(ISC min_cards, ISC max_cards);

  /* Draws 2 cards from the stock (a starting hand). */
  ISC DealTwoCards(CardStack& stock);

  /* Returns a one-line description of this hand. */
  const CHA* Describe(CHA* buffer, ISC buffer_size);
};

}       //< namespace CardsWorld
}       //< namespace iGeek
#endif  //< _Seam >= _iGeekCardsWorldCore
#endif  //< _iGeekCardsWorldCore
