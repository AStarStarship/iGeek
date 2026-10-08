// Copyright AStarship <https://astarship.net>.
#ifndef PACWORLD_GHOST
#define PACWORLD_GHOST
//
#include "pac.h"

namespace kabuki {
namespace arcade {

class Ghost : public Character {
  bool is_weak_;
  sf::Time weakness_duration_;
  sf::Sprite visual_;
  Pac* pac_;
  Animator strong_animator_, weak_animator_;

  void draw(sf::RenderTarget& target, sf::RenderStates states) const;

 public:
  enum {
    kStateStrong = 0,
    kStateWeak
  }

  Ghost(sf::Texture& texture, Pac* pacwomen);

  void SetWeak(sf::Time duration);
  bool IsWeak() const;

  void Update(sf::Time delta);

 protected:
  void ChangeDirection();
};
}  //< namespace arcade
}  //< namespace kabuki
#endif
