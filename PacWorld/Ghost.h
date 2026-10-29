// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef PACWORLD_GHOST
#define PACWORLD_GHOST

#include "Character.h"
#include "Pac.h"

namespace PacWorld {

/* A headless ghost that chases pac. Extends Character (movement/collision).
   The AI (ChangeDirection) picks the open direction that best points toward
   pac, using a no-stdlib angle comparison (dot product, no atan2). No SFML,
   no stdlib. */
class Ghost : public Character {
  BOL is_weak_;
  FPC weakness_duration_;  // seconds remaining in weak state.
  const Pac* pac_;         // the pac this ghost chases.

 public:
  enum {
    kStateStrong = 0,
    kStateWeak,
  };

  Ghost(const Pac* pac);
  Ghost();  // default ctor (for fixed arrays like StatePlaying::ghosts_[8]).

  /* Set the pac this ghost chases (call after default construction). */
  void SetPac(const Pac* pac) { pac_ = pac; }

  /* Set the ghost to weak (frightened) for `duration` seconds. */
  void SetWeak(FPC duration);
  BOL IsWeak() const;

  void Update(FPC delta) override;

  ISC State() const { return is_weak_ ? kStateWeak : kStateStrong; }

 protected:
  /* AI: pick the open direction that best points toward pac. */
  void ChangeDirection() override;
};
}  // namespace PacWorld
#endif
