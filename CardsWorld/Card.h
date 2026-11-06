// Copyright AStarship <https://astarship.net>.
  #pragma once
#ifndef _iGeekCardsWorldCoreSCARD_DECL
#define _iGeekCardsWorldCoreSCARD_DECL
#include <_Config.h>
#if _Seam >= _iGeekCardsWorldCore
namespace iGeek {
namespace CardsWorld {

/* An enumerated list of the different playing card suits (French). */
enum class CardSuit : ISC {
  CSClub = 1,
  CSDiamond = 2,
  CSHeart = 3,
  CSSpade = 4,
};

/* An enumerated list of the different suit cultures. */
enum class SuitCulture : ISC {
  SCFrench = 1,
  SCGerman,
  SCSwissGerman,
  SCPiacentine,
  SCNapoletane,
  SCSpagnole,
  SCBergamasche,
};

enum {
  SuitCultureCount = static_cast<ISC>(SuitCulture::SCBergamasche) + 1,
  CardSuitCount = 4,
};

/* A playing card: a pip value 1-13 (Ace, 2-10, J, Q, K), a suit, a suit
culture, and a mutable point value (for blackjack ace revaluation). */
struct Card {
  ISC pip_value_;      //< 1 = Ace, 2-10, 11 = J, 12 = Q, 13 = K.
  ISC face_value_;     //< The rank value 1-14.
  ISC point_value_;    //< Points this card is worth (0-11, aces 1 or 11).
  ISC suit_value_;     //< 1-4 suit index.
  CardSuit suit_;      //< The suit.
  SuitCulture culture_;  //< The suit culture.
  const CHA* suit_name_;  //< Display name of the suit.

  /* Default constructor: a 2 of Clubs. */
  Card();

  /* Simple constructor. pip_value 1-13, suit 1-4. */
  Card(ISC pip_value, ISC suit);

  /* Verbose constructor. */
  Card(ISC pip_value, ISC suit, ISC face_value, ISC point_value,
       SuitCulture culture);

  /* Compare pip then suit. @return 0 equal, 1 greater, -1 less. */
  ISC Compare(Card& other);

  BOL Equals(Card& other);

  ISC PipValue() { return pip_value_; }
  ISC FaceValue() { return face_value_; }
  ISC PointValue() { return point_value_; }
  void SetPointValue(ISC value) { point_value_ = value; }
  ISC SuitValue() { return suit_value_; }
  CardSuit Suit() { return suit_; }
  SuitCulture Culture() { return culture_; }
  void SetCulture(SuitCulture culture);
  const CHA* SuitName() { return suit_name_; }

  /* Sets the suit name string from the culture + suit. */
  void SetSuitName();
};

}       //< namespace CardsWorld
}       //< namespace iGeek
#endif  //< _Seam >= _iGeekCardsWorldCore
#endif  //< _iGeekCardsWorldCoreSCARD_DECL
