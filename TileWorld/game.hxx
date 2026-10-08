// Copyright AStarship <https://astarship.net>.
#include "game.h"

void Game::InitVariables() {
  window_ = nullptr;

  dt = 0.f;

  grid_size_ = 64.f;
}

void Game::InitGraphicsSettings() {
  grfx_settings_.LoadFromFile("Config/graphics.s2");
}

void Game::InitWindow() {
  if (grfx_settings_.fullscreen)
    window_ = new sf::RenderWindow(grfx_settings_.resolution,
                                   grfx_settings_.title, sf::Style::Fullscreen,
                                   grfx_settings_.context_settings);
  else
    window_ =
        new sf::RenderWindow(grfx_settings_.resolution, grfx_settings_.title,
                             sf::Style::Titlebar | sf::Style::Close,
                             grfx_settings_.context_settings);

  window_->setFramerateLimit(grfx_settings_.frame_rate_limit);
  window_->setVerticalSyncEnabled(grfx_settings_.vertical_sync);
}

void Game::InitKeys() {
  std::ifstream ifs("Config/supported_keys.s2");

  if (ifs.is_open()) {
    _::AString key = "";
    int key_value = 0;

    while (ifs >> key >> key_value) {
      supported_keys_[key] = key_value;
    }
  }

  ifs.close();

#if DEBUG_THIS
  for (auto i : supported_keys_) {
    std::cout << i.first << " " << i.second << "\n";
  }
#endif
}

void Game::InitStateData() {
  state_data_.window_ = window_;
  state_data_.gfx_settings_ = &grfx_settings_;
  state_data_.supported_keys_ = &supported_keys_;
  state_data_.states_ = &states_;
  state_data_.grid_size_ = grid_size_;
}

void Game::InitStates() { states_.Push(new StateMenuMain(&state_data_)); }

Game::Game() {
  InitVariables();
  InitGraphicsSettings();
  InitWindow();
  InitKeys();
  InitStateData();
  InitStates();
}

Game::~Game() {
  delete window_;

  while (!states_.Empty()) {
    delete states_.Top();
    states_.Pop();
  }
}

void Game::EndApplication() { std::cout << "\nEnding Application."; }

void Game::UpdateTimeDelta() { dt = time_delta_.restart().asSeconds(); }

void Game::UpdateEvents() {
  while (window_->pollEvent(event_)) {
    if (event_.type == sf::Event::Closed) window_->close();
  }
}

void Game::Update() {
  UpdateEvents();
  if (!states_.IsEmpty()) {
    if (window_->HasFocus()) {
      states_.Top()->Update(dt);

      if (states_.Top()->GetQuit()) {
        states_.Top()->EndState();
        delete states_.Top();
        states_.pop();
      }
    }
  } else {
    EndApplication();
    window_->close();
  }
}

void Game::Render() {
  window_->clear();
  if (!states_.empty()) states_.Top()->Render();
  window_->display();
}

void Game::Run() {
  while (window_->isOpen()) {
    UpdateTimeDelta();
    Update();
    Render();
  }
}
