// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekTileWorldComponentHitboxHXX
#define _iGeekTileWorldComponentHitboxHXX
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

const _::FPC2 HitboxComponent::Position(const _::FPC2& entity_pos) {
  return _::FPC2(entity_pos.x + offset_x, entity_pos.y + offset_y);
}

const _::TFloatRect HitboxComponent::GlobalBounds(const _::FPC2& entity_pos) {
  return _::TFloatRect(entity_pos.x + offset_x, entity_pos.y + offset_y, width_, height_);
}

const _::TFloatRect& HitboxComponent::NextPosition(const _::FPC2& entity_pos,
                                                   const _::FPC2& velocity) {
  next_position_ = _::TFloatRect(entity_pos.x + offset_x + velocity.x,
                                 entity_pos.y + offset_y + velocity.y, width_, height_);
  return next_position_;
}

void HitboxComponent::PositionSet(const _::FPC x, const _::FPC y, _::FPC2* entity_pos) {
  if (entity_pos != nullptr) {
    entity_pos->x = x - offset_x;
    entity_pos->y = y - offset_y;
  }
}

bool HitboxComponent::Intersects(const _::FPC2& entity_pos, const _::TFloatRect& frect) {
  return GlobalBounds(entity_pos).Intersects(frect);
}

}  //< namespace TileWorld
#endif  //< _iGeekTileWorldComponentHitboxHXX
