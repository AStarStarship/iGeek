// Copyright AStarship <https://astarship.net>.
#ifndef TILEWORLD_DEFAULTEDITORMODE_DECL
#define TILEWORLD_DEFAULTEDITORMODE_DECL
#include <_Config.h>
//
#include "ModeEditor.h"
namespace TileWorld {

class State;
class StateData;
class EditorMode;
class TileMap;
class Tile;
class DefaultEditorMode : public EditorMode {
 private:
  sf::Text cursorText;
  sf::RectangleShape sidebar;
  sf::RectangleShape selectorRect;
  gui::TextureSelector* textureSelector;
  sf::IntRect textureRect;
  bool collision;
  short type;
  ISN layer;
  bool tileAddLock;

  void InitVariables();
  void initGui();

 public:
  DefaultEditorMode(StateData* state_data, TileMap* tile_map,
                    EditorStateData* editor_state_data);
  virtual ~DefaultEditorMode();

  void UpdateInput(const FPC& dt);
  void UpdateGui(const FPC& dt);
  void Update(const FPC& dt);

  void RenderGui(sf::RenderTarget& target);
  void Render(sf::RenderTarget& target);
};
}  //< namespace TileWorld
#endif
