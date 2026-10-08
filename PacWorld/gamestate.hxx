// Copyright AStarship <https://astarship.net>.
#include "gamestate.h"
//
#include <iostream>

using namespace kabuki::arcade;

GameState::GameState(kabuki::arcade::Game& game) : game_(game) {}

void GameState::InsertCoin() { std::cout << "\nInserted coin."; }

void GameState::PressButton() { std::cout << "\nPressedButton coin."; }

void GameState::MoveStick(sf::Vector2i direction) {
  std::cout << "\nMoved stick x:" << direction.x << " y:" << direction.y;
}

kabuki::arcade::Game& GameState::Game() { return game_; }

int GameState::Run() {
  int style = sf::Fullscreen;

  sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Window", style);

  auto fullscreen_modes = sf::VideoMode::getFullscreenModes();

  for (auto& videoMode : fullscreen_modes) {
    std::cout << "\nsize:" << videoMode.width << "& " << videoMode.height
              << " (bpp" << videoMode.bitsPerPixel;
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
          game_.InsertCoin();
        }

        if (event.key.code == sf::Keyboard::R) {
          game_.PressButton();
        }

        if (event.key.code == sf::Keyboard::W) {
          game_.MoveStick(sf::Vector2i(0, 1));
        }

        if (event.key.code == sf::Keyboard::S) {
          game_.MoveStick(sf::Vector2i(0, -1));
        }

        if (event.key.code == sf::Keyboard::A) {
          game_.MoveStick(sf::Vector2i(1, 0));
        }

        if (event.key.code == sf::Keyboard::D) {
          game_.MoveStick(sf::Vector2i(-1, 0));
        }
      }
      game_.Update(sf::seconds(1));
      window.clear();
      game_.draw(window);
      window.display();

      window.setFramerateLimit(60);
      sf::sleep(sf::seconds(1.f / 60.f));
    }
  }
  return EXIT_SUCCESS;
}

StateInsertCoin::StateInsertCoin(Game& game) : GameState(game) {
  sprite_.setTexture(game.Logo());
  sprite_.setPosition(20, 50);

  text_.setFont(game.Font());
  text_.setString("Insert coin");

  CenterOrigin(text_);
  text_.setPosition(240, 150);
}

void StateInsertCoin::PressButton() {}

void StateInsertCoin::HandleInput() {}

void StateInsertCoin::Update(sf::Time delta) {
  static sf::Time time_buffer = sf::Time::Zero;
  time_buffer += delta;

  while (time_buffer >= sf::seconds(0.5)) {
    bool draw_text = draw_text_;
    draw_text_ = !draw_text;
    time_buffer -= sf::seconds(1);
  }
}

void StateInsertCoin::draw(sf::RenderWindow& window) {
  window.draw(sprite_);

  if (draw_text_) {
    window.draw(text_);
  }
}

StateGetReady::StateGetReady(Game& game, GameState& playing)
    : GameState(game), playing_state_(playing) {
  text_.setFont(game.Font());
  text_.setString("Press Space when you're ready...");
  text_.setCharacterSize(14);

  CenterOrigin(text_);
  text_.setPosition(240, 240);
}

void StateGetReady::PressButton() {
  GameState::Game().ChangeState(GameState::Game().state_playing);
}

void StateGetReady::HandleInput() {}

void StateGetReady::Update(sf::Time delta) { playing_state_.Update(delta); }

void StateGetReady::draw(sf::RenderWindow& window) {
  playing_state_.draw(window);
  window.draw(text_);
}

