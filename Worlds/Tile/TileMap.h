// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekTileMapH
#define _iGeekTileMapH
#include <_Config.h>
#include <Homotuples.h>
#include <Occlusion.h>
#include "Entity.h"
#include "TileRegular.h"
#include "TileSpawner.h"

class TileMap {
  _::FPC grid_size_f_;
  _::ISC width_;
  _::ISC height_;
  _::ISC2 max_size_world_grid_;
  _::FPC2 max_size_world_f_;
  Tile** map_;  // [width_ * height_] layer 0; nullptr = empty cell
  _::OcclusionMap occlusion_;  // headless spatial awareness (layer A/B/C)

  // Observer slots for the occlusion map.
  _::ISC observation_player_;
  _::ISC observation_enemy_[7];  // up to 7 enemies (player + 7 <= TOcclusionObsMax=8)
  _::ISC observation_enemy_count_;

 public:
  /* Build an empty w x h grid of the given cell size (built in code, no
     texture sheet, no file load). */
  TileMap(_::FPC grid_size, _::ISC width, _::ISC height);
  virtual ~TileMap();

  const _::BOL TileEmpty(const _::ISC x, const _::ISC y);
  const _::ISC LayerSize(const _::ISC x, const _::ISC y);
  const _::ISC2& SizeGridMax();
  const _::FPC2& SizeMaxF();

  /** Cell width in world units; clients can choose metres for simulation. */
  _::FPC GridSize() { return grid_size_f_; }
  /** Read-only tile data. _NilP denotes an empty cell or invalid coordinates. */
  Tile* TileAt(_::ISC x, _::ISC y) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return _NilP;
    return map_[_::IUD(y) * _::IUD(width_) + _::IUD(x)];
  }
  /** Collision flag, independent of type; outside the map is solid. */
  _::BOL CellBlocked(_::ISC x, _::ISC y) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return true;
    Tile* tile = TileAt(x, y);
    return tile != _NilP && tile->GetCollision();
  }

  void AddTile(const _::ISC x, const _::ISC y, const _::TIntRect& texture_rect,
               const _::BOL collision, const _::ISM type);
  void AddSpawnerTile(const _::ISC x, const _::ISC y,
                      const _::TIntRect& texture_rect, const _::ISC enemy_type,
                      const _::ISC enemy_amount, const _::ISC enemy_tts,
                      const _::FPC enemy_md);
  void RemoveTile(const _::ISC x, const _::ISC y, const _::ISC type = -1);
  const _::BOL CheckType(const _::ISC x, const _::ISC y, const _::ISM type);

  /* One headless tick: world-bounds clamp, tile collision, tile updates
     (spawners), then rebuild + recompute the occlusion map with the player
     as observer 0 and the enemies as observers 1..n. */
  void Update(Entity* player, const _::FPC& dt, Entity** enemies,
              _::ISC enemy_count);

  /* --- Occlusion queries (O(1) after Update's Recompute). --- */
  const _::OcclusionMap& Occlusion() { return occlusion_; }
  /* Can the player (observer 0) see cell (cx,cy)? */
  _::BOL PlayerSeesCell(const _::ISC cx, const _::ISC cy);
  /* Can the player see enemy i (i < enemy_count)? Enemy cell = its grid cell. */
  _::BOL PlayerSeesEnemy(Entity* enemy, _::ISC i);
  /* Can the player see an arbitrary entity? */
  _::BOL PlayerSees(Entity* e);

 private:
  void Clear();
  /* Rebuild the occluder grid (layers A+B) and recompute visibility with
     player as observer 0, enemies 1..n. */
  void RebuildOcclusion(Entity* player, Entity** enemies, _::ISC enemy_count);
  /* Mark every grid cell covered by an entity's bounds as occluded (layer B). */
  void MarkEntityOccluder(Entity* e);
};

#endif
