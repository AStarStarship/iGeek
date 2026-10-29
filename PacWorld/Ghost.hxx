// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_GHOST_HXX
#define IGEEK_GHOST_HXX
#include "Ghost.h"

using namespace PacWorld;

Ghost::Ghost(const Pac* pac)
    : is_weak_(false), weakness_duration_(0.f), pac_(pac) {}

Ghost::Ghost()
    : is_weak_(false), weakness_duration_(0.f), pac_(NILP) {}

void Ghost::SetWeak(FPC duration) {
  is_weak_ = true;
  weakness_duration_ = duration;
}

BOL Ghost::IsWeak() const { return is_weak_; }

void Ghost::Update(FPC delta) {
  if (is_weak_) {
    weakness_duration_ -= delta;
    if (weakness_duration_ <= 0.f) {
      is_weak_ = false;
    }
  }
  // (animator updates are render concerns — stubbed for headless)
  Character::Update(delta);
}

/* AI: pick the open direction that best points toward pac. Uses dot
   product (no atan2, no std::map). For each of the 4 directions, compute
   the dot product with the (normalized-ish) direction to pac; choose the
   open direction with the highest dot. Ties break toward the current
   direction (prefer continuing). */
void Ghost::ChangeDirection() {
  static const ISC2 kDirections[4] = {
      ISC2(1, 0), ISC2(0, 1), ISC2(-1, 0), ISC2(0, -1)};

  // Direction to pac (in cell space).
  FPC2 to_pac = pac_->Position();
  FPC2 my_pos = Position();
  FPC dx = to_pac.x - my_pos.x;
  FPC dy = to_pac.y - my_pos.y;

  ISC best_dir = -1;
  FPC best_dot = -999999.f;

  for (ISC i = 0; i < 4; ++i) {
    // Skip walls (can't go where there's a wall).
    if (IsWallAhead(kDirections[i])) continue;
    // Dot product of the candidate direction with the direction to pac.
    FPC dot = FPC(kDirections[i].x) * dx + FPC(kDirections[i].y) * dy;
    // Prefer continuing in the current direction on ties.
    if (i == 0 && kDirections[i] == Direction()) dot += 0.001f;
    if (dot > best_dot) {
      best_dot = dot;
      best_dir = i;
    }
  }

  if (best_dir >= 0) {
    SetDirection(kDirections[best_dir]);
  }
}

#endif  // IGEEK_GHOST_HXX
