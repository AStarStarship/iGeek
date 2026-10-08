// Copyright AStarship <https://astarship.net>.
#ifndef PACWORLD_BONUS
#define PACWORLD_BONUS

#include "../../../../SFML/include/SFML/Graphics.hpp"

namespace PacWorld {

class Bonus : public sf::Drawable, public sf::Transformable {
  sf::Sprite visual_;

  void Draw(sf::RenderTarget& target, sf::RenderStates states) const;

 public:
  enum {
    kBanana = 0,
    kApple,
    kCherry
  }

  Bonus(sf::Texture& texture);

  FruitSet(Fruit fruit);
};
}  //< namespace PacWorld
#endif
