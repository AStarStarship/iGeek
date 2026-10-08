// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef TILEWORLD_EDITORMODE_DECL
#define TILEWORLD_EDITORMODE_DECL
#include <_Config.h>
//
#include "GUI.h"
#include "State.h"
#include "TileMap.h"
namespace TileWorld {

class EditorStateData {
 public:
  EditorStateData(){};

  sf::View* view;

  sf::Font* font;

  FPC *key_press_time_,  //<
      *key_press_time_max_;

  _::ADic<_::AString, ISN>* keybinds;

  sf::Vector2i *mouse_pos_screen,  //<
      *mouse_pos_window,           //<
      *mouse_pos_view,             //<
      *mouse_pos_grid;             //<
};

class ModeEditor {
 protected:
  StateData* state_data_;
  EditorStateData* editor_state_data_;
  TileMap* tiles_;

 public:
  ModeEditor(StateData* state_data, TileMap* tiles,
             EditorStateData* editor_state_data);
  virtual ~ModeEditor();

  const bool KeyTime();

  virtual void UpdateInput(const FPC& dt) = 0;
  virtual void UpdateGUI(const FPC& dt) = 0;
  virtual void Update(const FPC& dt) = 0;

  virtual void RenderGUI(sf::RenderTarget& target) = 0;
  virtual void Render(sf::RenderTarget& target) = 0;
};
}  //< namespace TileWorld
#endif
