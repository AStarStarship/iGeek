// Copyright AStarship <https://astarship.net>.
#include "state_game.h"

void GameState::InitDeferredRender() {
  render_texture_.create(state_data_->grfx_settings_->resolution.width,
                         state_data_->grfx_settings_->resolution.height);

  render_sprite_.setTexture(render_texture_.getTexture());
  render_sprite_.setTextureRect(
      sf::IntRect(0, 0, state_data_->grfx_settings_->resolution.width,
                  state_data_->grfx_settings_->resolution.height));
}

void GameState::InitView() {
  view_.setSize(sf::Vector2f(
      static_cast<float>(state_data_->grfx_settings_->resolution.width),
      static_cast<float>(state_data_->grfx_settings_->resolution.height)));

  view_.setCenter(sf::Vector2f(
      static_cast<float>(state_data_->grfx_settings_->resolution.width) / 2.f,
      static_cast<float>(state_data_->grfx_settings_->resolution.height) /
          2.f));
}

void GameState::InitKeybinds() {
  std::ifstream ifs("Config/gamestate_keybinds.s2");

  if (ifs.is_open()) {
    _::AString key = "";
    _::AString key2 = "";

    while (ifs >> key >> key2) {
      keybinds[key] = supported_keys_->at(key2);
    }
  }

  ifs.close();
}

void GameState::InitFonts() {
  if (!font_.LoadFromFile("Fonts/Dosis-Light.ttf")) {
    throw("ERROR::MAINMENUSTATE::COULD NOT LOAD FONT");
  }
}

void GameState::initTextures() {
  if (!textures["PLAYER_SHEET"].LoadFromFile(
          "Resources/Images/Sprites/Player/PLAYER_SHEET2.png")) {
    throw "ERROR::GAME_STATE::COULD_NOT_LOAD_PLAYER_TEXTURE";
  }

  if (!textures["RAT1_SHEET"].LoadFromFile(
          "Resources/Images/Sprites/Enemy/rat1_60x64.png")) {
    throw "ERROR::GAME_STATE::COULD_NOT_LOAD_RAT1_TEXTURE";
  }
}

void GameState::InitPauseMenu() {
  const sf::VideoMode& vm = state_data_->grfx_settings_->resolution;
  pmenu_ = new PauseMenu(state_data_->grfx_settings_->resolution, font_);

  pmenu_->AddButton("QUIT", gui::p2pY(74.f, vm), gui::p2pX(13.f, vm),
                    gui::p2pY(6.f, vm), gui::CalcCharSize(vm), "Quit");
}

void GameState::InitShaders() {
  if (!core_shader_.LoadFromFile("vertex_shader.vert",
                                 "fragment_shader.frag")) {
    std::cout << "\nERROR::GAMESTATE::COULD NOT LOAD SHADER.";
  }
}

void GameState::InitPlayers() {
  player_ = new Player(220, 220, textures["PLAYER_SHEET"]);
}

void GameState::InitPlayerGUI() {
  player_gui_ = new PlayerGUI(player_, state_data_->grfx_settings_->resolution);
}

void GameState::initEnemySystem() {
  enemy_system_ = new EnemySystem(active_enemies_, textures);
}

void GameState::initTileMap() { tiles_ = new TileMap("text.slmp"); }

// Constructors / Destructors
GameState::GameState(StateData* state_data) : State(state_data) {
  InitDeferredRender();
  InitView();
  InitKeybinds();
  InitFonts();
  initTextures();
  InitPauseMenu();
  InitShaders();

  InitPlayers();
  InitPlayerGUI();
  initEnemySystem();
  initTileMap();
}

GameState::~GameState() {
  delete pmenu_;
  delete player_;
  delete player_gui_;
  delete enemy_system_;
  delete tiles_;

  for (size_t i = 0; i < active_enemies_.size(); i++) {
    delete active_enemies_[i];
  }
}

