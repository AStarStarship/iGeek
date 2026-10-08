// Copyright AStarship <https://astarship.net>.
#pragma once
#include <_Config.h>

#ifndef TILEWORLD_GAME
#define TILEWORLD_GAME

#include "state_menu_main.h"

class Game {
  GraphicsSettings grfx_settings_;  //<
  StateData state_data_;            //<
  sf::RenderWindow *window_;        //<
  sf::Event event_;                 //<
  sf::Clock time_delta_;            //<
  FPC dt,                         //<
      grid_size_;
  _::AStack<State *> states_;                //<
  _::ADic<_::AString, ISN> supported_keys_;  //<

  void InitVariables();

  void InitGraphicsSettings();

  void InitWindow();

  void InitKeys();

  void InitStateData();

  void InitStates();

 public:
  Game();

  virtual ~Game();

  void EndApplication();

  void UpdateTimeDelta();

  void UpdateEvents();

  void Update();

  void Render();

  void Run();
};

#endif
