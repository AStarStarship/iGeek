// Copyright AStarship <https://astarship.net>.
#include "TileMap.h"

void TileMap::Clear() {
  if (map_ != NILP) {
    for (_::ISC x = 0; x < width_; x++) {
      for (_::ISC y = 0; y < height_; y++) {
        if (map_[y * width_ + x] != NILP) {
          delete map_[y * width_ + x];
          map_[y * width_ + x] = NILP;
        }
      }
    }
    delete[] map_;
    map_ = NILP;
  }
}

// PORT-NOTE: the original ctor loaded a texture sheet from file and built a
// 4-level nested _::Array; the sheet is gone and the grid is a flat array of
// Tile* (one layer). The original also had a duplicate `grid_size_f_`
// declaration (a bug — it would not compile); here grid_size_f_ holds the
// float cell size and width_/height_ hold the grid extents.
TileMap::TileMap(_::FPC grid_size, _::ISC width, _::ISC height)
    : grid_size_f_(grid_size),
      width_(width),
      height_(height),
      max_size_world_grid_(width, height),
      max_size_world_f_(static_cast<_::FPC>(width) * grid_size,
                        static_cast<_::FPC>(height) * grid_size),
      map_(NILP) {
  map_ = new Tile*[static_cast<_::IUD>(width) * static_cast<_::IUD>(height)]();
  if (!occlusion_.Init(width, height)) {
    // The occlusion grid failed to allocate; the map still functions
    // collision-wise, visibility queries simply return false.
  }
}

TileMap::~TileMap() {
  Clear();
}

const _::BOL TileMap::TileEmpty(const _::ISC x, const _::ISC y) const {
  if (x >= 0 && x < width_ && y >= 0 && y < height_) return map_[y * width_ + x] == NILP;
  return false;
}

const _::ISC TileMap::LayerSize(const _::ISC x, const _::ISC y) const {
  if (x >= 0 && x < width_) {
    if (y >= 0 && y < height_) return map_[y * width_ + x] == NILP ? 0 : 1;
  }
  return -1;
}

const _::TVec2I& TileMap::SizeGridMax() const { return max_size_world_grid_; }

const _::TVec2F& TileMap::SizeMaxF() const { return max_size_world_f_; }

// PORT-NOTE: the original pushed into map_[x][y][z] (a per-cell Tile array);
// the headless grid stores at most one tile per cell (last write wins).
void TileMap::AddTile(const _::ISC x, const _::ISC y,
                      const _::TIntRect& texture_rect, const _::BOL collision,
                      const _::ISM type) {
  if (x < width_ && x >= 0 && y < height_ && y >= 0) {
    if (map_[y * width_ + x] != NILP) delete map_[y * width_ + x];
    map_[y * width_ + x] = new TileRegular(type, x, y, grid_size_f_, texture_rect,
                                           collision);
  }
}

void TileMap::AddSpawnerTile(const _::ISC x, const _::ISC y,
                             const _::TIntRect& texture_rect, const _::ISC enemy_type,
                             const _::ISC enemy_amount, const _::ISC enemy_tts,
                             const _::FPC enemy_md) {
  if (x < width_ && x >= 0 && y < height_ && y >= 0) {
    if (map_[y * width_ + x] != NILP) delete map_[y * width_ + x];
    map_[y * width_ + x] = new TileSpawner(x, y, grid_size_f_, texture_rect,
                                           enemy_type, enemy_amount, enemy_tts,
                                           enemy_md);
  }
}

void TileMap::RemoveTile(const _::ISC x, const _::ISC y, const _::ISC type) {
  if (x < width_ && x >= 0 && y < height_ && y >= 0) {
    if (map_[y * width_ + x] != NILP) {
      if (type >= 0) {
        if (map_[y * width_ + x]->GetType() == type) {
          delete map_[y * width_ + x];
          map_[y * width_ + x] = NILP;
        }
      } else {
        delete map_[y * width_ + x];
        map_[y * width_ + x] = NILP;
      }
    }
  }
}

// PORT-NOTE: the original read map_[x][y][layer].back()->getType() with no
// empty check; the headless version guards the empty cell.
const _::BOL TileMap::CheckType(const _::ISC x, const _::ISC y,
                                const _::ISM type) const {
  if (x < 0 || x >= width_ || y < 0 || y >= height_) return false;
  Tile* t = map_[y * width_ + x];
  return t != NILP && t->GetType() == type;
}

