// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef PACWORLD_GAME
#define PACWORLD_GAME

#include "GameState.h"

namespace PacWorld {

/* The headless game. Owns the state objects and the current state.
   `Update(delta)` delegates to the current state. `Run()` is stubbed —
   the browser/display layer provides the main loop (headless-first
   design). No SFML, no stdlib. */
class Game {
  GameState* state_;  // current state.
 public:
  StateInsertCoin state_insert_coin;
  StateGetReady state_get_ready;
  StatePlaying state_playing;
  StateWon state_won;
  StateLost state_lost;

  enum State {
    kStateInsertCoin = 0,
    kStateGetReady,
    kStatePlaying,
    kStateWon,
    kStateLost,
  };

  Game();

  /* The main loop — stubbed for headless. The browser/display layer
     drives Update() + Draw() from its own event loop. */
  int Run();

  void ChangeState(GameState& next_state);

  /* Advance the current state by `delta` seconds. */
  void Update(FPC delta);

  /* Draw the current state — stubbed for headless (CImage later). */
  void Draw();

  /* Route an input event to the current state. */
  void InsertCoin();
  void PressButton();
  void MoveStick(ISC2 direction);
};
}  // namespace PacWorld
#endif
