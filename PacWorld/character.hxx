// Copyright AStarship <https://astarship.net>.
#include "character.h"

using namespace kabuki::arcade;

Character::Character()
    : speed_(1.f), direction_(1, 0), next_direction_(0, 0), maze_(nullptr) {}

void Character::Update(sf::Time delta) {
  sf::Vector2f pixel_position = getPosition();

  float pixel_traveled = Speed() * delta.asSeconds();

  sf::Vector2f next_pixel_position =
      pixel_position + sf::Vector2f(direction_) * pixel_traveled;
  setPosition(next_pixel_position);

  sf::Vector2i cell_position = maze_->PixelToCell(pixel_position);

  sf::Vector2f offset;
  offset.x = std::fmod(pixel_position.x, 32) - 16;
  offset.y = std::fmod(pixel_position.y, 32) - 16;

  if (maze_->IsWall(cell_position + direction_)) {
    if ((direction_.x == 1 && offset.x > 0) ||
        (direction_.x == -1 && offset.x < 0) ||
        (direction_.y == 1 && offset.y > 0) ||
        (direction_.y == -1 && offset.y < 0)) {
      setPosition(maze_->CellToPixel(cell_position));
    }
  }

  if (!maze_->IsWall(cell_position + direction_) &&
      direction_ != next_direction_) {
    if ((!direction_.y && (offset.x > -2 && offset.x < 2)) ||
        (!direction_.x && (offset.y > -2 && offset.y < 2))) {
      setPosition(CellToPixel(cell_position));
      direction_ = next_direction_;

      if (direction_ == sf::Vector2i(1, 0)) {
        setRotation(0);
        setScale(-1, 1);
      } else if (direction_ == sf::Vector2i(0, 1)) {
        setRotation(90);
        setScale(-1, 1);
      } else if (direction_ == sf::Vector2i(-1, 0)) {
        setRotation(0);
        setScale(1, 1);
      } else if (direction_ == sf::Vector2i(0, -1)) {
        setRotation(90);
        setScale(-1, 1);
      }
    }
  }
  static sf::Vector2i directions[4] = {
      sf::Vector2i(1, 0),
      sf::Vector2i(0, 1),
      sf::Vector2i(-1, 0),
      sf::Vector2i(0, -1),
  };
  if (cell_position != previous_intersection_) {
    if ((!current_direction_.y && (offset.x > -2 && offset.x < 2)) ||
        (!current_direction_.x && (offset.y > -2 && offset.y < 2))) {
      std::array<bool, 4> available_directions;

      int i = 0;
      for (auto direction : directions) {
        available_directions[i] = maze_->IsWall(cell_position + directions);
        ++i;
      }
      if (available_directions_ != available_directions) {
        previous_intersection_ = cell_position;
        available_directions_ = available_directions;
        ChangeDirection();
      }
    }
  }
}

void Character::SetDirection(sf::Vector2i direction) { direction_ = direction; }

sf::Vector2i Character::Direction() const { return direction_; }

void Character::SetSpeed(float speed) { speed_ = speed; }

float Character::Speed() const { return speed_; }

sf::FloatRect Character::CollisionBox() const {
  sf::FloatRect bounds(3, 3, 34, 34);
  return getTransform().transformRect(bounds);
}

void Character::SetMaze(Maze* maze) {
  if (!maze) return;
  maze_ = maze;
}

bool Character::WillMove() {
  return !maze_->IsWall(previous_intersection_ + next_direction_);
}
