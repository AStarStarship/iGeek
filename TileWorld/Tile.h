// Copyright AStarship <https://astarship.net>.
//
// Tile.h — headless tile (TileWorld core, SFML stripped).
//
// SFML port: the tile no longer owns an sf::Sprite (shape_). It stores its
// grid cell + the texture sub-rect it would draw (data only) and computes its
// world bounds from (grid_x, grid_y) * grid_size. Render() deleted
// (headless). No-stdlib.
#pragma once
#include <_ConfigHeader.h>
#include <TVec.h>

#ifndef TILEWORLD_TILE
#define TILEWORLD_TILE

enum TileTypes { kDefault = 0, kDamaging, kDoodad, kSpawnerEnemy };

class Tile {
 protected:
  _::ISC grid_x_;
  _::ISC grid_y_;
  _::FPC grid_size_f_;
  _::TIntRect texture_rect_;  // which atlas cell (data only)
  _::BOL collision_;
  _::ISM type_;
  _::TVec2F position_;  // world position (was the sprite position)

 public:
  Tile();
  Tile(_::ISM type, _::ISC grid_x, _::ISC grid_y, _::FPC grid_size_f_,
       const _::TIntRect& texture_rect, const _::BOL collision);
  virtual ~Tile();

  const _::ISM& GetType() const;
  virtual const _::BOL& GetCollision() const;

  virtual const _::TVec2F& GetPosition() const;
  virtual const _::TFloatRect GlobalBounds() const;
  virtual const _::BOL Intersects(const _::TFloatRect bounds) const;

  virtual void Update() = 0;
};

#endif
