// Copyright AStarship <https://astarship.net>.
//
// Player.h — headless player (TileWorld core, SFML stripped).
//
// PORT-NOTE: the original Player owned a Sword and an AttributeComponent and
// drove an AnimationComponent — all 2nd-pass. The headless Player keeps only
// the hitbox (12, 10, 40x54) and movement (200, 1600, 1000) from the original
// ctor. Render() deleted (headless). No-stdlib.
#pragma once
#include <_ConfigHeader.h>
#include <TVec.h>

#ifndef TILEWORLD_PLAYER
#define TILEWORLD_PLAYER

#include "Entity.h"

class Player : public Entity {
 public:
  Player(_::FPC x, _::FPC y);
  virtual ~Player();

  virtual void Update(const _::FPC& dt);
};

#endif
