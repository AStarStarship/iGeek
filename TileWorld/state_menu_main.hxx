// Copyright AStarship <https://astarship.net>.
#include "state_menu_main.h"

void StateMenuMain::InitVariables() {}

void StateMenuMain::InitFonts() {
  if (!font_.LoadFromFile("Fonts/Dosis-Light.ttf")) {
    throw("ERROR::MAINMENUSTATE::COULD NOT LOAD FONT");
  }
}

void StateMenuMain::InitKeybinds() {
  std::ifstream ifs("Config/mainmenustate_keybinds.s2");

  if (ifs.is_open()) {
    _::AString key = "";
    _::AString key2 = "";

    while (ifs >> key >> key2) {
      keybinds[key] = supported_keys_->at(key2);
    }
  }

  ifs.close();
}

void StateMenuMain::InitGUI() {
  const sf::VideoMode& vm = state_data_->gfx_settings->resolution;

  // Background
  background_.setSize(sf::Vector2f(static_cast<float>(vm.width),
                                   static_cast<float>(vm.height)));

  if (!background_texture_.LoadFromFile(
          "Resources/Images/Backgrounds/bg2.png")) {
    throw "ERROR::MAIN_MENU_STATE::FAILED_TO_LOAD_BACKGROUND_TEXTURE";
  }

  background_.setTexture(&background_texture_);

  bttn_background_.setSize(sf::Vector2f(static_cast<float>(vm.width / 5),
                                        static_cast<float>(vm.height)));

  bttn_background_.PositionSet(gui::p2pX(11.5f, vm), 0.f);
  bttn_background_.setFillColor(sf::Color(10, 10, 10, 220));

  buttons_["GAME_STATE"] = new gui::Button(
      gui::p2pX(15.6f, vm), gui::p2pY(30.f, vm), gui::p2pX(13.f, vm),
      gui::p2pY(6.f, vm), &font_, "New Game", gui::CalcCharSize(vm),
      sf::Color(200, 200, 200, 200), sf::Color(255, 255, 255, 255),
      sf::Color(20, 20, 20, 50), sf::Color(70, 70, 70, 0),
      sf::Color(150, 150, 150, 0), sf::Color(20, 20, 20, 0));

  buttons_["SETTINGS_STATE"] = new gui::Button(
      gui::p2pX(15.6f, vm), gui::p2pY(40.f, vm), gui::p2pX(13.f, vm),
      gui::p2pY(6.f, vm), &font_, "Settings", gui::CalcCharSize(vm),
      sf::Color(200, 200, 200, 200), sf::Color(255, 255, 255, 255),
      sf::Color(20, 20, 20, 50), sf::Color(70, 70, 70, 0),
      sf::Color(150, 150, 150, 0), sf::Color(20, 20, 20, 0));

  buttons_["EDITOR_STATE"] = new gui::Button(
      gui::p2pX(15.6f, vm), gui::p2pY(50.f, vm), gui::p2pX(13.f, vm),
      gui::p2pY(6.f, vm), &font_, "Editor", gui::CalcCharSize(vm),
      sf::Color(200, 200, 200, 200), sf::Color(255, 255, 255, 255),
      sf::Color(20, 20, 20, 50), sf::Color(70, 70, 70, 0),
      sf::Color(150, 150, 150, 0), sf::Color(20, 20, 20, 0));

  buttons_["EXIT_STATE"] = new gui::Button(
      gui::p2pX(15.6f, vm), gui::p2pY(65.f, vm), gui::p2pX(13.f, vm),
      gui::p2pY(6.f, vm), &font_, "Quit", gui::CalcCharSize(vm),
      sf::Color(200, 200, 200, 200), sf::Color(255, 255, 255, 255),
      sf::Color(20, 20, 20, 50), sf::Color(100, 100, 100, 0),
      sf::Color(150, 150, 150, 0), sf::Color(20, 20, 20, 0));
}

void StateMenuMain::ResetGUI() {
  auto it = buttons_.begin();
  for (it = buttons_.begin(); it != buttons_.end(); ++it) {
    delete it->second;
  }
  buttons_.clear();

  InitGUI();
}

StateMenuMain::StateMenuMain(StateData* state_data) : State(state_data) {
  InitVariables();
  InitFonts();
  InitKeybinds();
  InitGUI();
  ResetGUI();
}

StateMenuMain::~StateMenuMain() {
  auto it = buttons_.begin();
  for (it = buttons_.begin(); it != buttons_.end(); ++it) {
    delete it->second;
  }
}

void StateMenuMain::UpdateInput(const float& dt) {}

void StateMenuMain::UpdateButtons() {
  for (auto& it : buttons_) {
    it.second->Update(mouse_pos_window_);
  }

  if (buttons_["GAME_STATE"]->IsPressed()) {
    states_->push(new GameState(state_data_));
  }
  if (buttons_["SETTINGS_STATE"]->IsPressed()) {
    states_->push(new StateSettings(state_data_));
  }
  if (buttons_["EDITOR_STATE"]->IsPressed()) {
    states_->push(new StateEditor(state_data_));
  }

  if (buttons_["EXIT_STATE"]->IsPressed()) {
    EndState();
  }
}

void StateMenuMain::Update(const float& dt) {
  UpdateMousePositions();
  UpdateInput(dt);

  UpdateButtons();
}

void StateMenuMain::RenderButtons(sf::RenderTarget& target) {
  for (auto& it : buttons_) {
    it.second->Render(target);
  }
}

void StateMenuMain::Render(sf::RenderTarget* target) {
  if (!target) target = window_;

  target->draw(background_);

  target->draw(bttn_background_);

  RenderButtons(*target);
}
