// Copyright AStarship <https://astarship.net>.
#include "game.h"
//
#include <iostream>

using namespace kabuki::arcade;

Game::Game()
    : state_insert_coin(*this),
      state_playing(*this, state_playing),
      state_won(*this),
      state_lost(*this),
      state_(state_insert_coin),
      window_(sf::VideoMode(640, 500), "Pac-Ze") {
  if (!font_.loadFromFile("resources/arcade_classic.ttf")) {
    std::cout << "\nError loading font!";
    return;
  };

  if (!logo_.loadFromFile("resources/logo.png")) return;

  if (!texture_.loadFromFile("resources/texture.png")) return;

  ChangeState(state_insert_coin);
}

void Game::ChangeState(GameState& next_state) { state_ = next_state; }

sf::Font& Game::Font() { return font_; }

sf::Texture& Game::Logo() { return logo_; }

sf::Texture& Game::Texture() { return texture_; }

int Game::Run() {
  sf::Clock frame_clock;

  int style = sf::Fullscreen;

  sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Window", style);

  auto fullscreen_modes = sf::VideoMode::getFullscreenModes();

  for (auto& video_mode : fullscreen_modes) {
    std::cout << "\nsize:" << video_mode.width << "& " << video_mode.height
              << " (bpp" << video_mode.bitsPerPixel;
  }

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape ||
            event.type == sf::Event::Closed) {
          window.close();
        }

        if (event.key.code == sf::Keyboard::I) {
          state_.InsertCoin();
        }

        if (event.key.code == sf::Keyboard::R) {
          state_.PressButton();
        }

        if (event.key.code == sf::Keyboard::W) {
          state_.MoveStick(sf::Vector2i(0, 1));
        }

        if (event.key.code == sf::Keyboard::S) {
          state_.MoveStick(sf::Vector2i(0, -1));
        }

        if (event.key.code == sf::Keyboard::A) {
          state_.MoveStick(sf::Vector2i(1, 0));
        }

        if (event.key.code == sf::Keyboard::D) {
          state_.MoveStick(sf::Vector2i(-1, 0));
        }
      }
      state_->Update(frame_clock.restart());
      window.clear();
      state_->Draw(window);
      window.display();

      window.setFramerateLimit(60);
      sf::sleep(sf::seconds(1.f / 60.f));
    }
  }
  return EXIT_SUCCESS;
}
