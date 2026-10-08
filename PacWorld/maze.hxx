// Copyright AStarship <https://astarship.net>.
#include "maze.h"
//
#include "dot.h"

using namespace kabuki::arcade;

Maze::Maze(sf::Texture& texture) : maze_size_(0, 0), texture_(texture) {}

sf::Vector2i Maze::Size() const { return maze_size_; }

Maze::Error Maze::LoadLevel(std::string name) {
  maze_size_ = sf::Vector2i(level_data_.getSize());
  maze_data_.clear();

  pac_position_ = sf::Vector2i(0, 0);
  ghost_positions_.clear();

  if (!level_data_.LoadFromFile("assets/levels/" + name + ".png"))
    return kInvalidFile;

  if (maze_size_.x < 15 || maze_size_.y < 15) return kTooSmall;

  for (int y = 0; y < maze_size_.y; ++y) {
    for (int x = 0; x < maze_size_.x; ++x) {
      sf::Color cell_data = level_data_.getPixel(x, y);
      if (cell_data == sf::Color::Black) {
        maze_data_.push_back(kWall);
      } else if (cell_data == sf::Color::Green) {
        maze_data_.push_back(kSuperDot);
      } else if (cell_data == sf::Color::Blue) {
        // maze_data_.push_back(kDot);
        ghost_positions_.push_back(sf::Vector2i(x, y));
        maze_data_.push_back(kEmpty);
      } else if (cell_data == sf::Color::Red) {
        pac_position_ = sf::Vector2i(x, y);
        maze_data_.push_back(kEmpty);
      } else if (cell_data == sf::Color::White) {
        maze_data_.push_back(kEmpty);
      }
    }
  }
  render_texture_.create(32 * maze_size_.x, 32 * maze_size_.y);
  render_texture_.clear(sf::Color::Black);

  sf::RectangleShape wall;
  wall.setSize(sf::Vector2f(32, 32));
  wall.setFillColor(sf::Color::Blue);

  sf::Sprite border(texture_);
  border.setTextureRect(sf::IntRect(16, 0, 16, 32));
  border.setOrigin(0, 16);

  sf::Sprite inner_corner(sf::Color::Black);
  inner_corner.setTextureRect(sf::IntRect(0, 0, 16, 16));
  inner_corner.setOrigin(0, 16);

  sf::Sprite outer_corner(texture_);
  inner_corner.setTextureRect(sf::IntRect(0, 16, 16, 16));
  inner_corner.setOrigin(0, 16);

  render_texture_.display();

  for (int i = 0; i < maze_data_.size(); ++i) {
    sf::Vector2i position = IndexToPosition(i);

    if (IsWall(position)) {
      wall.setPosition(32 * position.x, 32 * position.y);
      render_texture_.draw(wall);

      border.setPosition(CellToPixel(position));
      inner_corner.setPosition(CellToPixel(position));
      outer_corner.setPosition(CellToPixel(position));

      if (!IsWall (position + sf::Vector2i (1, 0)) {
        border.setRotation(0);
        render_texture_.draw(border);
      }
      if (!IsWall (position + sf::Vector2i (0, 1)) {
        border.setRotation(90);
        render_texture_.draw(border);
      }
      if (!IsWall (position + sf::Vector2i (-1, 0))) {
        border.setRotation(180);
        render_texture_.draw(border);
      }
      if (!IsWall (position + sf::Vector2i (0, -1))) {
        border.setRotation(270);
        render_texture_.draw(border);
      }

      // inner corner
      if (IsWall (position + sf::Vector2i (1, 0)) && IsWall(position + sf::Vector2i(0, -1))) {
        inner_corner.setRotation(0);
        render_texture_.draw(inner_corner);
      }
      if (IsWall (position + sf::Vector2i (0, 1) && IsWall (position + sf::Vector2i (1, 0))) {
        inner_corner.setRotation(90);
        render_texture_.draw(inner_corner);
      }
      if (IsWall (position + sf::Vector2i (-1, 0)) && IsWall (position + sf::Vector2i (0, 1))) {
        inner_corner.setRotation(180);
        render_texture_.draw(inner_corner);
      }
      if (IsWall (position + sf::Vector2i (0, -1)) && IsWall (position + sf::Vector2i (-1, 0))) {
        inner_corner.setRotation(270);
        render_texture_.draw(inner_corner);
      }

      // outer corner
      if (IsWall (position + sf::Vector2i (1, 0)) && IsWall (position + sf::Vector2i (0, -1))) {
        outer_corner.setRotation(0);
        render_texture_.draw(outer_corner);
      }
      if (IsWall (position + sf::Vector2i (0, 1) && IsWall (position + sf::Vector2i (1, 0))) {
        outer_corner.setRotation(90);
        render_texture_.draw(outer_corner);
      }
      if (IsWall (position + sf::Vector2i (-1, 0)) && IsWall (position + sf::Vector2i (0, 1))) {
        outer_corner.setRotation(180);
        render_texture_.draw(outer_corner);
      }
      if (IsWall (position + sf::Vector2i (0, -1)) && IsWall (position + sf::Vector2i (-1, 0))) {
        outer_corner.setRotation(270);
        render_texture_.draw(outer_corner);
      }
    }
  }

  return kSuccess;
}

sf::Vector2i Maze::PacPosition() const { return pac_position_; }

std::vector<sf::Vector2i> Maze::GhostPositions() const {
  return ghost_positions_;
}

std::size_t Maze::PositionToIndex(sf::Vector2i position) const {
  return position.y * maze_size_.x + position.x;
}

sf::Vector2i Maze::IndexToPosition(std::size_t index) const {
  sf::Vector2i position;
  position.x = index % maze_size_.x;
  position.y = index / maze_size_.x;

  return position;
}

sf::Vector2i Maze::PixelToCell(sf::Vector2i pixel) const {
  sf::Vector2i cell;
  cell.x = std::floor(pixel.x / 32.f);
  cell.y = std::floor(pixel.y / 32.f);
  return cell;
}

sf::Vector2i Maze::CellToPixel(sf::Vector2i cell) const {
  sf::Vector2i pixel;
  pixel.x = cell.x * 32 + 16;
  pixel.y = cell.y * 32 + 16;
  return pixel;
}

template <Maze::CellData kCellType_>
bool IsTile(sf::Vector2i position) {
  return maze_data_[PositionToIndex(position)] == kCellType_;
}

bool Maze::IsWall(sf::Vector2i position) const {
  if (position.x < 0 || position.y < 0 || position.x >= maze_size_.x ||
      position.y >= maze_size_.y)
    return true;
  return IsTile<kWall>(position);
}

bool Maze::IsDot(sf::Vector2i position) const { return IsTile<kDot>(position); }

bool Maze::IsSuperDot(sf::Vector2i position) const {
  return IsTile<kSuperDot>(position);
}

bool Maze::IsBonus(sf::Vector2i position) const {
  return IsTile<kBonus>(position);
}

void Maze::PickObject(sf::Vector2i position) const {
  if (!IsWall(position)) return;
  maze_data_[PositionToIndex(position)] = kEmpty;
}

sf::Vector2i Maze::Size() const { return 0; }

int Maze::DotCount() const {
  int count = 0;
  for (int i = 0; i < maze_data_.size(); ++i) {
    auto value = maze_data_[i];
    if (value == kDot || value == kSuperDot) ++count;
  }
  return count;
}

void Maze::draw(sf::RenderTarget& target, sf::RenderStates states) const {
  target.draw(sf::Sprite(render_texture_.getTexture()), states);

  sf::CircleShape dot = Dot();  //
  sf::CircleShape super_dot = SuperDot();

  for (int i = 0; i < maze_data_.size(); ++i) {
    sf::Vector2i position = IndexToPosition(i);
    auto tile = maze_data_[i];
    if (tile == kDot) {
      dot.setPosition(32 * position.x + 16, 32 * position.y + 16);
      target.draw(dot, states);
    } else if (tile == kSuperDot) {
      super_dot.setPosition(32 * position.x + 16, 32 * position.y + 16);
      target.draw(super_dot, states);
    }
  }
}
