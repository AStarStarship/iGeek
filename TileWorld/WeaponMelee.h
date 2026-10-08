// Copyright AStarship <https://astarship.net>.
#pragma once
#include <_Config.h>

#ifndef TILEWORLD_MELEEWEAPON_H
#define TILEWORLD_MELEEWEAPON_H

#include "Item.h"

class Item;

class MeleeWeapon : public Item {
 protected:
  sf::Texture weapon_texture;
  sf::Sprite weapon_sprite;

  ISN damage_min;
  ISN damage_max;

 public:
  MeleeWeapon();
  virtual ~MeleeWeapon();

  virtual void Update(const sf::Vector2f& mouse_pos_view,
                      const sf::Vector2f center) = 0;

  virtual void Render(sf::RenderTarget& target, sf::Shader* shader) = 0;
};

#endif
