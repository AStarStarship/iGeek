// Copyright AStarship <https://astarship.net>.

#pragma once
#ifndef IGEEK_CARDSCARDDECL
#define IGEEK_CARDSCARDDECL
#include <_Config.h>
#if SEAM >= IGEEK_DECK
#include "Card.h"
#include "CardStack.h"
namespace _ {
namespace CardsWorld {

/* A Deck of Cards.
The Deck owns an array of Card objects. By default it builds a standard
52-card French deck (4 suits x 13 ranks). A Deck can be aces-high or
aces-low, and optionally include two jokers (54 cards). */
class Deck {
 public:
  enum {
    CardsCountDefault = 52,      //< Standard deck without jokers.
    CardsCountWithJokers = 54,   //< Standard deck with two jokers.
    AcesHigh = 14,               //< Ace face value when aces are high.
    AcesLow = 1,                 //< Ace face value when aces are low.
    JokersYes = 1,
    JokersNo = 0,
  };

 private:
  BOL has_jokers_;        //< True if the deck has two jokers.
  SuitCulture culture_;   //< The suit culture.
  ISC aces_high_;         //< 14 if aces are high, 1 if aces are low.
  ISC card_count_;        //< Number of cards in the deck.
  Card cards_[54];        //< The Card objects, owned by this deck.

 public:
  /* Constructor. @param deck_contains_jokers 1 for jokers, 0 without.
  @param aces_are_high AcesHigh (14) or AcesLow (1).
  @param culture The suit culture. */
  Deck(BOL deck_contains_jokers = JokersNo, ISC aces_are_high = AcesHigh,
       SuitCulture culture = SuitCulture::kFrench);

  /* Returns true if the deck has jokers. */
  BOL HasJokers() const { return has_jokers_; }

  /* Returns the suit culture. */
  SuitCulture Culture() const { return culture_; }

  /* Returns the number of cards in the deck. */
  ISC CardCount() const { return card_count_; }

  /* Returns the ace face value (14 or 1). */
  ISC GetAcesHigh() const { return aces_high_; }

  /* Returns a pointer to the card at index, or nil if out of range. */
  Card* GetCard(ISC index) {
    if (index < 0 || index >= card_count_) return NILP;
    return &cards_[index];
  }

  /* Returns a const pointer to the card at index, or nil if out of range. */
  const Card* GetCard(ISC index) const {
    if (index < 0 || index >= card_count_) return NILP;
    return &cards_[index];
  }

  /* Builds a CardStack containing pointers to all cards in the deck.
  The returned stack does not own the cards; the deck does. */
  CardStack Stock() const;

  /* Returns the suit name for the given suit value. */
  const CHA* SuitString(ISC suit) const;
};

}  //< namespace CardsWorld
}  //< namespace _
#endif
#endif
