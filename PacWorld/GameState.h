// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef KABUKI_ARCADE_GAMESTATE
#define KABUKI_ARCADE_GAMESTATE
#include "SFML/Graphics.hpp"
//
#include "Game.h"
#include "Ghost.h"
#include "Maze.h"
#include "Pac.h"

namespace PacWorld {

class GameState {
  sf::RenderWindow window_;     //< Window to render too.
  PacWorld::Game& game_;  //< The main game.
 public:
  GameState(PacWorld::Game& game);

  PacWorld::Game& Game();

  int Run();

  virtual void InsertCoin();

  virtual void PressButton();

  virtual void MoveStick(sf::Vector2i direction);

  virtual void HandleInput() = 0;

  virtual void Update(sf::Time delta) = 0;

  virtual void Draw(sf::RenderWindow& window) = 0;
};

class StateInsertCoin : public GameState {
  bool draw_text_;     //< Flag for if the text_ is drawn.
  sf::Text text_;      //< Insert coin text.
  sf::Sprite sprite_;  //< Foo.

 public:
  StateInsertCoin(Game& game);

  void PressButton();

  virtual void HandleInput();

  virtual void Update(sf::Time delta);

  virtual void Draw(sf::RenderWindow& window);
};

class StateGetReady : public GameState {
  sf::Text text_;  //<
  GameState& playing_state_;

 public:
  StateGetReady(Game& game, GameState& playing);

  void PressButton();

  virtual void HandleInput();

  virtual void Update(sf::Time delta);

  virtual void draw(sf::RenderWindow& window);
};

class StatePlaying : public GameState {
  Pac pac_;
  std::vector<Ghost*> ghosts_;
  Maze maze_;
  sf::View camera_;
  sf::RenderTexture scene_;
  sf::Text score_text_, level_text_, dots_left_text_;
  sf::Sprite lives_sprite_[3];
  int level_, speed_, life_count_, score_;

 public:
  StatePlaying(Game& game);

  ~StatePlaying();

  void Clear();

  void ResetToZero();

  void ResetCurrentLevel();

  void ResetLifeCount();

  void ResetLevel();

  void InsertCoin();

  void PressButton();

  void MoveStick(sf::Vector2i direction);

  void HandleInput();

  void Update(sf::Time delta);

  void Draw(sf::RenderWindow& window);

  void LoadNextLevel();

  void ResetCharacters();

  void UpdateCameraPosition();
};

class StateWon : public GameState {
  sf::Text text_;  //< Insert coin text.
  StatePlaying& state_playing_;

 public:
  StateWon(Game& game, StatePlaying& state_playing);

  virtual void HandleInput();

  virtual void Update(sf::Time delta);

  virtual void draw(sf::RenderWindow& window);
};

class StateLost : public GameState {
  sf::Text text_, count_down_, count_down_text_;
  StatePlaying& state_playing_;

 public:
  StateLost(Game& game, StatePlaying& state_playing);

  void InsertCoin();

  virtual void HandleInput();

  virtual void Update(sf::Time delta);

  virtual void draw(sf::RenderWindow& window);
};

class StateError : public GameState {
 public:
  StateError(Game& game);

  virtual void HandleInput();

  virtual void Update(sf::Time delta);

  virtual void draw(sf::RenderWindow& window);
};
}  //< namespace arcade
}  //< namespace kabuki
#endif
