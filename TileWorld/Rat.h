// Copyright AStarship <https://astarship.net>.
#pragma once
#include "TVec.h"
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
  void Update(const FPC& dt, _::FPC2& mouse_pos_view);

  void Render(sf::RenderTarget& target, sf::Shader* shader,
              const _::FPC2 light_position, const bool show_hitbox);
};

#endif
