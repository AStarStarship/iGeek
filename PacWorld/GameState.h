// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef PACWORLD_GAMESTATE
#define PACWORLD_GAMESTATE

#include "Maze.h"
#include "Pac.h"
#include "Ghost.h"

namespace PacWorld {

class Game;

/* A headless game state. The state machine: each state handles Update
   (logic), and optionally InsertCoin/PressButton/MoveStick (input).
   Draw is stubbed — the browser/display layer provides rendering (CImage).
   No SFML, no stdlib. */
class GameState {
  Game* game_;  // the game that owns this state.

 public:
  GameState(Game* game);

  Game* GameRef() const { return game_; }

  virtual ~GameState() {}

  virtual void InsertCoin() {}
  virtual void PressButton() {}
  virtual void MoveStick(ISC2 direction) { (void)direction; }

  /* Advance this state by `delta` seconds (pure logic). */
  virtual void Update(FPC delta) { (void)delta; }

  /* Draw — stubbed for headless (CImage later). */
  virtual void Draw() {}
};

/* Wait for the player to insert a coin. */
class StateInsertCoin : public GameState {
  BOL blink_;
  FPC blink_timer_;

 public:
  StateInsertCoin(Game* game);

  void PressButton() override;
  void Update(FPC delta) override;
  void Draw() override;
};

/* 3-2-1 countdown before play. */
class StateGetReady : public GameState {
  GameState* playing_state_;
  FPC countdown_;

 public:
  StateGetReady(Game* game, GameState* playing);

  void PressButton() override;
  void Update(FPC delta) override;
  void Draw() override;
};

/* The playing state — holds the pac, ghosts, maze, and game counters.
   This is where the real game logic lives. */
class StatePlaying : public GameState {
  Pac pac_;
  Ghost ghosts_[8];  // fixed array (was std::vector<Ghost*> + new/delete).
  ISC ghost_count_;
  Maze maze_;
  FPC2 camera_;  // camera center (was sf::View).

  ISC level_;
  ISC speed_;
  ISC life_count_;
  ISC score_;

 public:
  StatePlaying(Game* game);

  void Clear();
  void ResetToZero();
  void ResetCurrentLevel();
  void ResetLifeCount();
  void ResetLevel();

  void InsertCoin() override;
  void PressButton() override;
  void MoveStick(ISC2 direction) override;
  void Update(FPC delta) override;
  void Draw() override;

  void LoadNextLevel();
  void ResetCharacters();
  void UpdateCameraPosition();

  /* Accessors (for testing + the display layer). */
  ISC Score() const { return score_; }
  ISC LifeCount() const { return life_count_; }
  ISC Level() const { return level_; }
  ISC DotsLeft() const { return maze_.DotCount(); }
};

/* Won state — advance to next level after a delay. */
class StateWon : public GameState {
  StatePlaying* state_playing_;
  FPC delay_;

 public:
  StateWon(Game* game, StatePlaying* state_playing);

  void Update(FPC delta) override;
  void Draw() override;
};

/* Lost state — wait for a coin to continue. */
class StateLost : public GameState {
  StatePlaying* state_playing_;
  FPC count_down_;

 public:
  StateLost(Game* game, StatePlaying* state_playing);

  void InsertCoin() override;
  void Update(FPC delta) override;
  void Draw() override;
};

/* Error state. */
class StateError : public GameState {
 public:
  StateError(Game* game);

  void Update(FPC delta) override;
  void Draw() override;
};
}  // namespace PacWorld
#endif
