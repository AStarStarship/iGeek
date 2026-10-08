// Copyright AStarship <https://astarship.net>.
#include "tile_regular.h"

TileRegular::TileRegular(short type, int grid_x, int grid_y, float grid_size_f_,
                         const sf::Texture& texture,
                         const sf::IntRect& texture_rect, bool collision)
    : Tile(type, grid_x, grid_y, grid_size_f_, texture, texture_rect,
           collision) {}

TileRegular::~TileRegular() {}

const _::AString TileRegular::ToString() const {
  std::stringstream ss;

  ss << type_ << " " << shape_.getTextureRect().left << " "
     << shape_.getTextureRect().Top << " " << collision_;

  std::cout << ss.str() << "\n";

  return ss.str();
}

void TileRegular::Update() {}

void TileRegular::Render(sf::RenderTarget& target, sf::Shader* shader,
                         const sf::Vector2f player_position) {
  if (shader) {
    shader->setUniform("HasTexture", true);
    shader->setUniform("LightPos", player_position);

    target.draw(shape_, shader);
  } else
    target.draw(shape_);
}
