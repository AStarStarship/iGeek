// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef KABUKI_ARCADE_MAZE
#define KABUKI_ARCADE_MAZE
namespace PacWorld {

class Maze : public sf::Drawable {
  enum CellData {
    kEmpty = 0,
    kWall,
    kDot,
    kSuperDot,
    kBonus,
  };

  sf::Vector2i maze_size_;
  std::vector<CellData> maze_data_;
  sf::Vector2i pac_position_;
  std::vector<sf::Vector2i> ghost_positions_;
  sf::RenderTexture render_texture_;
  sf::Texture& texture_;

  void draw(sf::RenderTarget& target, sf::RenderStates states) const;

 public:
  enum Error {
    kInvalidFile,
    kTooSmall,
  };

  Maze(sf::Texture& texture);

  sf::Vector2i Size() const;

  /* Loads a level from an image with the given name.
  @return 0 upon success or -1 upon failure. */
  Error LoadLevel(std::string name);

  sf::Vector2i PacPosition() const;

  std::vector<sf::Vector2i> GhostPositions() const;

  inline std::size_t PositionToIndex(sf::Vector2i position) const;

  inline sf::Vector2i IndexToPosition(std::size_t index) const;

  sf::Vector2i PixelToCell(sf::Vector2i pixel) const;

  sf::Vector2i CellToPixel(sf::Vector2i cell) const;

  bool IsWall(sf::Vector2i position) const;

  bool IsDot(sf::Vector2i position) const;

  bool IsSuperDot(sf::Vector2i position) const;

  bool IsBonus(sf::Vector2i position) const;

  void PickObject(sf::Vector2i position) const;

  sf::Vector2i Size() const;

  int DotCount() const;
};
}  //< namespace PacWorld
#endif
