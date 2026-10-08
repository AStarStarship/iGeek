// Copyright AStarship <https://astarship.net>.
#pragma once
#include <_Config.h>

#ifndef MAINMENUSTATE_H
#define MAINMENUSTATE_H

#include "GUI.h"
#include "state_editor.h"
#include "state_game.h"
#include "state_settings.h"

class StateMenuMain : public State {
  sf::Texture background_texture_;
  sf::RectangleShape background_;
  sf::Font font_;

  sf::RectangleShape bttn_background_;
  _::ADic<_::AString, gui::Button*> buttons_;

  void InitVariables();
  void InitFonts();
  void InitKeybinds();
  void InitGUI();
  void ResetGUI();

 public:
  StateMenuMain(StateData* state_data);
  virtual ~StateMenuMain();

  void UpdateInput(const FPC& dt);
  void UpdateButtons();
  void Update(const FPC& dt);
  void RenderButtons(sf::RenderTarget& target);
  void Render(sf::RenderTarget* target = nullptr);
};

#endif
