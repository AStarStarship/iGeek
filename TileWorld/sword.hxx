// Copyright AStarship <https://astarship.net>.
#include "sword.h"

Sword::Sword() {
  if (!weapon_texture.LoadFromFile("Resources/Images/Sprites/Player/sword.png"))
    D_COUT("\nError: Couldn't load Player's weapon texture!.");

  weapon_sprite.setTexture(weapon_texture);

  weapon_sprite.setOrigin(weapon_sprite.GlobalBounds().width / 2.f,
                          weapon_sprite.GlobalBounds().height);
}

Sword::~Sword() {}

void Sword::Update(const sf::Vector2f& mouse_pos_view,
                   const sf::Vector2f center) {
  weapon_sprite.PositionSet(center);

  float dX = mouse_pos_view.x - weapon_sprite.getPosition().x;
  float dY = mouse_pos_view.y - weapon_sprite.getPosition().y;

  const float PI = 3.14159265f;
  float deg = atan2(dY, dX) * 180.f / PI;

  weapon_sprite.setRotation(deg + 90.f);
}

void Sword::Render(sf::RenderTarget& target, sf::Shader* shader) {
  if (shader)
    target.draw(weapon_sprite, shader);
  else
    target.draw(weapon_sprite);
}
