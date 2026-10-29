// Copyright AStarship <https://astarship.net>.
//
// ComponentMovement.hxx — headless movement (TileWorld core, SFML stripped).
// Matches the SFML-stripped ComponentMovement.h: the component no longer holds
// an sf::Sprite&. It stores velocity + tuning constants; Move() integrates the
// velocity, Update() clamps/decelerates then applies the displacement to a
// passed-in _::TVec2F& position (the entity's pos_). No-stdlib.
#include "ComponentMovement.h"

namespace TileWorld {

MovementComponent::MovementComponent(_::FPC velocity_max, _::FPC acceleration,
                                     _::FPC deceleration)
    : velocity_max_(velocity_max), acceleration_(acceleration),
      deceleration_(deceleration), velocity_(_::TVec2F(0.f, 0.f)) {}

MovementComponent::~MovementComponent() {}

const _::FPC& MovementComponent::VelocityMax() const { return velocity_max_; }

const _::TVec2F& MovementComponent::Velocity() const { return velocity_; }

const _::BOL MovementComponent::GetState(const _::IUM state) const {
  switch (state) {
    case kIdle:
      if (velocity_.x == 0.f && velocity_.y == 0.f) return true;
      break;
    case kMoving:
      if (velocity_.x != 0.f || velocity_.y != 0.f) return true;
      break;
    case kMovingLeft:
      if (velocity_.x < 0.f) return true;
      break;
    case kMovingRight:
      if (velocity_.x > 0.f) return true;
      break;
    case kMovingUp:
      if (velocity_.y < 0.f) return true;
      break;
    case kMovingDown:
      if (velocity_.y > 0.f) return true;
      break;
  }

  return false;
}

void MovementComponent::StopVelocity() {
  velocity_.x = 0.f;
  velocity_.y = 0.f;
}

void MovementComponent::StopVelocityX() { velocity_.x = 0.f; }
void MovementComponent::StopVelocityY() { velocity_.y = 0.f; }

void MovementComponent::Move(const _::FPC dir_x, const _::FPC dir_y,
                             const _::FPC& dt) {
  velocity_.x += acceleration_ * dir_x * dt;
  velocity_.y += acceleration_ * dir_y * dt;
}

void MovementComponent::Update(const _::FPC& dt, _::TVec2F& position) {
  if (velocity_.x > 0.f) {
    if (velocity_.x > velocity_max_) velocity_.x = velocity_max_;
    velocity_.x -= deceleration_ * dt;
    if (velocity_.x < 0.f) velocity_.x = 0.f;
  } else if (velocity_.x < 0.f) {
    if (velocity_.x < -velocity_max_) velocity_.x = -velocity_max_;
    velocity_.x += deceleration_ * dt;
    if (velocity_.x > 0.f) velocity_.x = 0.f;
  }

  if (velocity_.y > 0.f) {
    if (velocity_.y > velocity_max_) velocity_.y = velocity_max_;
    velocity_.y -= deceleration_ * dt;
    if (velocity_.y < 0.f) velocity_.y = 0.f;
  } else if (velocity_.y < 0.f) {
    if (velocity_.y < -velocity_max_) velocity_.y = -velocity_max_;
    velocity_.y += deceleration_ * dt;
    if (velocity_.y > 0.f) velocity_.y = 0.f;
  }

  position += velocity_ * dt;
}

}  //< namespace TileWorld
