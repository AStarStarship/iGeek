// Copyright AStarship <https://astarship.net>.

#include "Card.h"
#if SEAM >= IGEEK_CARD
namespace _ {
namespace CardsWorld {

/* Returns the suit name for the given suit index (1-4) and culture. */
static const CHA* SuitNameFor(ISC suit, SuitCulture culture) {
  switch (culture) {
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

/* Sets the point value from the pip value: aces are 11, J/Q/K are 10,
else the pip value. */
static ISC PointValueFromPip(ISC pip) {
  if (pip == 1) return 11;
  if (pip >= 11 && pip <= 13) return 10;
  return pip;
}

Card::Card()
    : pip_value_(1),
      face_value_(1),
      point_value_(11),
      suit_value_(1),
      suit_(CardSuit::kClub),
      culture_(SuitCulture::kFrench),
      suit_name_("Clubs") {}

Card::Card(ISC pip_value, ISC suit)
    : pip_value_(pip_value < 1 ? 1 : (pip_value > 13 ? 13 : pip_value)),
      face_value_(pip_value < 1 ? 1 : (pip_value > 13 ? 13 : pip_value)),
      point_value_(PointValueFromPip(pip_value)),
      suit_value_(suit < 1 ? 1 : (suit > 4 ? 4 : suit)),
      suit_(static_cast<CardSuit>(suit < 1 ? 1 : suit)),
      culture_(SuitCulture::kFrench),
      suit_name_("Clubs") {
  SetSuitName();
}

Card::Card(ISC pip_value, ISC suit, ISC face_value, ISC point_value,
           SuitCulture culture)
    : pip_value_(pip_value < 1 ? 1 : (pip_value > 13 ? 13 : pip_value)),
      face_value_(face_value < 1 ? 1 : (face_value > 14 ? 14 : face_value)),
      point_value_(point_value < 0 ? 0 : point_value),
      suit_value_(suit < 1 ? 1 : (suit > 4 ? 4 : suit)),
      suit_(static_cast<CardSuit>(suit < 1 ? 1 : suit)),
      culture_(culture),
      suit_name_("Clubs") {
  SetSuitName();
}

ISC Card::Compare(const Card& other) const {
  if (pip_value_ > other.pip_value_) return 1;
  if (pip_value_ < other.pip_value_) return -1;
  if (suit_value_ > other.suit_value_) return 1;
  if (suit_value_ < other.suit_value_) return -1;
  return 0;
}

BOL Card::Equals(const Card& other) const { return Compare(other) == 0; }

void Card::SetCulture(SuitCulture culture) {
  if (culture < SuitCulture::kFrench ||
      culture > SuitCulture::kBergamasche)
    return;
  culture_ = culture;
  SetSuitName();
}

void Card::SetSuitName() { suit_name_ = SuitNameFor(suit_value_, culture_); }

}  //< namespace CardsWorld
}  //< namespace _
#endif
