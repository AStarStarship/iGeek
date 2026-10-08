// Copyright AStarship <https://astarship.net>.
#include "Player.h"

// PORT-NOTE: the original ctor created the animation/attribute/skill components
// (2nd-pass) and a Sword; the headless ctor keeps only the hitbox + movement
// with the original numbers.
Player::Player(_::FPC x, _::FPC y) {
  CreateHitboxComponent(12.f, 10.f, 40.f, 54.f);
  CreateMovementComponent(200.f, 1600.f, 1000.f);
  SetTextureId(0);
  PositionSet(x, y);
}

Player::~Player() {}

// PORT-NOTE: the original Update was movement_->Update(dt); UpdateAttack (mouse
// — no GUI in headless); UpdateAnimation (removed); hitbox_->Update (no-op);
// sword_.Update (removed). The headless Update keeps the movement integration.
void Player::Update(const _::FPC& dt) {
  if (movement_) movement_->Update(dt, pos_);
}
