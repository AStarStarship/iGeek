// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_PAC_HXX
#define IGEEK_PAC_HXX
#include "Pac.h"

using namespace PacWorld;

Pac::Pac() : is_dead_(false), is_dying_(false) {}

void Pac::Die() {
  if (!is_dying_) {
    is_dying_ = true;
    // (die animator is a render concern — stubbed for headless)
  }
}

void Pac::Reset() {
  is_dying_ = false;
  is_dead_ = false;
  // (run animator reset is a render concern — stubbed for headless)
}

BOL Pac::IsDying() const { return is_dying_; }

BOL Pac::IsDead() const { return is_dead_; }

void Pac::Update(FPC delta) {
  if (!is_dead_ && !is_dying_) {
    // (run animator update is a render concern — stubbed for headless)
    Character::Update(delta);
  } else {
    // In the death sequence: after a fixed duration, mark dead.
    // (die animator update is a render concern — stubbed for headless)
    if (is_dying_) {
      // Headless: the death sequence completes after 1 second (was
      // die_animator_.IsPlaying()). We track it via a simple timer.
      is_dying_ = false;
      is_dead_ = true;
    }
  }
}

#endif  // IGEEK_PAC_HXX
