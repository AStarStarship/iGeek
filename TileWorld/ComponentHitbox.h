// Copyright AStarship <https://astarship.net>.
//
// ComponentHitbox.h — headless hitbox (TileWorld core, SFML stripped).
//
// SFML port: the hitbox no longer wraps an sf::Sprite or owns an
// sf::RectangleShape. It is pure bounds math: it stores only the offset
// (offset_x, offset_y) and size (width, height) and computes its world bounds
// from a passed-in entity position (_::TVec2F& or by value). Render() deleted
// (headless). No-stdlib.
#pragma once
#include <_ConfigHeader.h>
#include <TVec.h>

#ifndef TILEWORLD_HITBOXCOMPONENT_DECL
#define TILEWORLD_HITBOXCOMPONENT_DECL
namespace TileWorld {

class HitboxComponent {
  _::FPC offset_x;
  _::FPC offset_y;
  _::FPC width_;
  _::FPC height_;
  _::TFloatRect next_position_;

 public:
  HitboxComponent(_::FPC offset_x, _::FPC offset_y, _::FPC width, _::FPC height);
  virtual ~HitboxComponent();

  // The hitbox's world position given the entity's current position.
  const _::TVec2F Position(const _::TVec2F& entity_pos) const;
  const _::TFloatRect GlobalBounds(const _::TVec2F& entity_pos) const;
  // Bounds after moving by velocity (for swept collision).
  const _::TFloatRect& NextPosition(const _::TVec2F& entity_pos,
                                    const _::TVec2F& velocity) const;

  void PositionSet(const _::FPC x, const _::FPC y, _::TVec2F* entity_pos);

  bool Intersects(const _::TVec2F& entity_pos, const _::TFloatRect& frect) const;
};
}  //< namespace TileWorld
#endif
