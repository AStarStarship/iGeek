// Copyright AStarship <https://astarship.net>.
//
// TileSpawner.hxx — headless enemy-spawner tile (TileWorld core, SFML
// stripped). Matches the SFML-stripped TileSpawner.h: the texture sub-rect is
// data-only; Render() deleted (headless). No-stdlib.
#include "TileSpawner.h"

TileSpawner::TileSpawner(_::ISC grid_x, _::ISC grid_y, _::FPC grid_size_f_,
                         const _::TIntRect& texture_rect, _::ISC enemy_type,
                         _::ISC enemy_amount, _::ISC enemy_time_to_spawn,
                         _::FPC enemy_max_distance)
    : Tile(kSpawnerEnemy, grid_x, grid_y, grid_size_f_, texture_rect, false) {
  enemy_type_ = enemy_type;
  enemy_amount_ = enemy_amount;
  enemy_time_to_spawn_ = enemy_time_to_spawn;
  enemy_max_distance_ = enemy_max_distance;
  spawned_ = false;
}

TileSpawner::~TileSpawner() {}

const _::ISC& TileSpawner::GetEnemyType() const { return enemy_type_; }
const _::BOL& TileSpawner::GetSpawned() const { return spawned_; }

void TileSpawner::SetSpawned(const _::BOL spawned) { spawned_ = spawned; }

void TileSpawner::Update() {}
