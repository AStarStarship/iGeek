// Copyright AStarship <https://astarship.net>.
#include "stdafx.h"
//
#include "enemy_system.h"

EnemySystem::EnemySystem(_::Array<Enemy*>& active_enemies_,
                         _::ADic<_::AString, sf::Texture>& textures)
    : textures(textures), active_enemies_(active_enemies_) {}

EnemySystem::~EnemySystem() {}

void EnemySystem::CreateEnemy(const short type, const float xPos,
                              const float yPos) {
  switch (type) {
    case EnemyTypes::RAT:
      active_enemies_.Push(new Rat(xPos, yPos, textures["RAT1_SHEET"]));
      break;
    default:
      std::cout << "\nError: EnemySystem::CreateEnemy Type does not exist!";
      break;
  }
}

void EnemySystem::Update(const float& dt) {}

void EnemySystem::Render(sf::RenderTarget* target) {}
