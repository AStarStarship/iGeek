// Copyright AStarship <https://astarship.net>.
#ifndef TILEWORLD_ENEMYEDITORMODE
#define TILEWORLD_ENEMYEDITORMODE

#include "TileSpawner"
#include "ModeEditor.h"

class State;
class StateData;
class EditorMode;
class TileMap;
class Tile;
class EnemySpawner;

class EnemyEditorMode : public EditorMode {
 private:
  sf::Text cursor_text_;
  sf::RectangleShape sidebar_;
  sf::RectangleShape selector_rect_;
  sf::IntRect texture_rect_;

  ISN type_;
  ISN amount_;
  ISN time_to_spawn_;
  FPC distance_max_;

  void InitVariables();
  void initGui();

 public:
  EnemyEditorMode(StateData* state_data, TileMap* tile_map,
                  EditorStateData* editor_state_data);
  virtual ~EnemyEditorMode();

  // Functions
  void updateInput(const FPC& dt);
  void updateGui(const FPC& dt);
  void Update(const FPC& dt);

  void renderGui(sf::RenderTarget& target);
  void Render(sf::RenderTarget& target);
};

#endif  //! ENEMYEDITORMODE_H
