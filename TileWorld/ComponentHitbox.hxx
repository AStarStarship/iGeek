// Copyright AStarship <https://astarship.net>.
#include "ComponentHitbox.h"

// PORT-NOTE: the original stored an sf::Sprite& and an sf::RectangleShape and
// initialized the shape position/size/fill-color from the sprite. The sprite
// and shape are gone; only the offsets and size survive.
TileWorld::HitboxComponent::HitboxComponent(_::FPC offset_x, _::FPC offset_y,
                                            _::FPC width, _::FPC height)
    : offset_x(offset_x), offset_y(offset_y), width_(width), height_(height),
      next_position_(0.f, 0.f, width, height) {}

TileWorld::HitboxComponent::~HitboxComponent() {}

const _::TVec2F TileWorld::HitboxComponent::Position(const _::TVec2F& entity_pos) const {
  return _::TVec2F(entity_pos.x + offset_x, entity_pos.y + offset_y);
}

const _::TFloatRect TileWorld::HitboxComponent::GlobalBounds(
    const _::TVec2F& entity_pos) const {
  return _::TFloatRect(entity_pos.x + offset_x, entity_pos.y + offset_y, width_, height_);
}

const _::TFloatRect& TileWorld::HitboxComponent::NextPosition(
    const _::TVec2F& entity_pos, const _::TVec2F& velocity) const {
  next_position_.x = entity_pos.x + offset_x + velocity.x;
  next_position_.y = entity_pos.y + offset_y + velocity.y;

  return next_position_;
}

// PORT-NOTE: the original wrote the hitbox shape AND back-set the sprite by
// the offset. The sprite is gone; PositionSet on a hitbox is a no-op because
// the hitbox is derived from the entity pos (the entity's own PositionSet
// writes the shared store). The pointer form is kept so Entity can apply the
// position to its own pos_ if an offset model is ever needed.
void TileWorld::HitboxComponent::PositionSet(const _::FPC x, const _::FPC y,
                                             _::TVec2F* entity_pos) {
  (void)x;
  (void)y;
  (void)entity_pos;
}

bool TileWorld::HitboxComponent::Intersects(const _::TVec2F& entity_pos,
                                            const _::TFloatRect& frect) const {
  return GlobalBounds(entity_pos).Intersects(frect);
}