void GameState::updateView(const float& dt) {
  view_.setCenter(
      std::floor(player_->getPosition().x +
                 (static_cast<float>(mouse_pos_window.x) -
                  static_cast<float>(
                      state_data_->grfx_settings_->resolution.width / 2)) /
                     10.f),
      std::floor(player_->getPosition().y +
                 (static_cast<float>(mouse_pos_window.y) -
                  static_cast<float>(
                      state_data_->grfx_settings_->resolution.height / 2)) /
                     10.f));

  if (tiles_->getMaxSizeF().x >= view_.getSize().x) {
    if (view_.Center().x - view_.getSize().x / 2.f < 0.f) {
      view_.setCenter(0.f + view_.getSize().x / 2.f, view_.Center().y);
    } else if (view_.Center().x + view_.getSize().x / 2.f >
               tiles_->getMaxSizeF().x) {
      view_.setCenter(tiles_->getMaxSizeF().x - view_.getSize().x / 2.f,
                      view_.Center().y);
    }
  }

  if (tiles_->getMaxSizeF().y >= view_.getSize().y) {
    if (view_.Center().y - view_.getSize().y / 2.f < 0.f) {
      view_.setCenter(view_.Center().x, 0.f + view_.getSize().y / 2.f);
    } else if (view_.Center().y + view_.getSize().y / 2.f >
               tiles_->getMaxSizeF().y) {
      view_.setCenter(view_.Center().x,
                      tiles_->getMaxSizeF().y - view_.getSize().y / 2.f);
    }
  }

  view_grid_position_.x = static_cast<int>(view_.Center().x) /
                          static_cast<int>(state_data_->grid_size_);
  view_grid_position_.y = static_cast<int>(view_.Center().y) /
                          static_cast<int>(state_data_->grid_size_);
}

void GameState::UpdateInput(const float& dt) {
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(keybinds.at("CLOSE"))) &&
      KeyTime()) {
    if (!paused)
      pauseState();
    else
      unpauseState();
  }
}

void GameState::updatePlayerInput(const float& dt) {
  // Update player input
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(keybinds.at("MOVE_LEFT"))))
    player_->Move(-1.f, 0.f, dt);
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(keybinds.at("MOVE_RIGHT"))))
    player_->Move(1.f, 0.f, dt);
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(keybinds.at("MOVE_UP")))) {
    player_->Move(0.f, -1.f, dt);
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(keybinds.at("MOVE_DOWN")))) {
    player_->Move(0.f, 1.f, dt);
  }
}

void GameState::updatePlayerGUI(const float& dt) { player_gui_->Update(dt); }

void GameState::UpdatePauseMenuButtons() {
  if (pmenu_->IsButtonPressed("QUIT")) EndState();
}

void GameState::updateTileMap(const float& dt) {
  tiles_->updateWorldBoundsCollision(player_, dt);
  tiles_->updateTileCollision(player_, dt);
  tiles_->updateTiles(player_, dt, *enemy_system_);

  for (auto* i : active_enemies_) {
    tiles_->updateWorldBoundsCollision(i, dt);
    tiles_->updateTileCollision(i, dt);
  }
}

void GameState::updatePlayer(const float& dt) {}

void GameState::updateEnemies(const float& dt) {
  // active_enemies_.Push(new Rat(200.f, 100.f, textures["RAT1_SHEET"]));
}

void GameState::Update(const float& dt) {
  UpdateMousePositions(&view_);
  UpdateKeyTime(dt);
  UpdateInput(dt);

  if (!paused)  // Unpaused Update
  {
    updateView(dt);

    updatePlayerInput(dt);

    updateTileMap(dt);

    player_->Update(dt, mouse_pos_value_);

    player_gui_->Update(dt);

    for (auto* i : active_enemies_) {
      i->Update(dt, mouse_pos_value_);
    }
  } else  // Paused Update
  {
    pmenu_->Update(mouse_pos_window);
    UpdatePauseMenuButtons();
  }
}

void GameState::Render(sf::RenderTarget* target) {
  if (!target) target = window;

  render_texture_.clear();

  render_texture_.setView(view_);

  tiles_->Render(render_texture_, view_grid_position_, &core_shader_,
                 player_->Center(), false);

  for (auto* i : active_enemies_) {
    i->Render(render_texture_, &core_shader_, player_->Center(), false);
  }

  player_->Render(render_texture_, &core_shader_, player_->Center(), false);

  tiles_->RenderDeferred(render_texture_, &core_shader_, player_->Center());

  // Render GUI
  render_texture_.setView(render_texture_.getDefaultView());
  player_gui_->Render(render_texture_);

  if (paused)  // Pause menu Render
  {
    // renderTexture.setView(renderTexture.getDefaultView());
    pmenu_->Render(render_texture_);
  }

  // FINAL RENDER
  render_texture_.display();
  // renderSprite.setTexture(renderTexture.getTexture());
  target->draw(render_sprite_);
}
