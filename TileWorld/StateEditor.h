// Copyright AStarship <https://astarship.net>.
#pragma once
#include <_Config.h>

#ifndef TILEWORLD_EDITORSTATE_H
#define TILEWORLD_EDITORSTATE_H

#include "editor_modes.h"
#include "GUI.h"
#include "menu_pause.h"
#include "State.h"
#include "TileMap.h"

enum EditorModes { kModeEditorDefault = 0, kModeEditorEnemy };

class StateEditor : public State {
  EditorStateData editor_state_data_;
  sf::View view_;
  FPC camera_speed_;
  sf::Font font_;
  PauseMenu* pmenu_;
  _::ADic<_::AString, gui::Button*> buttons_;
  TileMap* tiles_;
  _::Array<ModeEditor*> modes_;
  unsigned active_mode_;

  void InitVariables();
  void initEditorStateData();
  void InitView();
  void InitFonts();
  void InitKeybinds();
  void InitPauseMenu();
  void initButtons();
  void InitGUI();
  void initTileMap();
  void initModes();

 public:
  StateEditor(StateData* state_data);
  virtual ~StateEditor();

  void UpdateInput(const FPC& dt);
  void updateEditorInput(const FPC& dt);
  void UpdateButtons();
  void UpdateGUI(const FPC& dt);
  void UpdatePauseMenuButtons();
  void UpdateModes(const FPC& dt);
  void Update(const FPC& dt);
  void RenderButtons(sf::RenderTarget& target);
  void RenderGUI(sf::RenderTarget& target);
  void RenderModes(sf::RenderTarget& target);
  void Render(sf::RenderTarget* target = nullptr);
};

#endif
