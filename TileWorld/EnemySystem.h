// Copyright AStarship <https://astarship.net>.
//
// EnemySystem.h — headless enemy roster (TileWorld core, SFML stripped).
//
// PORT-NOTE: the original EnemySystem held a _::ADic<AString, sf::Texture>
// (texture dict — gone) and pushed new Rat* into a shared _::Array<Enemy*>.
// The headless version owns a fixed-capacity array of Enemy* and a CreateEnemy
// factory (only the RAT type exists in the spine). No-stdlib.
#pragma once
#include <_ConfigHeader.h>
#include <TVec.h>

#ifndef TILEWORLD_ENEMYSYSTEM_DECL
#define TILEWORLD_ENEMYSYSTEM_DECL
#include "Enemy.h"

enum EnemyTypes { RAT = 0 };

namespace TileWorld {

class EnemySystem {
  _::ISC capacity_;
  _::ISC count_;
  TileWorld::Enemy** enemies_;

 public:
  EnemySystem(_::ISC capacity);
  virtual ~EnemySystem();

  // Create an enemy of the given type at (xPos, yPos); returns its index or
  // -1 (unknown type / full).
  _::ISC CreateEnemy(const _::ISM type, const _::FPC xPos, const _::FPC yPos);
  _::ISC Count() const { return count_; }
  TileWorld::Enemy* Get(_::ISC i) const {
    return (i >= 0 && i < count_) ? enemies_[i] : NILP;
  }

  void Update(const _::FPC& dt);
};
}  //< namespace TileWorld
#endif
