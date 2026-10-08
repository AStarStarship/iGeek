// Copyright AStarship <https://astarship.net>.
//
// TileSpawner.h — headless enemy-spawner tile (TileWorld core, SFML stripped).
// Render() deleted (headless). No-stdlib.
#pragma once
#include <_ConfigHeader.h>
#include <TVec.h>
//
#include "Tile.h"

#ifndef TILEWORLD_ENEMYSPAWNERTILE_DECL
#define TILEWORLD_ENEMYSPAWNERTILE_DECL

class TileSpawner : public Tile {
  _::ISC enemy_type_;
  _::ISC enemy_amount_;
  _::ISC enemy_time_to_spawn_;
  _::FPC enemy_max_distance_;
  _::BOL spawned_;

 public:
  TileSpawner(_::ISC grid_x, _::ISC grid_y, _::FPC grid_size_f_,
              const _::TIntRect& texture_rect, _::ISC enemy_type,
              _::ISC enemy_amount, _::ISC enemy_time_to_spawn,
              _::FPC enemy_max_distance);
  virtual ~TileSpawner();

  const _::ISC& GetEnemyType() const;
  const _::BOL& GetSpawned() const;

  // Modifiers
  void SetSpawned(const _::BOL spawned);

  virtual void Update();
};
#endif
