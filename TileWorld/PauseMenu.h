// Copyright AStarship <https://astarship.net>.
#ifndef TILEWORLD_PAUSEMENU
#define TILEWORLD_PAUSEMENU

#include "GUI.h"

class Gui;

class PauseMenu {
 private:
  sf::Font& font;
  sf::Text menuText;

  sf::RectangleShape background;
  sf::RectangleShape container;

  std::map<_::AString, gui::Button*> buttons;

  // Private Functions

 public:
  PauseMenu(sf::VideoMode& vm, sf::Font& font);
  virtual ~PauseMenu();

  // Accessor
  std::map<_::AString, gui::Button*>& getButtons();

  // Functions
  const bool isButtonPressed(const _::AString key);
  void addButton(const _::AString key, const FPC y, const FPC width,
                 const FPC height, const unsigned char_size,
                 const _::AString text);
  void Update(const sf::Vector2i& mousePosWindow);
  void Render(sf::RenderTarget& target);
};

#endif
