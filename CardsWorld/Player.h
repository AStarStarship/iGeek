// Copyright AStarship <https://astarship.net>.

#pragma once
#ifndef IGEEK_CARDSPLAYER_DECL
#define IGEEK_CARDSPLAYER_DECL
#include <_Config.h>
#if SEAM >= IGEEK_PLAYER
#include "Hand.h"
namespace _ {
namespace CardsWorld {

/* A player in a card game.
A Player has a name, a point/credit count, a win count, and a current Hand.
The name is stored in a fixed-size buffer (no dynamic strings, no std). */
class Player {
  enum { NameLengthMax = 32 };
  CHA name_[NameLengthMax];  //< The player's name.
  ISC win_count_;            //< Number of rounds won.
  ISC point_count_;          //< Credit/point balance.
  Hand hand_;                //< The current hand of cards.

 public:
  /* Constructor. @param name The player's name (up to 31 chars).
  @param points The starting credit balance. */
  Player(const CHA* name = "You", ISC points = 10);

  /* Virtual destructor. */
  virtual ~Player() {}

  /* Gets the name. */
  const CHA* Name() const { return name_; }

  /* Sets the name (truncated to NameLengthMax-1). */
  void SetName(const CHA* name);

  /* Returns the current hand. */
  Hand& GetHand() { return hand_; }

  /* Returns the point/credit total. */
  ISC PointsCount() const { return point_count_; }

  /* Adds (or subtracts) points. @return the new total. */
  ISC PointsAdd(ISC num_points);

  /* Resets the win count to 0. */
  void ResetWins() { win_count_ = 0; }

  /* Returns the win count. */
  ISC WinCount() const { return win_count_; }

  /* Adds a win. */
  void WinAdd() { ++win_count_; }
};

}  //< namespace CardsWorld
}  //< namespace _
#endif
#endif
