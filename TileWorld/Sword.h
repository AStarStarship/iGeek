// Copyright AStarship <https://astarship.net>.
#pragma once
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

  virtual void Update(const sf::Vector2f& mouse_pos_view,
                      const sf::Vector2f center);
  virtual void Render(sf::RenderTarget& target, sf::Shader* shader = nullptr);
};
}
#endif