// --- Collision (ported 1:1 from the original, sprite-less) ----------------
void TileMap::Update(Entity* player, const _::FPC& dt, Entity** enemies,
                     _::ISC enemy_count) {
  // --- World-bounds + tile collision for the player and every enemy. ---
  {
    Entity* all[8];
    _::ISC n = 0;
    all[n++] = player;
    for (_::ISC i = 0; i < enemy_count && n < 8; i++) all[n++] = enemies[i];

    for (_::ISC e = 0; e < n; e++) {
      Entity* entity = all[e];
      if (entity == NILP) continue;
      if (entity->Position().x < 0.f) {
        entity->PositionSet(0.f, entity->Position().y);
        entity->StopVelocityX();
      } else if (entity->Position().x + entity->GlobalBounds().w >
                 max_size_world_f_.x) {
        entity->PositionSet(max_size_world_f_.x - entity->GlobalBounds().w,
                            entity->Position().y);
        entity->StopVelocityX();
      }
      if (entity->Position().y < 0.f) {
        entity->PositionSet(entity->Position().x, 0.f);
        entity->StopVelocityY();
      } else if (entity->Position().y + entity->GlobalBounds().h >
                 max_size_world_f_.y) {
        entity->PositionSet(entity->Position().x,
                            max_size_world_f_.y - entity->GlobalBounds().h);
        entity->StopVelocityY();
      }

      // --- Tile collision (ported 1:1 from UpdateTileCollision). ---
      _::ISC from_x = entity->GridPosition(grid_size_f_).x - 1;
      if (from_x < 0)
        from_x = 0;
      else if (from_x > width_)
        from_x = width_;

      _::ISC to_x = entity->GridPosition(grid_size_f_).x + 3;
      if (to_x < 0)
        to_x = 0;
      else if (to_x > width_)
        to_x = width_;

      _::ISC from_y = entity->GridPosition(grid_size_f_).y - 1;
      if (from_y < 0)
        from_y = 0;
      else if (from_y > height_)
        from_y = height_;

      _::ISC to_y = entity->GridPosition(grid_size_f_).y + 3;
      if (to_y < 0)
        to_y = 0;
      else if (to_y > height_)
        to_y = height_;

      for (_::ISC x = from_x; x < to_x; x++) {
        for (_::ISC y = from_y; y < to_y; y++) {
          Tile* t = map_[y * width_ + x];
          if (t == NILP) continue;
          _::TFloatRect player_bounds = entity->GlobalBounds();
          _::TFloatRect wall_bounds = t->GlobalBounds();
          _::TFloatRect next_position_bounds = entity->NextPositionBounds(dt);

          // PORT-NOTE: the original checked `map_[x][y][layer][k]->getCollision() &&
          // ->intersects(nextPositionBounds)` but only applied the resolution when
          // it intersected the NEXT position; the per-face tests below use the
          // CURRENT bounds. This asymmetry is preserved (original quirk).
          if (t->GetCollision() && t->Intersects(next_position_bounds)) {
            // Bottom collision
            if (player_bounds.Top() < wall_bounds.Top() &&
                player_bounds.Top() + player_bounds.h <
                    wall_bounds.Top() + wall_bounds.h &&
                player_bounds.Left() < wall_bounds.Left() + wall_bounds.w &&
                player_bounds.Left() + player_bounds.w > wall_bounds.Left()) {
              entity->StopVelocityY();
              entity->PositionSet(player_bounds.Left(),
                                  wall_bounds.Top() - player_bounds.h);
            }

            // Top collision
            else if (player_bounds.Top() > wall_bounds.Top() &&
                     player_bounds.Top() + player_bounds.h >
                         wall_bounds.Top() + wall_bounds.h &&
                     player_bounds.Left() < wall_bounds.Left() + wall_bounds.w &&
                     player_bounds.Left() + player_bounds.w > wall_bounds.Left()) {
              entity->StopVelocityY();
              entity->PositionSet(player_bounds.Left(),
                                  wall_bounds.Top() + wall_bounds.h);
            }

            // Right collision
            if (player_bounds.Left() < wall_bounds.Left() &&
                player_bounds.Left() + player_bounds.w <
                    wall_bounds.Left() + wall_bounds.w &&
                player_bounds.Top() < wall_bounds.Top() + wall_bounds.h &&
                player_bounds.Top() + player_bounds.h > wall_bounds.Top()) {
              entity->StopVelocityX();
              entity->PositionSet(wall_bounds.Left() - player_bounds.w,
                                  player_bounds.Top());
            }

            // Left collision
            else if (player_bounds.Left() > wall_bounds.Left() &&
                     player_bounds.Left() + player_bounds.w >
                         wall_bounds.Left() + wall_bounds.w &&
                     player_bounds.Top() < wall_bounds.Top() + wall_bounds.h &&
                     player_bounds.Top() + player_bounds.h > wall_bounds.Top()) {
              entity->StopVelocityX();
              entity->PositionSet(wall_bounds.Left() + wall_bounds.w,
                                  player_bounds.Top());
            }
          }
        }
      }
    }
  }

  // --- Tile updates (spawners) near the player. ---
  {
    _::ISC from_x = player->GridPosition(grid_size_f_).x - 15;
    if (from_x < 0)
      from_x = 0;
    else if (from_x > width_)
      from_x = width_;

    _::ISC to_x = player->GridPosition(grid_size_f_).x + 16;
    if (to_x < 0)
      to_x = 0;
    else if (to_x > width_)
      to_x = width_;

    _::ISC from_y = player->GridPosition(grid_size_f_).y - 8;
    if (from_y < 0)
      from_y = 0;
    else if (from_y > height_)
      from_y = height_;

    _::ISC to_y = player->GridPosition(grid_size_f_).y + 9;
    if (to_y < 0)
      to_y = 0;
    else if (to_y > height_)
      to_y = height_;

    for (_::ISC x = from_x; x < to_x; x++) {
      for (_::ISC y = from_y; y < to_y; y++) {
        Tile* t = map_[y * width_ + x];
        if (t == NILP) continue;
        t->Update();
        // Spawner tiles are present in the grid; the headless spine does not
        // spawn enemies from them (no EnemySystem in this pass).
      }
    }
  }

  // --- Occlusion map: rebuild layers A+B, recompute visibility (layer C). ---
  RebuildOcclusion(player, enemies, enemy_count);
}

