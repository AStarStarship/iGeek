// Copyright AStarship <https://astarship.net>.
//
// ComponentMovement.h — headless movement (TileWorld core, SFML stripped).
//
// SFML port: the component no longer holds an sf::Sprite&. It stores velocity
// and tuning constants; Move() integrates the velocity, and Update() clamps /
// decelerates then applies the displacement to a passed-in _::FPC2&
// position (the entity's pos_). Render() deleted (headless). No-stdlib.
#pragma once
#include <_Config.h>
#include <Homotuples.h>

#ifndef TILEWORLD_MOVEMENTCOMPONENT
#define TILEWORLD_MOVEMENTCOMPONENT
namespace TileWorld {

enum movement_states {
  kIdle = 0,
  kMoving,
  kMovingLeft,
  kMovingRight,
  kMovingUp,
  kMovingDown
};

class MovementComponent {
  _::FPC velocity_max_;
  _::FPC acceleration_;
  _::FPC deceleration_;
  _::FPC2 velocity_;

 public:
  MovementComponent(_::FPC velocity_max, _::FPC acceleration, _::FPC deceleration);
  virtual ~MovementComponent();

  const _::FPC& VelocityMax();
  const _::FPC2& Velocity();

  const BOL GetState(const _::IUM state);
  void StopVelocity();
  void StopVelocityX();
  void StopVelocityY();

  void Move(const _::FPC x, const _::FPC y, const _::FPC& dt);
  void Update(const _::FPC& dt, _::FPC2& position);
};
}  //< namespace TileWorld
#endif
