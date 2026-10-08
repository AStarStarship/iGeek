// Copyright AStarship <https://astarship.net>.
#include "state_settings.h"

void StateSettings::InitVariables() {
  modes_ = sf::VideoMode::getFullscreenModes();
}

void StateSettings::InitFonts() {
  if (!font_.LoadFromFile("Fonts/Dosis-Light.ttf")) {
    throw("ERROR::MAINMENUSTATE::COULD NOT LOAD FONT");
  }
}

void StateSettings::InitKeybinds() {
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

void StateSettings::InitGUI() {
  const sf::VideoMode& vm = state_data_->gfx_settings->resolution;

  // Background
  background_.setSize(sf::Vector2f(static_cast<float>(vm.width),
                                   static_cast<float>(vm.height)));

  if (!background_texture_.LoadFromFile(
          "Resources/Images/Backgrounds/bg1.png")) {
    throw "ERROR::MAIN_MENU_STATE::FAILED_TO_LOAD_BACKGROUND_TEXTURE";
  }

  background_.setTexture(&background_texture_);

  buttons_["BACK"] = new gui::Button(
      gui::p2pX(72.f, vm), gui::p2pY(81.5f, vm), gui::p2pX(13.f, vm),
      gui::p2pY(6.f, vm), &font_, "Back", gui::CalcCharSize(vm),
      sf::Color(70, 70, 70, 200), sf::Color(250, 250, 250, 250),
      sf::Color(20, 20, 20, 50), sf::Color(100, 100, 100, 0),
      sf::Color(150, 150, 150, 0), sf::Color(20, 20, 20, 0));

  buttons_["APPLY"] = new gui::Button(
      gui::p2pX(60.f, vm), gui::p2pY(81.5f, vm), gui::p2pX(13.f, vm),
      gui::p2pY(6.f, vm), &font_, "Apply", gui::CalcCharSize(vm),
      sf::Color(70, 70, 70, 200), sf::Color(250, 250, 250, 250),
      sf::Color(20, 20, 20, 50), sf::Color(100, 100, 100, 0),
      sf::Color(150, 150, 150, 0), sf::Color(20, 20, 20, 0));

  _::Array<_::AString> modes_str;
  for (auto& i : modes) {
    modes_str.Push(std::to_string(i.width) + 'x' + std::to_string(i.height));
  }

  // Drop down lists
  drop_down_lists_["RESOLUTION"] = new gui::DropDownList(
      gui::p2pX(42.f, vm), gui::p2pY(42.f, vm), gui::p2pX(10.4f, vm),
      gui::p2pY(4.5f, vm), font_, modes_str.data(), modes_str.size());

  options_text_.setFont(font_);
  options_text_.PositionSet(
      sf::Vector2f(gui::p2pX(5.2f, vm), gui::p2pY(41.7f, vm)));
  options_text_.setCharacterSize(gui::CalcCharSize(vm, 70));
  options_text_.setFillColor(sf::Color(255, 255, 255, 200));

  options_text_.setString(
      "Resolution \n\nFullscreen \n\nVsync \n\nAntialiasing \n\n ");
}

void StateSettings::ResetGUI() {
  auto it = buttons_.begin();
  for (it = buttons_.begin(); it != buttons_.end(); ++it) {
    delete it->second;
  }
  buttons_.clear();

  auto it2 = drop_down_lists_.begin();
  for (it2 = drop_down_lists_.begin(); it2 != drop_down_lists_.end(); ++it2) {
    delete it2->second;
  }
  drop_down_lists_.clear();

  InitGUI();
}

StateSettings::StateSettings(StateData* state_data) : State(state_data) {
  InitVariables();
  InitFonts();
  InitKeybinds();
  InitGUI();
}

StateSettings::~StateSettings() {
  auto it = buttons_.begin();
  for (it = buttons_.begin(); it != buttons_.end(); ++it) {
    delete it->second;
  }

  auto it2 = drop_down_lists_.begin();
  for (it2 = drop_down_lists_.begin(); it2 != drop_down_lists_.end(); ++it2) {
    delete it2->second;
  }
}

void StateSettings::UpdateInput(const float& dt) {}

void StateSettings::UpdateGUI(const float& dt) {
  for (auto& it : buttons_) {
    it.second->Update(mouse_pos_window);
  }

  if (buttons_["BACK"]->IsPressed()) {
    EndState();
  }

  if (buttons_["APPLY"]->IsPressed()) {
    state_data_->grfx_settings_->resolution =
        modes[drop_down_lists_["RESOLUTION"]->GetActiveElementId()];

    window->create(state_data_->grfx_settings_->resolution,
                   state_data_->grfx_settings_->title, sf::Style::Default);

    ResetGUI();
  }
  for (auto& it : drop_down_lists_) {
    it.second->Update(mouse_pos_window, dt);
  }
}

void StateSettings::Update(const float& dt) {
  UpdateMousePositions();
  UpdateInput(dt);

  UpdateGUI(dt);
}

void StateSettings::RenderGUI(sf::RenderTarget& target) {
  for (auto& it : buttons_) {
    it.second->Render(target);
  }

  for (auto& it : drop_down_lists_) {
    it.second->Render(target);
  }
}

void StateSettings::Render(sf::RenderTarget* target) {
  if (!target) target = window;

  target->draw(background_);

  RenderGUI(*target);

  target->draw(options_text_);

  // REMOVE LATER!!!
  sf::Text mouseText;
  mouseText.PositionSet(mouse_pos_value_.x, mouse_pos_value_.y - 50);
  mouseText.setFont(font_);
  mouseText.setCharacterSize(12);
  std::stringstream ss;
  ss << mouse_pos_value_.x << " " << mouse_pos_value_.y;
  mouseText.setString(ss.str());
  target->draw(mouseText);
}
