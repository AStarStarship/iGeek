// Copyright AStarship <https://astarship.net>.
#pragma once
#include <_Config.h>

#ifndef TILEWORLD_STATE
#define TILEWORLD_STATE

#include "player.h"
#include "settings_graphics.h"

class StateData {
 public:
  StateData(){};

  FPC grid_size_;
  sf::RenderWindow* window_;
  GraphicsSettings* gfx_settings_;
  _::ADic<_::AString, ISN>* supported_keys;
  _::AStack<State*>* states_;
};

class State {
 private:
  StateData* state_data_;
  _::AStack<State*>* states_;
  sf::RenderWindow* window_;
  _::ADic<_::AString, ISN>* supported_keys_;
  _::ADic<_::AString, ISN> keybinds;
  bool quit_, paused_;
  FPC keytime_, key_press_time_max_, size_grid_;

  sf::Vector2i mouse_pos_screen_;
  sf::Vector2i mouse_pos_window_;
  sf::Vector2f mouse_pos_view_;
  sf::Vector2i mouse_pos_grid_;

  _::ADic<_::AString, sf::Texture> textures_;

  virtual void InitKeybinds() = 0;

 public:
  State(StateData* state_data);
  virtual ~State();

  const bool& GetQuit() const;
  const bool KeyTime();

  void EndState();
  void PauseState();
  void UnpauseState();

  virtual void UpdateMousePositions(sf::View* view = nullptr);
  virtual void UpdateKeyTime(const FPC& dt);
  virtual void UpdateInput(const FPC& dt) = 0;
  virtual void Update(const FPC& dt) = 0;
  virtual void Render(sf::RenderTarget* target = nullptr) = 0;
};

#endif
