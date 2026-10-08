// Copyright AStarship <https://astarship.net>.
#pragma once
#include <_Config.h>

#ifndef TILEWORLD_RAT
#define TILEWORLD_RAT

#include "enemy.h"

class Rat : public Enemy {
  void InitVariables();
  void InitAnimation();

  public:

  Rat(FPC x, FPC y, sf::Texture& texture_sheet);
  virtual ~Rat();

  void UpdateAnimation(const FPC& dt);
  void Update(const FPC& dt, sf::Vector2f& mouse_pos_view);

  void Render(sf::RenderTarget& target, sf::Shader* shader,
              const sf::Vector2f light_position, const bool show_hitbox);
};

#endif
