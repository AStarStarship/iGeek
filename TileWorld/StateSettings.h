// Copyright AStarship <https://astarship.net>.
#pragma once
#include <_Config.h>

#ifndef TILEWORLD_SETTINGSSTATE
#define TILEWORLD_SETTINGSSTATE

#include "GUI.h"
#include "State.h"

class StateSettings : public State {
  sf::Texture background_texture_;
  sf::RectangleShape background_;
  sf::Font font_;

  _::ADic<_::AString, gui::Button*> buttons_;
  _::ADic<_::AString, gui::DropDownList*> drop_down_lists_;

  sf::Text options_text_;

  _::Array<sf::VideoMode> modes_;

  void InitVariables();
  void InitFonts();
  void InitKeybinds();
  void InitGUI();
  void ResetGUI();

 public:
  StateSettings(StateData* state_data);
  virtual ~StateSettings();

  void UpdateInput(const FPC& dt);
  void UpdateGUI(const FPC& dt);
  void Update(const FPC& dt);
  void RenderGUI(sf::RenderTarget& target);
  void Render(sf::RenderTarget* target = nullptr);
};

#endif
