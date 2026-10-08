// Copyright AStarship <https://astarship.net>.
#include "state_game.h"
#include "stdafx.h"

void GameState::initDeferredRender() {
  renderTexture.create(stateData->gfxSettings->resolution.width,
                       stateData->gfxSettings->resolution.height);

  renderSprite.setTexture(renderTexture.getTexture());
  renderSprite.setTextureRect(
      sf::IntRect(0, 0, stateData->gfxSettings->resolution.width,
                  stateData->gfxSettings->resolution.height));
}

void GameState::initView() {
  view.setSize(sf::Vector2f(
      static_cast<float>(stateData->gfxSettings->resolution.width),
      static_cast<float>(stateData->gfxSettings->resolution.height)));

  view.setCenter(sf::Vector2f(
      static_cast<float>(stateData->gfxSettings->resolution.width) / 2.f,
      static_cast<float>(stateData->gfxSettings->resolution.height) / 2.f));
}

void GameState::InitKeybinds() {
  std::ifstream ifs("Config/gamestate_keybinds.ini");

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

void GameState::InitTextures() {
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
  const sf::VideoMode& vm = stateData->gfxSettings->resolution;
  pmenu_ = new PauseMenu(stateData->gfxSettings->resolution, font_);

  pmenu_->AddButton("QUIT", gui::p2pY(74.f, vm), gui::p2pX(13.f, vm),
                    gui::p2pY(6.f, vm), gui::calcCharSize(vm), "Quit");
}

void GameState::InitShaders() {
  if (!core_shader_.LoadFromFile("vertex_shader.vert",
                                 "fragment_shader.frag")) {
    std::cout << "ERROR::GAMESTATE::COULD NOT LOAD SHADER."
              << "\n";
  }
}

void GameState::InitPlayers() {
  player_ = new Player(220, 220, textures["PLAYER_SHEET"]);
}

void GameState::InitPlayerGUI() {
  playerGUI = new PlayerGUI(player_, stateData->gfxSettings->resolution);
}

void GameState::InitEnemySystem() {
  enemySystem = new EnemySystem(activeEnemies, textures);
}

void GameState::InitTileMap() { tileMap = new TileMap("text.slmp"); }

// Constructors / Destructors
GameState::GameState(StateData* state_data) : State(state_data) {
  InitDeferredRender();
  InitView();
  InitKeybinds();
  InitFonts();
  InitTextures();
  InitPauseMenu();
  InitShaders();

  InitPlayers();
  InitPlayerGUI();
  InitEnemySystem();
  InitTileMap();
}

GameState::~GameState() {
  delete pmenu_;
  delete player_;
  delete playerGUI;
  delete enemy_system_;
  delete tileMap;

  for (size_t i = 0; i < activeEnemies.size(); i++) {
    delete activeEnemies[i];
  }
}

void GameState::updateView(const float& dt) {
  view_.setCenter(
      std::floor(
          player_->getPosition().x +
          (static_cast<float>(mousePosWindow.x) -
           static_cast<float>(stateData->gfxSettings->resolution.width / 2)) /
              10.f),
      std::floor(
          player_->getPosition().y +
          (static_cast<float>(mousePosWindow.y) -
           static_cast<float>(stateData->gfxSettings->resolution.height / 2)) /
              10.f));

  if (tileMap->getMaxSizeF().x >= view_.getSize().x) {
    if (view_.getCenter().x - view_.getSize().x / 2.f < 0.f) {
      view_.setCenter(0.f + view_.getSize().x / 2.f, view_.getCenter().y);
    } else if (view_.getCenter().x + view_.getSize().x / 2.f >
               tileMap->getMaxSizeF().x) {
      view_.setCenter(tileMap->getMaxSizeF().x - view_.getSize().x / 2.f,
                      view_.getCenter().y);
    }
  }

  if (tileMap->getMaxSizeF().y >= view_.getSize().y) {
    if (view_.getCenter().y - view_.getSize().y / 2.f < 0.f) {
      view_.setCenter(view_.getCenter().x, 0.f + view_.getSize().y / 2.f);
    } else if (view_.getCenter().y + view_.getSize().y / 2.f >
               tileMap->getMaxSizeF().y) {
      view_.setCenter(view_.getCenter().x,
                      tileMap->getMaxSizeF().y - view_.getSize().y / 2.f);
    }
  }

  view_grid_position_.x = static_cast<int>(view_.getCenter().x) /
                          static_cast<int>(stateData->grid_size);
  view_grid_position_.y = static_cast<int>(view_.getCenter().y) /
                          static_cast<int>(stateData->grid_size);
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

void GameState::updatePlayerGUI(const float& dt) { playerGUI->Update(dt); }

void GameState::updatePauseMenuButtons() {
  if (pmenu->IsButtonPressed("QUIT")) endState();
}

void GameState::updateTileMap(const float& dt) {
  tileMap->updateWorldBoundsCollision(player_, dt);
  tileMap->updateTileCollision(player_, dt);
  tileMap->updateTiles(player_, dt, *enemy_system_);

  for (auto* i : activeEnemies) {
    tileMap->updateWorldBoundsCollision(i, dt);
    tileMap->updateTileCollision(i, dt);
  }
}

void GameState::updatePlayer(const float& dt) {}

void GameState::updateEnemies(const float& dt) {
  // activeEnemies.Push(new Rat(200.f, 100.f, textures["RAT1_SHEET"]));
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

    player_->Update(dt, mousePosView);

    playerGUI->Update(dt);

    for (auto* i : activeEnemies) {
      i->Update(dt, mousePosView);
    }
  } else  // Paused Update
  {
    pmenu_->Update(mousePosWindow);
    updatePauseMenuButtons();
  }
}

void GameState::Render(sf::RenderTarget* target) {
  if (!target) target = window;

  render_texture_.clear();

  render_texture_.setView(view_);

  tileMap->Render(render_texture_, view_grid_position_, &core_shader_,
                  player_->getCenter(), false);

  for (auto* i : activeEnemies) {
    i->Render(render_texture_, &core_shader_, player_->getCenter(), false);
  }

  player_->Render(render_texture_, &core_shader_, player_->getCenter(), false);

  tileMap->renderDeferred(render_texture_, &core_shader_, player_->getCenter());

  // Render GUI
  render_texture_.setView(render_texture_.getDefaultView());
  playerGUI->Render(render_texture_);

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
