// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekTileEnvH
#define _iGeekTileEnvH
#include <_Config.h>
#if _Seam >= _iGeekTileWorldCore
#include "Environment.h"
#include "Entity.h"
#include "TileMap.h"
#include "Stringf.hpp"

namespace iGeek {

/* A concrete actor: Entity is abstract (Update is pure), and the tile world
   only needs a position to move, so the env's player/enemies are bare actors.
   Renderer, hitbox and weapon components are irrelevant headless. */
class TileActor : public ::Entity
{
 public:
  void Update(const _::FPC& dt) { (void)dt; }
};

/* The tile world AS an Environment: a Room/thread an agent can join. The
   action is a 4-way move on the tile grid; the reward is a step cost plus a
   bonus for keeping an enemy in the (occlusion-computed) field of view. The
   env IS the Environment, so it owns the Room and the manifest a joining
   agent reads. */
class TileEnv : public Environment
{
 public:
  enum {
    GridWidth = 24,
    GridHeight = 18,
    EnemyCount = 3,
    ObservationLength = 12,
    ActionCount = 4,          //< up, down, left, right
    MaxSteps = 512,
  };
  TileEnv(IUD seed = 7);
  void Reset();
  FPC Step(ISC action);
  void Observe();
  const FPC* Observation() { return observation_; }
  BOL Done() { return done_ || steps_ >= ISC(MaxSteps); }
  ISC Steps() { return steps_; }
  ::TileMap& Map() { return map_; }

 private:
  ::TileMap map_;
  TileActor player_;
  TileActor enemies_[EnemyCount];
  ::Entity* enemy_ptrs_[EnemyCount];
  ISC player_x_, player_y_, steps_;
  BOL done_;
  ISC seen_mask_;            //< which enemies have been seen this episode
  FPC observation_[ObservationLength] = {};
  IUD seed_;
};

}  //< namespace iGeek
#endif  //< _Seam >= _iGeekTileWorldCore
#endif  //< _iGeekTileEnvH