StatePlaying::StatePlaying(Game& game)
    : GameState(game),
      pac_(game.Texture()),
      maze_(game.Texture()),
      level_(0),
      speed_(0),
      life_count_(3),
      score_(0) {
  Game& game = GameState::Game();
  // pac_.Move(100, 100);
  // ghost_.Move(200, 200);
  // pac_ = new Pac(game.Texture());
  pac_.SetMaze(&maze_);
  pac_.setPosition(maze_.CellToPixel(maze_.PacPosition()));

  ResetToZero();

  camera_.setSize(sf::Vector2f(480, 480));
  scene_.create(480, 480);

  score_text_.setFont(game.Font());
  score_text_.setCharacterSize(10);
  score_text_.setPosition(10, 480);

  level_text_.setFont(game.Font());
  level_text_.setCharacterSize(10);
  level_text_.setPosition(160, 480);

  dots_left_text_.setFont(game.Font());
  dots_left_text_.setCharacterSize(10);
  dots_left_text_.setPosition(280, 480);

  for (auto& live_sprite : lives_sprite_) {
    live_sprite.setTexture(game.Texture());
    live_sprite.setTextureRect(sf::IntRect(122, 0, 20, 20));
  }
  lives_sprite_[0].setPosition(sf::Vector2f(415, 480));
  lives_sprite_[1].setPosition(sf::Vector2f(435, 480));
  lives_sprite_[2].setPosition(sf::Vector2f(455, 480));
}

StatePlaying::~StatePlaying() { Clear(); }

void StatePlaying::Clear() {
  for (Ghost* ghost : ghosts_) delete ghost;
  ghosts_.clear();
}

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
  float speed = 100 + (speed_ * 50);

  pac_.SetSpeed(speed + 25);

  for (auto ghost_position : maze_.GhostPositions()) {
    Ghost* ghost = new Ghost(Game().Texture(), &pac_);
    ghost->SetSpeed(speed);
    ghost->SetMaze(&maze_);
    ghost->setPosition(maze_.CellToPixel(ghost_position));

    ghosts_.push_back(ghost);
  }

  level_text_.setString("level " + std::to_string(speed_) + " - " +
                        std::to_string(level_ + 1));
}

void StatePlaying::InsertCoin() {
  // pac_.Die();
}

void StatePlaying::PressButton() {
  // ghost.SetWeak(sf::seconds(3));
}

void StatePlaying::HandleInput() {}

void StatePlaying::MoveStick(sf::Vector2i direction) {}

void StatePlaying::Update(sf::Time delta) {
  pac_.Update(delta);
  for (Ghost* ghost : ghosts_) ghost->Update(delta);

  sf::Vector2f pixel_position = pac_.Position();
  sf::Vector2f offset(std::fmod(pixel_position.x, 32),
                      std::fmod(pixel_position.y, 32));
  offset -= sf::Vector2f(16, 16);

  if (offset.x <= 2 && offset.x >= -2 && offset.y <= 2 && offset.y >= -2) {
    sf::Vector2i cell_position = maze_.PixelToCell(pixel_position);

    if (maze_.IsDot(cell_position)) {
      score_ += 5;
    } else if (maze_.IsSuperDot(cell_position)) {
      score_ += 25;
      for (Ghost* ghost : ghosts_) ghost->SetWeak(sf::seconds(5));
    } else if (maze_.IsBonus(cell_position)) {
      score_ += 500;
    }

    maze_.PickObject(cell_position);
  }

  for (Ghost* ghost : ghosts_) {
    if (ghost->CollisionBox().intersects(pac_.CollisionBox())) {
      if (ghost->IsWeak()) {
        score_ +=
            100 ghosts_.erase(std::find(ghosts_.begin(), ghosts_.end(), ghost));
      } else {
        pac_.Die();
      }
    }
  }
  if (pac_.IsDead()) {
    --life_count_;
    pac_.Reset();
    if (life_count_ < 0) {
      Game().ChangeState(Game().state_lost);
    } else {
      ResetCharacters();
    }
  }

  if (maze_.DotCount() == 0) Game().ChangeState(Game().state_won);
  UpdateCameraPosition();

  score_text_.setString(std::to_string(score_) + " points");
  dots_left_text_.setString(std::to_string(maze_.DotCount() + "x dots"));
}

