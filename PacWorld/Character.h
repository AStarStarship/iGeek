// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef PACWORLD_CHARACTER
#define PACWORLD_CHARACTER
#include <SFML/Graphics.hpp>
#include "Maze.h"
//
#include <array>
namespace PacWorld {

class Character : public sf::Drawable, public sf::Transformable {
  float speed_;             //< The spped in ???.
  sf::Vector2i direction_,  //< The current direction.
      next_direction_;      //< The next direction to face.
  Maze* maze_;              //< The maze this is in.

  sf::Vector2i previous_intersection_;
  std::array<bool, 4> available_directions_;

  public:
  Character();

  virtual void Update(sf::Time delta);

  void DirectionSet(sf::Vector2i direction);

  sf::Vector2i Direction() const;

  void SpeedSet(float speed);

  float Speed() const;

  sf::FloatRect CollisionBox() const;

  void MazeSet(Maze* maze);

  bool WillMove();

 protected:
  virtual void ChangeDirection() {}
};
}  //< namespace PacWorld
#endif
