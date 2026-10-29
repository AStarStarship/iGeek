// Copyright AStarship <https://astarship.net>.
//
// ComponentHitbox.hxx — headless hitbox (TileWorld core, SFML stripped).
// Matches the SFML-stripped ComponentHitbox.h: the hitbox stores only its
// offset + size and computes world bounds from a passed-in entity position.
// (Replaces the old sf::Sprite-wrapping implementation, which no longer
// compiles without SFML.)
#include "ComponentHitbox.h"

namespace TileWorld {

HitboxComponent::HitboxComponent(_::FPC offset_x, _::FPC offset_y, _::FPC width,
                                 _::FPC height)
    : offset_x(offset_x), offset_y(offset_y), width_(width), height_(height),
      next_position_(_::TFloatRect(0.f, 0.f, 0.f, 0.f)) {}

HitboxComponent::~HitboxComponent() {}

const _::TVec2F HitboxComponent::Position(const _::TVec2F& entity_pos) const {
  return _::TVec2F(entity_pos.x + offset_x, entity_pos.y + offset_y);
}

const _::TFloatRect HitboxComponent::GlobalBounds(const _::TVec2F& entity_pos) const {
  return _::TFloatRect(entity_pos.x + offset_x, entity_pos.y + offset_y, width_, height_);
}

const _::TFloatRect& HitboxComponent::NextPosition(const _::TVec2F& entity_pos,
                                                   const _::TVec2F& velocity) const {
  next_position_ = _::TFloatRect(entity_pos.x + offset_x + velocity.x,
                                 entity_pos.y + offset_y + velocity.y, width_, height_);
  return next_position_;
}

void HitboxComponent::PositionSet(const _::FPC x, const _::FPC y, _::TVec2F* entity_pos) {
  if (entity_pos != nullptr) {
    entity_pos->x = x - offset_x;
    entity_pos->y = y - offset_y;
  }
}

bool HitboxComponent::Intersects(const _::TVec2F& entity_pos, const _::TFloatRect& frect) const {
  return GlobalBounds(entity_pos).Intersects(frect);
}

}  //< namespace TileWorld
