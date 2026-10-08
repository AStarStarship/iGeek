// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef TILEWORLD_PLAYERGUI
#define TILEWORLD_PLAYERGUI
#include <_Config.h>
//
#include "GUI.h"
#include "Player.h"
class Player;
class sf::RectangleShape;
namespace TileWorld {

class PlayerGUI {
 private:
  Player* player_;

  sf::VideoMode& vm_;
  sf::Font font_;

  _::AString level_bar_string_;
  sf::Text level_bar_text_;
  sf::RectangleShape level_bar_back_;

  gui::ProgressBar *exp_bar_, *hp_bar_;

  void InitFont();
  void InitLevelBar();
  void InitExpBar();
  void InitHPBar();

 public:
  PlayerGUI(Player* player, sf::VideoMode& vm);
  virtual ~PlayerGUI();

  void UpdateLevelBar();
  void UpdateExpBar();
  void UpdateHPBar();
  void Update(const FPC& dt);

  void RenderLevelBar(sf::RenderTarget& target);
  void RenderExpBar(sf::RenderTarget& target);
  void RenderHPBar(sf::RenderTarget& target);
  void Render(sf::RenderTarget& target);
};
}  //< namespace TileWorld
#endif