void StatePlaying::draw(sf::RenderWindow& window) {
  scene_.setView(camera_);
  scene_.draw(maze_);
  scene_.draw(pac_);
  for (Ghost* ghost : ghosts_) scene_.draw(ghost);

  scene_.display();

  window.draw(sf::Sprite(scene_.getTexture()));

  window.draw(score_text_);
  window.draw(level_text_);
  window.draw(dots_left_text_);

  for (int i = 0; i < life_count_; ++i) {
    window.draw(lives_sprite_[i]);
  }
}

void StatePlaying::LoadNextLevel() {
  ++level_;
  int map_level = level_ % 3, speel_level = level_ / 3;
  const char* filename = nullptr;
  switch (map_level) {
    case 0:
      filename = "resources/levels/small.png";
      break;
    case 1:
      filename = "resources/levels/medium.png";
      break;
    case 2:
      filename = "resources/levels/large.png";
      break;
  }
  Clear();
  maze_.LoadLevel(filename);
}

void StatePlaying::ResetCharacters() {
  pac_.setPosition(maze_.CellToPixel(maze_.PacPosition()));
  auto ghost_positions = maze_.GhostPositions();
  for (int i = 0; i < ghosts_.size(); ++i)
    ghosts_[i]->setPosition(maze_.CellToPixel(ghost_positions[i]));
  UpdateCameraPosition();
}

void StatePlaying::UpdateCameraPosition() {
  camera_.setCenter(pac_.getPosition());

  if (camera_.getCenter().x < 240)
    camera_.setCenter(240, camera_.getCenter().y);
  if (camera_.getCenter().y < 240)
    camera_.setCenter(camera_.getCenter().x, 240);

  if (camera_.getCenter().x > maze_.Size().x * 32 - 240)
    camera_.setCenter(maze_.Size().x * 32 - 240, camera_.getCenter().y);
  if (camera_.getCenter().y > maze_.Size().y * 32 - 240)
    camera_.setCenter(camera_.getCenter().x, maze_.Size().y * 32 - 240);
}

StateWon::StateWon(Game& game, StatePlaying& state_playing)
    : GameState(game), state_playing_(state_playing) {
  text_.setFont(GameState::Game().Font());
  text_.setString("You won!");
  text_.setCharacterSize(42);
  CenterOrigin(text_);
  text_.setPosition(240, 120);
}
void StateWon::HandleInput() {}

void StateWon::Update(sf::Time delta) {
  static sf::Time time_buffer = sf::Time::Zero;
  time_buffer += delta;

  if (time_buffer.asSeconds() > 5) {
    state_playing_.LoadNextLevel();
    Game().ChangeState(Game().state_get_ready);
  }
}

void StateWon::draw(sf::RenderWindow& window) { window.draw(text_); }

StateLost::StateLost(Game& game, StatePlaying& state_playing)
    : GameState(game), state_playing_(state_playing) {
  text_.setFont(game.Font());
  text_.setString("You lost!");
  text_.setCharacterSize(42);
  CenterOrigin(text_);
  text_.setPosition(240, 120);

  count_down_text_.setFont(game.Font());
  count_down_text_.setCharacterSize(12);
  CenterOrigin(count_down_text_);
  count_down_text_.setPosition(240, 240);
}

void StateLost::InsertCoin() {
  state_playing_.ResetLifeCount();
  state_playing_.ResetLevel();
  Game().ChangeState(Game().state_get_ready);
}

void StateLost::HandleInput() {}

void StateLost::Update(sf::Time delta) {
  count_down_ += delta;

  if (count_down_.asSeconds() >= 10) {
    state_playing_.ResetCurrentLevel();
    Game().ChangeState(game_.state_insert_coin);
  }

  count_down_text_.setString(
      "Insert a coin to continue... " +
      std::to_string(10 - static_cast<int>(count_down_.asSeconds())));
}

void StateLost::draw(sf::RenderWindow& window) {}

StateError::StateError(Game& game) : GameState(game) {}

void StateError::HandleInput() {}

void StateError::Update(sf::Time delta) {}

void StateError::draw(sf::RenderWindow& window) {}
