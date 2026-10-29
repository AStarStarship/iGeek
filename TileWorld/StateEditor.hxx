// Copyright AStarship <https://astarship.net>.
#include "state_editor.h"

void StateEditor::InitVariables() { camera_speed_ = 1000.f; }

void StateEditor::initEditorStateData() {
  editor_state_data_.view_ = &view_;
  editor_state_data_.font_ = &font_;
  editor_state_data_.keytime_ = &keytime_;
  editor_state_data_.key_press_time_max_ = &key_press_time_max_;
  editor_state_data_.keybinds = &keybinds;
  editor_state_data_.mouse_pos_grid = &mouse_pos_grid;
  editor_state_data_.mouse_pos_screen = &mouse_pos_screen;
  editor_state_data_.mouse_pos_value_ = &mouse_pos_value_;
  editor_state_data_.mouse_pos_window = &mouse_pos_window;
}

void StateEditor::InitView() {
  view_.setSize(sf::Vector2f(
      static_cast<float>(state_data_->grfx_settings_->resolution.width),
      static_cast<float>(state_data_->grfx_settings_->resolution.height)));

  view_.setCenter(
      static_cast<float>(state_data_->grfx_settings_->resolution.width) / 2.f,
      static_cast<float>(state_data_->grfx_settings_->resolution.height) / 2.f);
}

void StateEditor::InitFonts() {
  if (!font_.LoadFromFile("Fonts/Dosis-Light.ttf")) {
    throw("ERROR::EDITORSTATE::COULD NOT LOAD FONT");
  }
}

void StateEditor::InitKeybinds() {
  std::ifstream ifs("Config/editorstate_keybinds.s2");

  if (ifs.is_open()) {
    _::AString key = "";
    _::AString key2 = "";

    while (ifs >> key >> key2) {
      keybinds[key] = supported_keys_->at(key2);
    }
  }

  ifs.close();
}

void StateEditor::InitPauseMenu() {
  const sf::VideoMode& vm = state_data_->grfx_settings_->resolution;
  pmenu = new PauseMenu(state_data_->grfx_settings_->resolution, font_);

  pmenu->AddButton("QUIT", gui::p2pY(74.f, vm), gui::p2pX(13.f, vm),
                   gui::p2pY(6.f, vm), gui::CalcCharSize(vm), "Quit");
  pmenu->AddButton("SAVE", gui::p2pY(46.f, vm), gui::p2pX(13.f, vm),
                   gui::p2pY(6.f, vm), gui::CalcCharSize(vm), "Save");
  pmenu->AddButton("LOAD", gui::p2pY(37.f, vm), gui::p2pX(13.f, vm),
                   gui::p2pY(6.f, vm), gui::CalcCharSize(vm), "Load");
}

void StateEditor::initButtons() {}

void StateEditor::InitGUI() {}

void StateEditor::initTileMap() {
  tiles_ = new TileMap(state_data_->grid_size_, 10, 10,
                       "Resources/Images/Tiles/tilesheet3.png");
}

void StateEditor::initModes() {
  modes.Push(new DefaultEditorMode(state_data_, tiles_, &editor_state_data_));
  modes.Push(new ModeEditorEnemy(state_data_, tiles_, &editor_state_data_));

  active_mode_ = EditorModes::kModeEditorDefault;
}

StateEditor::StateEditor(StateData* state_data) : State(state_data) {
  InitVariables();
  initEditorStateData();
  InitView();
  InitFonts();
  InitKeybinds();
  InitPauseMenu();
  initButtons();
  initTileMap();
  InitGUI();
  initModes();
}

StateEditor::~StateEditor() {
  auto it = buttons.begin();
  for (it = buttons.begin(); it != buttons.end(); ++it) {
    delete it->second;
  }

  delete pmenu;

  delete tiles_;

  for (size_t i = 0; i < modes.size(); i++) {
    delete modes[i];
  }
}

void StateEditor::UpdateInput(const float& dt) {
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(keybinds.at("CLOSE"))) &&
      KeyTime()) {
    if (!paused)
      pauseState();
    else
      unpauseState();
  }
}

void StateEditor::updateEditorInput(const float& dt) {
  if (sf::Keyboard::isKeyPressed(
          sf::Keyboard::Key(keybinds.at("MOVE_CAMERA_UP")))) {
    view_.move(0.f, -camera_speed_ * dt);
  } else if (sf::Keyboard::isKeyPressed(
                 sf::Keyboard::Key(keybinds.at("MOVE_CAMERA_DOWN")))) {
    view_.move(0.f, camera_speed_ * dt);
  }
  if (sf::Keyboard::isKeyPressed(
          sf::Keyboard::Key(keybinds.at("MOVE_CAMERA_LEFT")))) {
    view_.move(-camera_speed_ * dt, 0.f);
  } else if (sf::Keyboard::isKeyPressed(
                 sf::Keyboard::Key(keybinds.at("MOVE_CAMERA_RIGHT")))) {
    view_.move(camera_speed_ * dt, 0.f);
  }

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(keybinds.at("MODE_UP")))) {
    if (active_mode_ < modes.size() - 1) {
      active_mode_++;
    } else {
      std::cout << "ERROR::EDITORSTATE::CANNOT CHANGE MODE UP!"
                << "\n";
    }
  } else if (sf::Keyboard::isKeyPressed(
                 sf::Keyboard::Key(keybinds.at("MODE_DOWN")))) {
    if (active_mode_ > 0) {
      active_mode_--;
    } else {
      std::cout << "ERROR::EDITORSTATE::CANNOT CHANGE MODE DOWN!"
                << "\n";
    }
  }
}

void StateEditor::UpdateButtons() {
  for (auto& it : buttons) {
    it.second->Update(mouse_pos_window);
  }
}

void StateEditor::UpdateGUI(const float& dt) {}

void StateEditor::UpdatePauseMenuButtons() {
  if (pmenu->IsButtonPressed("QUIT")) EndState();

  if (pmenu->IsButtonPressed("SAVE")) tiles_->SaveToFile("text.slmp");

  if (pmenu->IsButtonPressed("LOAD")) tiles_->LoadFromFile("text.slmp");
}

void StateEditor::UpdateModes(const float& dt) {
  modes[active_mode_]->Update(dt);
}

void StateEditor::Update(const float& dt) {
  UpdateMousePositions(&view_);
  UpdateKeyTime(dt);
  UpdateInput(dt);

  if (!paused) {
    UpdateButtons();
    UpdateGUI(dt);
    updateEditorInput(dt);
    UpdateModes(dt);
  } else {
    pmenu->Update(mouse_pos_window);
    UpdatePauseMenuButtons();
  }
}

void StateEditor::RenderButtons(sf::RenderTarget& target) {
  for (auto& it : buttons) {
    it.second->Render(target);
  }
}

void StateEditor::RenderGUI(sf::RenderTarget& target) {}

void StateEditor::RenderModes(sf::RenderTarget& target) {
  modes[active_mode_]->Render(target);
}

void StateEditor::Render(sf::RenderTarget* target) {
  if (!target) target = window;

  target->setView(view_);
  tiles_->Render(*target, mouse_pos_grid, nullptr, sf::Vector2f(), true);
  tiles_->RenderDeferred(*target);

  target->setView(window->getDefaultView());
  RenderButtons(*target);

  RenderGUI(*target);

  RenderModes(*target);

  if (paused) {
    target->setView(window->getDefaultView());
    pmenu->Render(*target);
  }
}
