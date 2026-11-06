// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekPacWorldPacH
#define _iGeekPacWorldPacH
#include "Character.h"
namespace PacWorld {

/* The headless pac character. Extends Character (movement/collision).
Tracks dead/dying state. No SFML, no stdlib. */
class Pac : public Character {
  BOL is_dead_;
  BOL is_dying_;

 public:
  Pac();

  /* Start the death sequence. */
  void Die();

  /* Reset to a fresh (alive, not-dying) state. */
  void Reset();

  BOL IsDying();
  BOL IsDead();

  void Update(FPC delta) override;
};
}  //< namespace PacWorld
#endif
