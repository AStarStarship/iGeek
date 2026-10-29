// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_GAMESTATE_HXX
#define IGEEK_GAMESTATE_HXX
#include "GameState.h"
#include "Game.h"

using namespace PacWorld;

/* --- GameState base --- */

GameState::GameState(Game* game) : game_(game) {}

/* --- StateInsertCoin --- */

StateInsertCoin::StateInsertCoin(Game* game)
    : GameState(game), blink_(false), blink_timer_(0.f) {}

void StateInsertCoin::PressButton() {
  GameRef()->ChangeState(GameRef()->state_get_ready);
}

void StateInsertCoin::Update(FPC delta) {
  blink_timer_ += delta;
  if (blink_timer_ >= 0.5f) {
    blink_ = !blink_;
    blink_timer_ -= 0.5f;
  }
}

void StateInsertCoin::Draw() {
  // (text/sprite rendering is a display concern — stubbed for headless)
}

/* --- StateGetReady --- */

StateGetReady::StateGetReady(Game* game, GameState* playing)
    : GameState(game), playing_state_(playing), countdown_(3.0f) {}

void StateGetReady::PressButton() {
  GameRef()->ChangeState(GameRef()->state_playing);
}

void StateGetReady::Update(FPC delta) {
  countdown_ -= delta;
  playing_state_->Update(delta);
  if (countdown_ <= 0.f) {
    GameRef()->ChangeState(GameRef()->state_playing);
  }
}

void StateGetReady::Draw() {
  // (text + playing-state rendering is a display concern — stubbed)
  playing_state_->Draw();
}

/* --- StatePlaying --- */

StatePlaying::StatePlaying(Game* game)
    : GameState(game),
      ghost_count_(0),
      camera_(0.f, 0.f),
      level_(0),
      speed_(0),
      life_count_(3),
      score_(0) {
  pac_.SetMaze(&maze_);
  ResetToZero();
}

void StatePlaying::Clear() { ghost_count_ = 0; }

void StatePlaying::ResetToZero() {
  ResetLifeCount();
  level_ = 0;
  score_ = 0;
  ResetCurrentLevel();
}

void StatePlaying::ResetCurrentLevel() {
  --level_;
  LoadNextLevel();
}

void StatePlaying::ResetLifeCount() { life_count_ = 3; }

void StatePlaying::ResetLevel() {
  FPC speed = 100.f + FPC(speed_) * 50.f;

  pac_.SetSpeed(speed + 25.f);

  ghost_count_ = maze_.GhostCount();
  for (ISC i = 0; i < ghost_count_; ++i) {
    ghosts_[i].SetPac(&pac_);
    ghosts_[i].SetSpeed(speed);
    ghosts_[i].SetMaze(&maze_);
    ghosts_[i].SetPosition(maze_.CellToPixel(maze_.GhostPosition(i)));
  }
}

void StatePlaying::InsertCoin() {
  // (coin insertion logic — no-op for now)
}

void StatePlaying::PressButton() {
  // (button press — no-op for now)
}

void StatePlaying::MoveStick(ISC2 direction) {
  pac_.SetDirection(direction);
}

