// Copyright AStarship <https://astarship.net>.
#ifndef PACWORD_PAC
#define PACWORD_PAC
#include "Character.h"
//
#include "Animator.h"
namespace PacWorld {

class Pac : public Character {
  bool is_dead_, is_dying_;
  sf::Sprite visual_;
  Animator run_animator_, die_animator_;

  void Draw(sf::RenderTarget& target, sf::RenderStates states) const;

 public:
  Pac(sf::Texture& texture);

  void Die();

  void Reset();

  bool IsDying();

  bool IsDead();

  void Update(sf::Time delta);
};
}  //< namespace PackWorld
#endif