// --- Occlusion wiring ------------------------------------------------------
void TileMap::RebuildOcclusion(Entity* player, Entity** enemies,
                               _::ISC enemy_count) {
  if (occlusion_.width <= 0) return;  // Init failed

  // Clear the occluder grid, then layer A: collision tiles.
  for (_::ISC y = 0; y < height_; y++) {
    for (_::ISC x = 0; x < width_; x++) {
      Tile* t = map_[y * width_ + x];
      occlusion_.SetOccluded(x, y, t != NILP && t->GetCollision());
    }
  }

  // Layer B: entities flagged occlude_ (the player and any enemy).
  if (player != NILP && player->Occlude()) MarkEntityOccluder(player);
  for (_::ISC i = 0; i < enemy_count; i++) {
    if (enemies[i] != NILP && enemies[i]->Occlude()) MarkEntityOccluder(enemies[i]);
  }

  // Observers: player = 0, enemies = 1..n (grid-cell-center positions).
  occlusion_.ClearObservers();
  obs_player_ = -1;
  obs_enemy_count_ = 0;
  if (player != NILP) {
    _::TVec2F c = player->Center();
    obs_player_ =
        occlusion_.AddObserver(c.x / grid_size_f_, c.y / grid_size_f_);
  }
  for (_::ISC i = 0; i < enemy_count && i < 7; i++) {
    if (enemies[i] == NILP) continue;
    _::TVec2F c = enemies[i]->Center();
    obs_enemy_[obs_enemy_count_++] =
        occlusion_.AddObserver(c.x / grid_size_f_, c.y / grid_size_f_);
  }

  occlusion_.Recompute();
}

// Mark every grid cell the entity's bounds cover as a dynamic occluder.
void TileMap::MarkEntityOccluder(const Entity* e) {
  _::TFloatRect b = e->GlobalBounds();
  _::ISC x0 = static_cast<_::ISC>(b.Left() / grid_size_f_);
  _::ISC y0 = static_cast<_::ISC>(b.Top() / grid_size_f_);
  _::ISC x1 = static_cast<_::ISC>(b.Right() / grid_size_f_);
  _::ISC y1 = static_cast<_::ISC>(b.Bottom() / grid_size_f_);
  for (_::ISC x = x0; x <= x1; x++)
    for (_::ISC y = y0; y <= y1; y++) occlusion_.SetOccluded(x, y, true);
}

_::BOL TileMap::PlayerSeesCell(const _::ISC cx, const _::ISC cy) const {
  if (obs_player_ < 0) return false;
  return occlusion_.ObserverSees(obs_player_, cx, cy);
}

_::BOL TileMap::PlayerSees(const Entity* e) const {
  if (e == NILP) return false;
  _::TVec2I g = e->GridPosition(grid_size_f_);
  return PlayerSeesCell(g.x, g.y);
}

_::BOL TileMap::PlayerSeesEnemy(const Entity* enemy, _::ISC i) const {
  (void)i;  // i is the observer index of the enemy; the query is player->enemy
  return PlayerSees(enemy);
}
