// Copyright AStarship <https://astarship.net>.
//
// Entity.h — headless ECS entity (TileWorld core, SFML stripped).
//
// SFML port notes:
//   - sf::Sprite is GONE. The entity owns its position as a FPC2 (pos_).
//     The old code used the sprite as the shared position store and passed
//     sf::Sprite& into the components; now the components read/update a
//     _::FPC2& (the entity's pos_). This is the one consistent
//     position-ownership model used across the spine.
//   - _::FPC2 -> _::FPC2, _::ISC2 -> _::TVec2I, _::FPC4 ->
//     _::TFloatRect, _::ISC4 -> _::TIntRect (ASCIICrabs/TVec.h).
//   - Render() is DELETED (headless). A future SDL3/CrabsTK renderer reads
//     pos_ + texture_id + the occlusion map; it is not this layer's concern.
//   - 'Sprite' as a concept is DATA ONLY here: texture_id (which atlas cell a
//     future renderer draws) and occlude_ (dynamic occluder, layer B).
//   - Animation/Attribute/Skill components are OUT OF SCOPE for this pass
//     (2nd pass): their members and Create* methods are removed.
//   - No C++ stdlib. No-stdlib.
#pragma once
#include <_Config.h>
#include <Homotuples.h>

#ifndef TILEWORLD_ENTITY
#define TILEWORLD_ENTITY

#include "ComponentHitbox.h"
#include "ComponentMovement.h"

class Entity {
 private:
  void InitVariables();

 protected:
  // The entity's position (was the sf::Sprite position). Owner: Entity.
  // HitboxComponent offsets from it; MovementComponent mutates it.
  _::FPC2 pos_;
  _::FPC2 size_;  // bounds (was sprite global bounds w/h)
  _::IUD texture_id;  // which atlas cell a future renderer draws (data only)
  _::BOL occlude_;  // if true, this entity is a dynamic occluder (layer B)

  TileWorld::HitboxComponent* hitbox_component_;
  TileWorld::MovementComponent* movement_;

 public:
  Entity();
  virtual ~Entity();

  // Data-only "sprite" accessors (no sf::Sprite).
  void SetTextureId(_::IUD texture_id);
  void SetOcclude(_::BOL on);
  _::BOL Occlude();

  void CreateHitboxComponent(_::FPC offset_x, _::FPC offset_y, _::FPC width,
                             _::FPC height);
  void CreateMovementComponent(const _::FPC velocity_max,
                               const _::FPC acceleration,
                               const _::FPC deceleration);

  virtual const _::FPC2& Position();
  virtual const _::FPC2 Center();
  virtual _::TVec2I GridPosition(const _::FPC grid_size_f_);
  virtual _::TFloatRect GlobalBounds();
  virtual const _::TFloatRect NextPositionBounds(const _::FPC& dt);

  virtual void PositionSet(const _::FPC x, const _::FPC y);

  virtual void Move(const _::FPC x, const _::FPC y, const _::FPC& dt);
  virtual void StopVelocity();
  virtual void StopVelocityX();
  virtual void StopVelocityY();

  virtual void Update(const _::FPC& dt) = 0;
};

#endif
