// Copyright AStarship <https://astarship.net>.
#pragma once
#include <_Config.h>

#ifndef TILEWORLD_PAUSEMENU_H #pragma once
#define TILEWORLD_PAUSEMENU_H

#include "GUI.h"

class PauseMenu {
  sf::Font& font;
  sf::Text menuText;

  sf::RectangleShape background, container;

  _::ADic<_::AString, gui::Button*> buttons;

 public:
  PauseMenu(sf::VideoMode& vm, sf::Font& font);
  virtual ~PauseMenu();

  _::ADic<_::AString, gui::Button*>& getButtons();

  const bool IsButtonPressed(const _::AString key);
  void AddButton(const _::AString key, const FPC y, const FPC width,
                 const FPC height, const unsigned char_size,
                 const _::AString text);
  void Update(const sf::Vector2i& mouse_pos_window);
  void Render(sf::RenderTarget& target);
};

#endif
