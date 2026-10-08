// Copyright AStarship <https://astarship.net>.
//
// Enemy.h — headless enemy (TileWorld core, SFML stripped).
//
// PORT-NOTE: the original declared pure-virtual InitVariables()/InitAnimation()
// and UpdateAnimation(), all tied to the removed animation component; the
// animation is 2nd-pass, so the headless Enemy is a concrete Entity subclass
// (a "rat") with chase movement. Render() deleted (headless). No-stdlib.
#pragma once
#include <_ConfigHeader.h>
#include <TVec.h>

#ifndef TILEWORLD_ENEMY
#define TILEWORLD_ENEMY
#include "Entity.h"

namespace TileWorld {

class Enemy : public Entity {
 public:
  // Hitbox offset (13, 39) 30x30 and movement tuning (50, 1600, 1000) from
  // the original Rat ctor.
  Enemy(_::FPC x, _::FPC y);
  virtual ~Enemy();

  virtual void Update(const _::FPC& dt);

  // Steer toward the target position (unit-ish direction into Move()).
  void Chase(const _::TVec2F& target, const _::FPC& dt);
};
}  //< namespace TileWorld
#endif
