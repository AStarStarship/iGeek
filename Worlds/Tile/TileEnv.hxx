// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekTileEnvHXX
#define _iGeekTileEnvHXX
#include <_Config.h>
#if _Seam >= _iGeekTileWorldCore
#include "TileEnv.h"
#include "TileRegular.h"
#include "TileSpawner.h"
#include "../../Gym.h"

namespace iGeek {

/* Cell size in world units. Positions are world units; the grid cell is
   position / GridSize. */
static const _::FPC TileEnvCellSize = 16.0f;

TileEnv::TileEnv(IUD seed)
    : Environment("tileworld", 2), map_(TileEnvCellSize, ISC(GridWidth),
                                       ISC(GridHeight)),
      player_x_(1), player_y_(1), steps_(0), done_(false), seen_mask_(0),
      seed_(seed ? seed : IUD(7))
{
  /* The tile env IS the Environment: it owns the Room and the manifest a
     joining agent reads. */
  TSPrint(manifest_.name, ISW(ManifestNameLen), "tileworld");
  TSPrint(manifest_.version, ISW(ManifestVersionLen), "1.0");
  manifest_.kind = ManifestKindEnv;
  manifest_.domain = ManifestDomainGame;
  TSPrint(manifest_.description, ISW(ManifestDescriptionLen),
          "Top-down tile world: navigate, keep enemies in view");
  TSPrint(manifest_.modality, ISW(ManifestModalityLen), "vector");
  manifest_.reward_low = FPC(-0.05);
  manifest_.reward_high = FPC(1.0);
  manifest_.reward_higher_is_better = true;
  manifest_.determinism = ManifestDeterminismSeeded;
  manifest_.observation_count = ISC(ObservationLength);
  manifest_.action_count = ISC(ActionCount);
  manifest_.max_episode_steps = ISC(MaxSteps);
  manifest_.environment_count_max = ISC(GymEnvMax);
  TSPrint(manifest_.world_name, ISW(ManifestWorldLen), "TileWorld");
  manifest_.seam_number = _iGeekTileWorldCore;
  manifest_.manifest_version = ISC(1);
  for (ISC i = 0; i < ISC(EnemyCount); ++i) enemy_ptrs_[i] = &enemies_[i];
  Reset();
}

void TileEnv::Reset() {
  player_x_ = 1;
  player_y_ = 1;
  steps_ = 0;
  done_ = false;
  seen_mask_ = 0;
  player_.PositionSet(FPC(player_x_) * TileEnvCellSize,
                      FPC(player_y_) * TileEnvCellSize);
  /* Deterministic enemy placement: same seed -> same layout. */
  for (ISC i = 0; i < ISC(EnemyCount); ++i) {
    const ISC ex = 3 + ISC((seed_ + IUD(i) * 7u) % IUD(ISC(GridWidth) - 6));
    const ISC ey = 3 + ISC((seed_ + IUD(i) * 13u) % IUD(ISC(GridHeight) - 6));
    enemies_[i].PositionSet(FPC(ex) * TileEnvCellSize,
                            FPC(ey) * TileEnvCellSize);
  }
  map_.Update(&player_, FPC(1.0f / 60.0f), enemy_ptrs_, ISC(EnemyCount));
  Observe();
}

FPC TileEnv::Step(ISC action) {
  ISC dir = action;
  if (dir < 0) dir = 0;
  if (dir >= ISC(ActionCount)) dir = ISC(ActionCount) - 1;
  ISC nx = player_x_;
  ISC ny = player_y_;
  if (dir == 0) --ny;
  else if (dir == 1) ++ny;
  else if (dir == 2) --nx;
  else ++nx;
  ++steps_;
  FPC reward = FPC(-0.01f);
  if (nx < 0 || nx >= ISC(GridWidth) || ny < 0 || ny >= ISC(GridHeight) ||
      map_.CellBlocked(nx, ny)) {
    reward = FPC(-0.05f);          //< blocked: penalty, stay put
  } else {
    player_x_ = nx;
    player_y_ = ny;
    player_.PositionSet(FPC(player_x_) * TileEnvCellSize,
                        FPC(player_y_) * TileEnvCellSize);
  }
  map_.Update(&player_, FPC(1.0f / 60.0f), enemy_ptrs_, ISC(EnemyCount));
  /* Reward the first time each enemy comes into view. */
  ISC newly_seen = 0;
  for (ISC i = 0; i < ISC(EnemyCount); ++i) {
    if ((seen_mask_ & (1 << i)) == 0 && map_.PlayerSeesEnemy(enemy_ptrs_[i], i)) {
      seen_mask_ |= (1 << i);
      ++newly_seen;
    }
  }
  if (newly_seen > 0) reward += FPC(0.5f) * FPC(newly_seen);
  if (seen_mask_ == ((1 << ISC(EnemyCount)) - 1)) {
    done_ = true;
    reward += FPC(1.0f);
  }
  Observe();
  return reward;
}

void TileEnv::Observe() {
  const FPC gw = FPC(GridWidth);
  const FPC gh = FPC(GridHeight);
  observation_[0] = FPC(player_x_) / gw;
  observation_[1] = FPC(player_y_) / gh;
  for (ISC d = 0; d < ISC(ActionCount); ++d) {
    ISC nx = player_x_;
    ISC ny = player_y_;
    if (d == 0) --ny;
    else if (d == 1) ++ny;
    else if (d == 2) --nx;
    else ++nx;
    const BOL blocked = nx < 0 || nx >= ISC(GridWidth) || ny < 0 ||
                        ny >= ISC(GridHeight) || map_.CellBlocked(nx, ny);
    observation_[2 + d] = blocked ? 1.0f : 0.0f;
  }
  for (ISC i = 0; i < ISC(EnemyCount); ++i) {
    observation_[6 + i] = map_.PlayerSeesEnemy(enemy_ptrs_[i], i) ? 1.0f : 0.0f;
  }
  observation_[9] = FPC(seen_mask_) / FPC(EnemyCount);
  observation_[10] = done_ ? 1.0f : 0.0f;
  observation_[11] = FPC(steps_) / FPC(MaxSteps);
}

}  //< namespace iGeek
#endif  //< _Seam >= _iGeekTileWorldCore
#endif  //< _iGeekTileEnvHXX
