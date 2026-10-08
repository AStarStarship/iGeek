// Copyright AStarship <https://astarship.net>.
#include "Entity.h"

// PORT-NOTE: the original InitVariables() nulled animation_/attribute_/skill_
// (members that don't exist in this port — 2nd pass). Minimal change: null
// only the two components that exist.
void Entity::InitVariables() {
  hitbox_component_ = NILP;
  movement_ = NILP;
}

Entity::Entity() : pos_(0.f, 0.f), size_(0.f, 0.f), texture_id(0), occlude_(false) {
  InitVariables();
}

Entity::~Entity() {
  delete hitbox_component_;
  delete movement_;
}

void Entity::SetTextureId(_::IUD texture_id) { this->texture_id = texture_id; }

void Entity::SetOcclude(_::BOL on) { occlude_ = on; }

_::BOL Entity::Occlude() const { return occlude_; }

// PORT-NOTE: the original passed an sf::Sprite& to the component ctor; the
// sprite no longer exists, so the component is built from the offsets alone
// (it derives world bounds from the entity pos at query time).
void Entity::CreateHitboxComponent(_::FPC offset_x, _::FPC offset_y, _::FPC width,
                                   _::FPC height) {
  hitbox_component_ = new TileWorld::HitboxComponent(offset_x, offset_y, width, height);
}

// PORT-NOTE: as above — no sprite argument; the component operates on a
// passed-in position reference.
void Entity::CreateMovementComponent(const _::FPC velocity_max,
                                     const _::FPC acceleration,
                                     const _::FPC deceleration) {
  movement_ = new TileWorld::MovementComponent(velocity_max, acceleration, deceleration);
}

const _::TVec2F& Entity::Position() const {
  // The original returned the hitbox position when a hitbox existed (the
  // hitbox and sprite shared one store, so this was identical to the sprite
  // position). Here the entity pos IS the shared store; the hitbox is an
  // offset derived from it.
  return pos_;
}

const _::TVec2F Entity::Center() const {
  if (hitbox_component_) {
    // The hitbox world bounds start at pos_ + (offset_x, offset_y); the
    // center is bounds.left/top + half size.
    _::TFloatRect b = hitbox_component_->GlobalBounds(pos_);
    return b.Center();
  }
  return pos_ + _::TVec2F(size_.x / 2.f, size_.y / 2.f);
}

const _::TVec2I Entity::GridPosition(const _::FPC grid_size_f_) const {
  // PORT-NOTE: the original computed grid position from the hitbox position
  // (= sprite position) with integer division by the grid size; identical math
  // on the shared store.
  return _::TVec2I(static_cast<_::ISC>(pos_.x) / grid_size_f_,
                   static_cast<_::ISC>(pos_.y) / grid_size_f_);
}

const _::TFloatRect Entity::GlobalBounds() const {
  if (hitbox_component_) return hitbox_component_->GlobalBounds(pos_);
  return _::TFloatRect(pos_.x, pos_.y, size_.x, size_.y);
}

const _::TFloatRect Entity::NextPositionBounds(const _::FPC& dt) const {
  if (hitbox_component_ && movement_)
    return hitbox_component_->NextPosition(pos_, movement_->Velocity() * dt);

  return _::TFloatRect(-1.f, -1.f, -1.f, -1.f);
}

// Functions
void Entity::PositionSet(const _::FPC x, const _::FPC y) {
  // PORT-NOTE: the original wrote the sprite position (and, via the hitbox,
  // back-adjusted the sprite by the offset — a no-op quirk because both shared
  // one store). Here we write the shared store directly.
  pos_.x = x;
  pos_.y = y;
}

// PORT-NOTE: the original also gained skill EXP here; the skill component is
// out of scope this pass, so only the movement integration remains.
void Entity::Move(const _::FPC dir_x, const _::FPC dir_y, const _::FPC& dt) {
  if (movement_) movement_->Move(dir_x, dir_y, dt);  // Sets velocity
}

void Entity::StopVelocity() {
  if (movement_) movement_->StopVelocity();
}

void Entity::StopVelocityX() {
  if (movement_) movement_->StopVelocityX();
}

void Entity::StopVelocityY() {
  if (movement_) movement_->StopVelocityY();
}
