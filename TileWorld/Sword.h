// Copyright AStarship <https://astarship.net>.
#pragma once
#include "TVec.h"
#ifndef TILEWORLD_SWORD
#define TILEWORLD_SWORD
#include <_Config.h>
//
#include "WeaponMelee.h"
namespace TileWorld {
class MeleeWeapon;
class Sword : public MeleeWeapon {
 private:
 public:
  Sword();
  virtual ~Sword();

  virtual void Update(const _::FPC2& mouse_pos_view,
                      const _::FPC2 center);
  virtual void Render(sf::RenderTarget& target, sf::Shader* shader = nullptr);
};
}
#endif
