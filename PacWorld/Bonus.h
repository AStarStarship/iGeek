// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef PACWORLD_BONUS
#define PACWORLD_BONUS

#include "Maze.h"  // brings in _Config.h (ISC/BOL) + TVec.h (ISC4)

namespace PacWorld {

/* A headless fruit bonus (banana/apple/cherry). Holds the current fruit type
   and its texture sub-rect (ISC4). The actual drawing is a display concern
   (CImage later) — Draw() is a no-op. No SFML, no stdlib. */
class Bonus {
  ISC fruit_;        // kBanana/kApple/kCherry.
  ISC4 texture_rect_;  // the texture sub-rect for the current fruit.

 public:
  enum Fruit {
    Banana = 0,
    Apple,
    Cherry,
  };

  Bonus();

  /* Set the current fruit (updates the texture sub-rect). */
  void SetFruit(Fruit fruit);

  Fruit FruitType() const { return static_cast<Fruit>(fruit_); }

  /* The current fruit's texture sub-rect. */
  ISC4 TextureRect() const { return texture_rect_; }

  /* No-op for headless — the draw is a display concern. */
  void Draw() {}
};

}  // namespace PacWorld
#endif
