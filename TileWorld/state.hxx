// Copyright AStarship <https://astarship.net>.
#include "state.h"

State::State(StateData* state_data) {
  state_data_ = state_data;
  window_ = state_data->window_;
  supported_keys_ = state_data->supported_keys_;
  states_ = state_data->states_;
  quit_ = false;
  paused_ = false;
  keytime_ = 0.f;
  key_press_time_max_ = 10.f;
  size_grid_ = state_data->grid_size_;
}

State::~State() {}

const bool& State::GetQuit() const { return quit_; }

const bool State::KeyTime() {
  if (keytime_ >= key_press_time_max_) {
    keytime_ = 0.f;
    return true;
  }

  return false;
}

void State::EndState() { quit_ = true; }

void State::PauseState() { paused_ = true; }

void State::UnpauseState() { paused_ = false; }

void State::UpdateMousePositions(sf::View* view) {
  mouse_pos_screen_ = sf::Mouse::getPosition();
  mouse_pos_window_ = sf::Mouse::getPosition(*window_);

  if (view) window_->setView(*view);

  mouse_pos_view_ = window_->mapPixelToCoords(sf::Mouse::getPosition(*window_));
  mouse_pos_grid_ = sf::Vector2i(
      static_cast<int>(mouse_pos_view_.x) / static_cast<int>(size_grid_),
      static_cast<int>(mouse_pos_view_.y) / static_cast<int>(size_grid_));

  window_->setView(window_->getDefaultView());
}

void State::UpdateKeyTime(const float& dt) {
  if (keytime_ < key_press_time_max_) keytime_ += 100.f * dt;
}
