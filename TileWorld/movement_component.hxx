// Copyright AStarship <https://astarship.net>.
#include "stdafx.h"
//
#include "movement_component.h"

MovementComponent::MovementComponent(sf::Sprite& sprite, float velocity_max,
                                     float acceleration, float deceleration)
    : sprite_(sprite),
      velocity_max_(velocity_max),
      acceleration_(acceleration),
      deceleration_(deceleration) {}

MovementComponent::~MovementComponent() {}

const float& MovementComponent::VelocityMax() const { return velocity_max_; }

const sf::Vector2f& MovementComponent::Velocity() const { return velocity_; }

const bool MovementComponent::GetState(const short unsigned state) const {
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

void MovementComponent::StopVelocityX() {
  /* Resets the velocity_ x to 0.*/

  velocity_.x = 0.f;
}

void MovementComponent::StopVelocityY() { velocity_.y = 0.f; }

void MovementComponent::Move(const float dir_x, const float dir_y,
                             const float& dt) {
  velocity_.x += acceleration * dir_x * dt;
  velocity_.y += acceleration * dir_y * dt;
}

void MovementComponent::Update(const float& dt) {
  if (velocity_.x > 0.f) {
    if (velocity_.x > velocity_max) velocity_.x = velocity_max;

    velocity_.x -= deceleration * dt;
    if (velocity_.x < 0.f) velocity_.x = 0.f;
  } else if (velocity_.x < 0.f) {
    if (velocity_.x < -velocity_max) velocity_.x = -velocity_max;

    velocity_.x += deceleration * dt;
    if (velocity_.x > 0.f) velocity_.x = 0.f;
  }

  if (velocity_.y > 0.f) {
    if (velocity_.y > velocity_max) velocity_.y = velocity_max;

    velocity_.y -= deceleration * dt;
    if (velocity_.y < 0.f) velocity_.y = 0.f;
  } else if (velocity_.y < 0.f) {
    if (velocity_.y < -velocity_max) velocity_.y = -velocity_max;

    velocity_.y += deceleration * dt;
    if (velocity_.y > 0.f) velocity_.y = 0.f;
  }

  sprite.move(velocity_ * dt);
}
