// Copyright AStarship <https://astarship.net>.
#include "Tile.h"

// PORT-NOTE: the original cached its position on an sf::Sprite (shape_). With
// no sprite, the position is a plain member computed in the ctor — identical
// value, same semantics.
Tile::Tile()
    : grid_x_(0), grid_y_(0), grid_size_f_(0.f), texture_rect_(), position_(0.f, 0.f),
      collision_(false), type_(0) {}

// PORT-NOTE: the original set the sprite position + texture + texture rect;
// the texture/texture-rect become data-only fields.
Tile::Tile(_::ISM type, _::ISC grid_x, _::ISC grid_y, _::FPC grid_size_f_,
           const _::TIntRect& texture_rect, const _::BOL collision)
    : grid_x_(grid_x),
      grid_y_(grid_y),
      grid_size_f_(grid_size_f_),
      texture_rect_(texture_rect),
      position_(static_cast<_::FPC>(grid_x) * grid_size_f_,
                static_cast<_::FPC>(grid_y) * grid_size_f_),
      collision_(collision),
      type_(type) {}

Tile::~Tile() {}

const _::ISM& Tile::GetType() const { return type_; }

const _::BOL& Tile::GetCollision() const { return collision_; }

const _::TVec2F& Tile::GetPosition() const { return position_; }

const _::TFloatRect Tile::GlobalBounds() const {
  return _::TFloatRect(position_.x, position_.y, grid_size_f_, grid_size_f_);
}

const _::BOL Tile::Intersects(const _::TFloatRect bounds) const {
  return GlobalBounds().Intersects(bounds);
}
