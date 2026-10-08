// Copyright AStarship <https://astarship.net>.
//
// TileMap.h — headless tile map (TileWorld core, SFML stripped).
//
// SFML port: the tile grid is a flat IUD-indexed array of Tile* (no _::Array
// nesting, no sf::Texture tile sheet, no culling render stack). The NEW piece
// is the headless spatial awareness: a _::TOcclusionMap rebuilt and
// recomputed on every Update() (layer A = collision tiles, layer B = entities
// with occlude_=true, observers = player + enemies). SaveToFile/LoadFromFile
// are DELETED (file I/O is a 2nd-pass concern). Render* deleted (headless).
// No-stdlib.
#pragma once
#include <_ConfigHeader.h>
#include <TVec.h>
#include <TOcclusion.h>

#ifndef TILEWORLD_TILEMAP
#define TILEWORLD_TILEMAP

#include "Entity.h"
#include "TileRegular.h"
#include "TileSpawner.h"

class TileMap {
  _::FPC grid_size_f_;
  _::ISC width_;
  _::ISC height_;
  _::TVec2I max_size_world_grid_;
  _::TVec2F max_size_world_f_;
  Tile** map_;  // [width_ * height_] layer 0; nullptr = empty cell
  _::TOcclusionMap occlusion_;  // headless spatial awareness (layer A/B/C)

  // Observer slots for the occlusion map.
  _::ISC obs_player_;
  _::ISC obs_enemy_[7];  // up to 7 enemies (player + 7 <= TOcclusionObsMax=8)
  _::ISC obs_enemy_count_;

 public:
  /* Build an empty w x h grid of the given cell size (built in code, no
     texture sheet, no file load). */
  TileMap(_::FPC grid_size, _::ISC width, _::ISC height);
  virtual ~TileMap();

  const _::BOL TileEmpty(const _::ISC x, const _::ISC y) const;
  const _::ISC LayerSize(const _::ISC x, const _::ISC y) const;
  const _::TVec2I& SizeGridMax() const;
  const _::TVec2F& SizeMaxF() const;

  void AddTile(const _::ISC x, const _::ISC y, const _::TIntRect& texture_rect,
               const _::BOL collision, const _::ISM type);
  void AddSpawnerTile(const _::ISC x, const _::ISC y,
                      const _::TIntRect& texture_rect, const _::ISC enemy_type,
                      const _::ISC enemy_amount, const _::ISC enemy_tts,
                      const _::FPC enemy_md);
  void RemoveTile(const _::ISC x, const _::ISC y, const _::ISC type = -1);
  const _::BOL CheckType(const _::ISC x, const _::ISC y, const _::ISM type) const;

  /* One headless tick: world-bounds clamp, tile collision, tile updates
     (spawners), then rebuild + recompute the occlusion map with the player
     as observer 0 and the enemies as observers 1..n. */
  void Update(Entity* player, const _::FPC& dt, Entity** enemies,
              _::ISC enemy_count);

  /* --- Occlusion queries (O(1) after Update's Recompute). --- */
  const _::TOcclusionMap& Occlusion() const { return occlusion_; }
  /* Can the player (observer 0) see cell (cx,cy)? */
  _::BOL PlayerSeesCell(const _::ISC cx, const _::ISC cy) const;
  /* Can the player see enemy i (i < enemy_count)? Enemy cell = its grid cell. */
  _::BOL PlayerSeesEnemy(const Entity* enemy, _::ISC i) const;
  /* Can the player see an arbitrary entity? */
  _::BOL PlayerSees(const Entity* e) const;

 private:
  void Clear();
  /* Rebuild the occluder grid (layers A+B) and recompute visibility with
     player as observer 0, enemies 1..n. */
  void RebuildOcclusion(Entity* player, Entity** enemies, _::ISC enemy_count);
  /* Mark every grid cell covered by an entity's bounds as occluded (layer B). */
  void MarkEntityOccluder(const Entity* e);
};

#endif
