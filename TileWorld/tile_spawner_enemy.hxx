// Copyright AStarship <https://astarship.net>.
#include "tile_spawner_enemy.h"

EnemySpawnerTile::EnemySpawnerTile(int grid_x, int grid_y, float grid_size_f,
                                   const sf::Texture& texture,
                                   const sf::IntRect& texture_rect,
                                   int enemy_type, int enemy_amount,
                                   int enemy_time_to_spawn,
                                   float enemy_max_distance)

    : Tile(TileTypes::kSpawnerEnemy, grid_x, grid_y, grid_size_f, texture,
           texture_rect, false) {
  enemy_type_ = enemy_type;
  enemy_amount_ = enemy_amount;
  enemy_time_to_spawn_ = enemy_time_to_spawn;
  enemy_max_distance_ = enemy_max_distance;
  spawned_ = false;
}

EnemySpawnerTile::~EnemySpawnerTile() {}

const _::AString EnemySpawnerTile::ToString() const {
  std::stringstream ss;
  ss << type_ << " " << shape_.getTextureRect().left << " "
     << shape_.getTextureRect().Top << " " << enemy_type_ << " "
     << enemy_amount_ << " " << enemy_time_to_spawn_ << " "
     << enemy_max_distance_;

  std::cout << ss.str() << "\n";

  return ss.str();
}

const bool& EnemySpawnerTile::GetSpawned() const { return spawned_; }

void EnemySpawnerTile::SetSpawned(const bool spawned) { spawned_ = spawned; }

void EnemySpawnerTile::Update() {}

void EnemySpawnerTile::Render(sf::RenderTarget& target, sf::Shader* shader,
                              const sf::Vector2f player_position) {
  if (shader) {
    shader->setUniform("HasTexture", true);
    shader->setUniform("LightPos", player_position);

    target.draw(shape_, shader);
  } else
    target.draw(shape_);
}
