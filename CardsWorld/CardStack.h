// Copyright AStarship <https://astarship.net>.

#pragma once
#ifndef _iGeekCardsWorldCoreCardStackH
#define _iGeekCardsWorldCoreCardStackH
#include <_Config.h>
#if _Seam >= _iGeekCardsWorldCore
#include "Card.h"
namespace iGeek {
namespace CardsWorld {

/* A stack of playing cards.
A CardStack stores pointers to the Card objects owned by a Deck. Cards are
drawn from the top (index count-1). The stack is backed by a fixed-size array
of Card* (max 54 cards), which is plenty for any single deck. */
class CardStack {
  enum {
    CardStackMax = 54,  //< Max cards in a single stack (deck + jokers).
  };
  ISC card_count_min_;  //< The minimum number of cards in this stack.
  ISC card_count_max_;  //< The maximum number of cards in this stack.
  BOL is_visible_;      //< Flags if the stack is face-up.
  Card* cards_[CardStackMax];  //< The cards, bottom to top.
  ISC count_;                 //< The number of cards in the stack.

 public:
  /* Constructs an empty card stack. */
  CardStack();

  /* Verbose constructor. */
  CardStack(ISC min_cards, ISC max_cards, BOL is_visible);

  /* Copies all cards from the given stack. */
  CardStack(CardStack& other);

  /* Destructor. Does not delete cards (the Deck owns them). */
  virtual ~CardStack() {}

  /* Compares this stack to other by point value. */
  ISC Compare(CardStack& other);

  /* Returns the point value total of this stack. */
  ISC PointValue();

  /* Fisher-Yates shuffle. */
  void Shuffle();

  /* Returns the number of cards in this stack. */
  ISC CardCount() { return count_; }
  ISC CardCountMin() { return card_count_min_; }
  ISC CardCountMax() { return card_count_max_; }

  /* Removes all cards from the stack. */
  void Clear() { count_ = 0; }

  /* Adds the card to the top of the stack. @return 0 ok, 2 over max. */
  ISC AddCard(Card* new_card);

  /* Inserts the card at index. @return 0 ok, 1 bad index, 2 over max. */
  ISC InsertCard(Card* new_card, ISC index);

  /* Copies all cards from the given stack into this one. */
  ISC AddCardStack(CardStack& cards);

  /* Draws cards_to_take cards from the source stack.
  @return the number drawn, or a negative error code. */
  ISC DrawCards(CardStack& source, ISC cards_to_take);

  /* Removes the first occurrence of card. */
  BOL RemoveCard(Card* card);

  /* Peeks the card at index without removing it. @return nil if out of range. */
  Card* PeekCard(ISC index);

  /* Returns and removes the card at index. @return nil if out of range. */
  Card* TakeCard(ISC index);

  /* Returns and removes the top card. @return nil if empty. */
  Card* TakeNextCard();

  /* Returns and removes a random card. @return nil if empty. */
  Card* TakeRandomCard();

  /* Returns true if the stack is empty. */
  BOL IsEmpty() { return count_ == 0; }
  BOL IsVisible() { return is_visible_; }
  void SetVisibility(BOL visible) { is_visible_ = visible; }
};

}  //< namespace CardsWorld
} //< namespace iGeek
#endif
#endif
