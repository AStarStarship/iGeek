// Copyright AStarship <https://astarship.net>.
#pragma once
#include <_Config.h>

#ifndef TILEWORLD_GAMESTATE
#define TILEWORLD_GAMESTATE

#include "Bow.h"
#include "menu_pause.h"
#include "player_gui.h"
#include "State.h"
#include "sword.h"
#include "TileMap.h"

class GameState : public State {
  sf::View view_;
  sf::Vector2i view_grid_position_;
  sf::RenderTexture render_texture_;
  sf::Sprite render_sprite_;
  sf::Font font_;
  PauseMenu* pmenu_;
  sf::Shader core_shader_;
  Player* player_;
  PlayerGUI* player_gui_;
  sf::Texture texture;
  _::Array<Enemy*> active_enemies_;
  EnemySystem* enemy_system_;
  TileMap* tiles_;

  void InitDeferredRender();
  void InitView();
  void InitKeybinds();
  void InitFonts();
  void initTextures();
  void InitPauseMenu();
  void InitShaders();
  void InitPlayers();
  void InitPlayerGUI();
  void initEnemySystem();
  void initTileMap();

 public:
  GameState(StateData* state_data);
  virtual ~GameState();

  void updateView(const FPC& dt);
  void UpdateInput(const FPC& dt);
  void updatePlayerInput(const FPC& dt);
  void updatePlayerGUI(const FPC& dt);
  void UpdatePauseMenuButtons();
  void updateTileMap(const FPC& dt);
  void updatePlayer(const FPC& dt);
  void updateEnemies(const FPC& dt);
  void Update(const FPC& dt);
  void Render(sf::RenderTarget* target = nullptr);
};

#endif
