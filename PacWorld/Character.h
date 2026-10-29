// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef PACWORLD_CHARACTER
#define PACWORLD_CHARACTER

#include "Maze.h"

namespace PacWorld {

/* A headless character (pac or ghost) that moves on the maze grid.
   Position is an explicit FPC2 member (was sf::Transformable). No SFML,
   no stdlib. The `Update(delta)` method advances position by
   speed * delta along `direction_`, handles wall collisions, and triggers
   `ChangeDirection()` at intersections (overridden by Ghost for AI). */
class Character {
  FPC speed_;                    // pixels per second.
  FPC2 position_;                // current pixel position (was Transformable).
  ISC2 direction_;               // current direction (unit vector in cells).
  ISC2 next_direction_;          // next direction to face.
  ISC2 previous_intersection_;   // last intersection cell we were at.
  BOL available_directions_[4];  // which of the 4 dirs are open at last xsect.
  Maze* maze_;                   // the maze this character is in.

 public:
  Character();

  /* Advance the character by `delta` seconds. */
  virtual void Update(FPC delta);

  void SetDirection(ISC2 direction);
  ISC2 Direction() const;

  void SetSpeed(FPC speed);
  FPC Speed() const;

  void SetPosition(FPC2 position);
  FPC2 Position() const;

  /* The character's collision box in pixel space (FPC4 AABB). */
  FPC4 CollisionBox() const;

  void SetMaze(Maze* maze);

  /* Can the character move in its next direction from the last xsect? */
  BOL WillMove() const;

 protected:
  /* Check if there's a wall in the given direction from the current cell.
     Used by Ghost's AI to pick open directions. */
  BOL IsWallAhead(ISC2 direction) const {
    if (NILP == maze_) return true;
    ISC2 cell = maze_->PixelToCell(Position());
    return maze_->IsWall(cell + direction);
  }

  /* Hook for subclasses to pick a new direction at an intersection.
     Ghost overrides this with its AI; Pac leaves it as a no-op. */
  virtual void ChangeDirection() {}
};
}  // namespace PacWorld
#endif
