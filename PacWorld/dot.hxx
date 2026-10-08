// Copyright AStarship <https://astarship.net>.
#include "dot.h"

using namespace kabuki::arcade;

sf::CircleShape Dot() {
  sf::CircleShape dot;
  dot.setRadius(4);
  dot.setFillColor(sf::Color::White);
  dot.setOrigin(2, 2);
  return dot;
}

sf::CircleShape SuperDot() {
  sf::CircleShape dot;
  dot.setRadius(8);
  dot.setFillColor(sf::Color::White);
  dot.setOrigin(4, 4);
  return dot;
}
