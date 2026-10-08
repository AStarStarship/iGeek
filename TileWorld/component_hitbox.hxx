// Copyright AStarship <https://astarship.net>.
#include "component_hitbox.h"

HitboxComponent::HitboxComponent(sf::Sprite& sprite, float offset_x,
                                 float offset_y, float width, float height)
    : sprite(sprite), offsetX(offset_x), offsetY(offset_y) {
  nextPosition.left = 0.f;
  nextPosition.Top = 0.f;
  nextPosition.width = width;
  nextPosition.height = height;

  hitbox.PositionSet(sprite.getPosition().x + offset_x,
                     sprite.getPosition().y + offset_y);
  hitbox.setSize(sf::Vector2f(width, height));
  hitbox.setFillColor(sf::Color::Transparent);
  hitbox.setOutlineThickness(-1.f);
  hitbox.setOutlineColor(sf::Color::Green);
}

HitboxComponent::~HitboxComponent() {}

const sf::Vector2f& HitboxComponent::getPosition() const {
  return hitbox.getPosition();
}

const sf::FloatRect HitboxComponent::GlobalBounds() const {
  return hitbox.GlobalBounds();
}

const sf::FloatRect& HitboxComponent::NextPosition(
    const sf::Vector2f& velocity) {
  nextPosition.left = hitbox.getPosition().x + velocity.x;
  nextPosition.Top = hitbox.getPosition().y + velocity.y;

  return nextPosition;
}

void HitboxComponent::PositionSet(const sf::Vector2f& position) {
  hitbox.PositionSet(position);
  sprite.PositionSet(position.x - offsetX, position.y - offsetY);
}

void HitboxComponent::PositionSet(const float x, const float y) {
  hitbox.PositionSet(x, y);
  sprite.PositionSet(x - offsetX, y - offsetY);
}

bool HitboxComponent::intersects(const sf::FloatRect& frect) {
  return hitbox.GlobalBounds().intersects(frect);
}

void HitboxComponent::Update() {
  hitbox.PositionSet(sprite.getPosition().x + offsetX,
                     sprite.getPosition().y + offsetY);
}

void HitboxComponent::Render(sf::RenderTarget& target) { target.draw(hitbox); }