void StatePlaying::Update(FPC delta) {
  pac_.Update(delta);
  for (ISC i = 0; i < ghost_count_; ++i) ghosts_[i].Update(delta);

  // Dot/superdot/bonus collection: check if pac is at a cell center.
  FPC2 pixel_position = pac_.Position();
  FPC offset_x = (pixel_position.x - FPC(ISC(pixel_position.x) / 32) * 32.f) - 16.f;
  FPC offset_y = (pixel_position.y - FPC(ISC(pixel_position.y) / 32) * 32.f) - 16.f;

  if (offset_x <= 2.f && offset_x >= -2.f && offset_y <= 2.f && offset_y >= -2.f) {
    ISC2 cell_position = maze_.PixelToCell(pixel_position);

    if (maze_.IsDot(cell_position)) {
      score_ += 5;
    } else if (maze_.IsSuperDot(cell_position)) {
      score_ += 25;
      for (ISC i = 0; i < ghost_count_; ++i) ghosts_[i].SetWeak(5.f);
    } else if (maze_.IsBonus(cell_position)) {
      score_ += 500;
    }

    maze_.PickObject(cell_position);
  }

  // Ghost collision.
  for (ISC i = 0; i < ghost_count_; ++i) {
    if (ghosts_[i].CollisionBox().Intersects(pac_.CollisionBox())) {
      if (ghosts_[i].IsWeak()) {
        score_ += 100;
        --ghost_count_;
        // Move the last ghost into this slot (swap-remove, no std::find).
        if (i < ghost_count_) ghosts_[i] = ghosts_[ghost_count_];
      } else {
        pac_.Die();
      }
    }
  }

  // Death handling.
  if (pac_.IsDead()) {
    --life_count_;
    pac_.Reset();
    if (life_count_ < 0) {
      GameRef()->ChangeState(GameRef()->state_lost);
    } else {
      ResetCharacters();
    }
  }

  // Win condition: all dots collected.
  if (maze_.DotCount() == 0) {
    GameRef()->ChangeState(GameRef()->state_won);
  }

  UpdateCameraPosition();
}

void StatePlaying::Draw() {
  // (maze/pac/ghost/HUD rendering is a display concern — stubbed for headless)
}

void StatePlaying::LoadNextLevel() {
  ++level_;
  // Data-driven: the level grid is loaded from a raw buffer. For now, load
  // a default level (the display/data layer provides the actual grids).
  // maze_.LoadLevel(level_data, w, h);
}

void StatePlaying::ResetCharacters() {
  pac_.SetPosition(maze_.CellToPixel(maze_.PacPosition()));
  for (ISC i = 0; i < ghost_count_; ++i) {
    ghosts_[i].SetPosition(maze_.CellToPixel(maze_.GhostPosition(i)));
  }
  UpdateCameraPosition();
}

void StatePlaying::UpdateCameraPosition() {
  camera_ = pac_.Position();

  FPC maze_w = FPC(maze_.Size().x) * 32.f;
  FPC maze_h = FPC(maze_.Size().y) * 32.f;

  if (camera_.x < 240.f) camera_.x = 240.f;
  if (camera_.y < 240.f) camera_.y = 240.f;
  if (camera_.x > maze_w - 240.f) camera_.x = maze_w - 240.f;
  if (camera_.y > maze_h - 240.f) camera_.y = maze_h - 240.f;
}

/* --- StateWon --- */

StateWon::StateWon(Game* game, StatePlaying* state_playing)
    : GameState(game), state_playing_(state_playing), delay_(0.f) {}

void StateWon::Update(FPC delta) {
  delay_ += delta;
  if (delay_ > 5.f) {
    state_playing_->LoadNextLevel();
    GameRef()->ChangeState(GameRef()->state_get_ready);
  }
}

void StateWon::Draw() {
  // (text rendering is a display concern — stubbed for headless)
}

/* --- StateLost --- */

StateLost::StateLost(Game* game, StatePlaying* state_playing)
    : GameState(game), state_playing_(state_playing), count_down_(0.f) {}

void StateLost::InsertCoin() {
  state_playing_->ResetLifeCount();
  state_playing_->ResetLevel();
  GameRef()->ChangeState(GameRef()->state_get_ready);
}

void StateLost::Update(FPC delta) {
  count_down_ += delta;
  if (count_down_ >= 10.f) {
    state_playing_->ResetCurrentLevel();
    GameRef()->ChangeState(GameRef()->state_insert_coin);
  }
}

void StateLost::Draw() {
  // (text rendering is a display concern — stubbed for headless)
}

/* --- StateError --- */

StateError::StateError(Game* game) : GameState(game) {}

void StateError::Update(FPC delta) { (void)delta; }

void StateError::Draw() {}

#endif  // IGEEK_GAMESTATE_HXX
