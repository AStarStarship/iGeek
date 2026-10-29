// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_GAME_HXX
#define IGEEK_GAME_HXX
#include "Game.h"

using namespace PacWorld;

Game::Game()
    : state_(NILP),
      state_insert_coin(this),
      state_get_ready(this, &state_playing),
      state_playing(this),
      state_won(this, &state_playing),
      state_lost(this, &state_playing) {
  ChangeState(state_insert_coin);
}

int Game::Run() {
  // Stubbed for headless. The browser/display layer drives the main loop:
  //   while (running) {
  //     Update(delta);
  //     Draw();
  //     HandleInput(events);
  //   }
  // For now, run a few update ticks (headless test mode).
  for (ISC i = 0; i < 60; ++i) {
    Update(1.f / 60.f);
  }
  return 0;
}

void Game::ChangeState(GameState& next_state) { state_ = &next_state; }

void Game::Update(FPC delta) {
  if (state_) state_->Update(delta);
}

void Game::Draw() {
  if (state_) state_->Draw();
}

void Game::InsertCoin() {
  if (state_) state_->InsertCoin();
}

void Game::PressButton() {
  if (state_) state_->PressButton();
}

void Game::MoveStick(ISC2 direction) {
  if (state_) state_->MoveStick(direction);
}

#endif  // IGEEK_GAME_HXX
