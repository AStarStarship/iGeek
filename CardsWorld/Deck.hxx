// Copyright AStarship <https://astarship.net>.

#include "Deck.h"
#if SEAM >= IGEEK_DECK
namespace _ {
namespace CardsWorld {

Deck::Deck(BOL deck_contains_jokers, ISC aces_are_high, SuitCulture culture)
    : has_jokers_(deck_contains_jokers),
      culture_(culture),
      aces_high_(aces_are_high == 0 ? 1 : aces_are_high),
      card_count_(deck_contains_jokers ? CardsCountWithJokers
                                       : CardsCountDefault) {
  // Build the standard 52 cards, sorted by suit then rank.
  // Ace (pip 1) first in each suit, then 2-10 (pip 2-10), J(11) Q(12) K(13).
  ISC ace_value = aces_high_ == AcesLow ? 1 : AcesHigh;
  for (ISC suit = 1; suit <= 4; ++suit) {
    // Ace.
    ISC idx = (suit - 1) * 13;
    cards_[idx] = Card(1, suit, ace_value, 11, culture);
    for (ISC pip = 2; pip <= 13; ++pip) {
      ISC face_value = pip;
      ISC point_value = (pip >= 11) ? 10 : pip;
      cards_[idx + (pip - 1)] = Card(pip, suit, face_value, point_value,
                                     culture);
    }
  }
  // Add jokers if requested (pip 0).
  if (has_jokers_) {
    cards_[52] = Card(0, 1, 0, 0, culture);  // "Black" joker.
    cards_[53] = Card(0, 2, 0, 0, culture);  // "Red" joker.
  }
}

CardStack Deck::Stock() const {
  CardStack stock;
  stock.SetVisibility(false);
  for (ISC i = 0; i < card_count_; ++i) stock.AddCard(const_cast<Card*>(&cards_[i]));
  return stock;
}

const CHA* Deck::SuitString(ISC suit) const {
  if (suit < 1 || suit > 4) return "Invalid";
  switch (culture_) {
    case SuitCulture::kGerman:
      switch (suit) {
        case 1: return "Acorns";
        case 2: return "Bells";
        case 3: return "Hearts";
        default: return "Lieves";
      }
    case SuitCulture::kSwissGerman:
      switch (suit) {
        case 1: return "Acorns";
        case 2: return "Bells";
        case 3: return "Roses";
        default: return "Shields";
      }
    case SuitCulture::kPiacentine:
    case SuitCulture::kNapoletane:
    case SuitCulture::kSpagnole:
    case SuitCulture::kBergamasche:
      switch (suit) {
        case 1: return "Clubs";
        case 2: return "Coins";
        case 3: return "Cups";
        default: return "Swords";
      }
    default:  // French.
      switch (suit) {
        case 1: return "Clubs";
        case 2: return "Diamonds";
        case 3: return "Hearts";
        default: return "Spades";
      }
  }
}

}  //< namespace CardsWorld
}  //< namespace _
#endif
