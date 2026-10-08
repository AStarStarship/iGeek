// Copyright AStarship <https://astarship.net>.
#include "Enemy.h"

// PORT-NOTE: the original Enemy ctor was empty and Rat did the component
// setup; the headless Enemy carries that setup directly (no Rat subclass in
// the spine, no animation).
TileWorld::Enemy::Enemy(_::FPC x, _::FPC y) {
  CreateHitboxComponent(13.f, 39.f, 30.f, 30.f);
  CreateMovementComponent(50.f, 1600.f, 1000.f);
  SetTextureId(0);
  PositionSet(x, y);
}

TileWorld::Enemy::~Enemy() {}

// PORT-NOTE: the original Rat::Update was movement_->Update(dt); UpdateAttack
// (commented out); UpdateAnimation (removed); hitbox_->Update (no-op). The
// headless Update keeps the movement integration on the shared pos_.
void TileWorld::Enemy::Update(const _::FPC& dt) {
  if (movement_) movement_->Update(dt, pos_);
}

// Simple chase: steer toward the target. Move() integrates velocity by
// acceleration * dir * dt, so the dir magnitude scales the acceleration; we
// normalize the bearing to ~unit length via a Newton–Raphson reciprocal
// square root (no <cmath>: initial guess from a 1/d2 scale, one refinement
// step). Keeps dependency-free while giving sane constant-speed chases.
void TileWorld::Enemy::Chase(const _::TVec2F& target, const _::FPC& dt) {
  if (movement_ == NILP) return;
  _::FPC dx = target.x - pos_.x;
  _::FPC dy = target.y - pos_.y;
  _::FPC d2 = dx * dx + dy * dy;
  if (d2 <= 0.0001f) return;
  // 1/|d|: guess g = 1/d2 (exact when d=1; < 1 for d>1), then one
  // Newton step on f(x)=1/x^2 - 1/d2 gives x' = x*(1.5 - 0.5*d2*x^2).
  _::FPC g = 1.f / d2;
  _::FPC inv = g * (1.5f - 0.5f * d2 * g * g);
  Move(dx * inv, dy * inv, dt);
}
