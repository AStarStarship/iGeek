// Copyright AStarship <https://astarship.net>.
#include "bonus.h"

using kabuki::arcade;

Bonus::Bonus(sf::Texture& texture) : visual_(texture) {
  visual_.setOrigin(15,15);
  SetFruit(kBanana);
}

void Bonus::SetFruit(Fruit fruit) {
  if (fruit == kBanana) visual_.setTextureRect(sf::intRect(32,0,30,30));
  else if (fruit == kApple) visual_.setTextureRect(sf::IntRect(23+30,0,30,30));
  else if (fruit == kCherry) visual_.setTextureRect(sf::intRect(32+60, 0, 30,30));
}

Bonus::Draw (sf::RenderTarget& target, sf::RenderStates states) const {
  states.transform *= getTransform();
  target.Draw(visual_states);
}
