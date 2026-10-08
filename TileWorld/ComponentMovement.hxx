// Copyright AStarship <https://astarship.net>.
#include "ComponentMovement.h"

// PORT-NOTE: the original stored an sf::Sprite& and moved the sprite at the
// end of Update(). The sprite is gone; Update() takes the entity position by
// reference and applies the displacement itself (same math).
TileWorld::MovementComponent::MovementComponent(_::FPC velocity_max,
                                                _::FPC acceleration,
                                                _::FPC deceleration)
    : velocity_max_(velocity_max),
      acceleration_(acceleration),
      deceleration_(deceleration),
      velocity_(0.f, 0.f) {}

TileWorld::MovementComponent::~MovementComponent() {}

const _::FPC& TileWorld::MovementComponent::VelocityMax() const { return velocity_max_; }

const _::TVec2F& TileWorld::MovementComponent::Velocity() const { return velocity_; }

const BOL TileWorld::MovementComponent::GetState(const _::IUM state) const {
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

void TileWorld::MovementComponent::StopVelocity() {
  velocity_.x = 0.f;
  velocity_.y = 0.f;
}

void TileWorld::MovementComponent::StopVelocityX() { velocity_.x = 0.f; }

void TileWorld::MovementComponent::StopVelocityY() { velocity_.y = 0.f; }

// PORT-NOTE: the original referenced the bare identifiers `acceleration` and
// `deceleration`/`velocity_max` (its own members are _-suffixed) — it would
// not compile as written. Minimal fix: use the _-suffixed members; the math
// is byte-identical.
void TileWorld::MovementComponent::Move(const _::FPC dir_x, const _::FPC dir_y,
                                        const _::FPC& dt) {
  velocity_.x += acceleration_ * dir_x * dt;
  velocity_.y += acceleration_ * dir_y * dt;
}

// PORT-NOTE: same member-suffix fix as Move(); the final `sprite.move(velocity_ * dt)`
// becomes `position += velocity_ * dt` (the sprite no longer exists). The
// clamp/deceleration math is preserved exactly.
void TileWorld::MovementComponent::Update(const _::FPC& dt, _::TVec2F& position) {
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
