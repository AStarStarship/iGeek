// Copyright AStarship <https://astarship.net>.
#include "stdafx.h"
//
#include "mode_editor_enemy.h"

void ModeEditorEnemy::InitVariables() {
  type_ = 0;
  amount_ = 1;
  time_to_spawn_ = 60;
  distance_max_ = 1000.f;
}

// Initializer functions
void ModeEditorEnemy::InitGUI() {
  // Text
  cursor_text_.setFont(*editor_state_data_->font);
  cursor_text_.setFillColor(sf::Color::White);
  cursor_text_.setCharacterSize(12);
  cursor_text_.PositionSet(editor_state_data_->mouse_pos_value_->x,
                           editor_state_data_->mouse_pos_value_->y);

  sidebar_.setSize(sf::Vector2f(
      80.f,
      static_cast<float>(state_data_->grfx_settings_->resolution.height)));
  sidebar_.setFillColor(sf::Color(50, 50, 50, 100));
  sidebar_.setOutlineColor(sf::Color(200, 200, 200, 150));
  sidebar_.setOutlineThickness(1.f);

  selector_rect_.setSize(
      sf::Vector2f(state_data_->grid_size_, state_data_->grid_size_));

  selector_rect_.setFillColor(sf::Color(255, 255, 255, 150));
  selector_rect_.setOutlineThickness(1.f);
  selector_rect_.setOutlineColor(sf::Color::Green);
}

// Constructors/Destructors
ModeEditorEnemy::ModeEditorEnemy(StateData* state_data, TileMap* tiles,
                                 EditorStateData* editor_state_data)
    : ModeEditor(state_data, tiles, editor_state_data) {
  InitVariables();
  InitGUI();
}

ModeEditorEnemy::~ModeEditorEnemy() {}

void ModeEditorEnemy::UpdateInput(const float& dt) {
  if (sf::Mouse::IsButtonPressed(sf::Mouse::Left) && KeyTime()) {
    if (!sidebar_.GlobalBounds().contains(
            sf::Vector2f(*editor_state_data_->mouse_pos_window))) {
      tiles_->AddTile(editor_state_data_->mouse_pos_grid->x,
                      editor_state_data_->mouse_pos_grid->y, 0, texture_rect_,
                      type_, amount_, time_to_spawn_, distance_max_);
    }
  } else if (sf::Mouse::IsButtonPressed(sf::Mouse::Right) && KeyTime()) {
    if (!sidebar_.GlobalBounds().contains(
            sf::Vector2f(*editor_state_data_->mouse_pos_window))) {
      tiles_->RemoveTile(editor_state_data_->mouse_pos_grid->x,
                         editor_state_data_->mouse_pos_grid->y, 0,
                         TileTypes::kSpawnerEnemy);
    }
  }

  if (sf::Keyboard::isKeyPressed(
          sf::Keyboard::Key(editor_state_data_->keybinds->at("TYPE_UP"))) &&
      KeyTime()) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
      if (type_ > 0) type_--;
    } else if (type_ < 1000)
      type_++;
    else
      type_ = 0;
  } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(
                 editor_state_data_->keybinds->at("AMOUNT_UP"))) &&
             KeyTime()) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
      if (amount_ > 0) amount_--;
    } else if (amount_ < 1000)
      amount_++;
    else
      amount_ = 0;
  } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(
                 editor_state_data_->keybinds->at("TTS_UP"))) &&
             KeyTime()) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
      if (time_to_spawn_ > 0) time_to_spawn_--;
    } else if (time_to_spawn_ < 1000)
      time_to_spawn_++;
    else
      time_to_spawn_ = 0;
  } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(
                 editor_state_data_->keybinds->at("MD_UP"))) &&
             KeyTime()) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
      if (distance_max_ > 0) distance_max_--;
    } else if (distance_max_ < 1000)
      distance_max_++;
    else
      distance_max_ = 0;
  }
}

void ModeEditorEnemy::UpdateGUI(const float& dt) {
  selector_rect_.PositionSet(
      editor_state_data_->mouse_pos_grid->x * state_data_->grid_size_,
      editor_state_data_->mouse_pos_grid->y * state_data_->grid_size_);

  cursor_text_.PositionSet(editor_state_data_->mouse_pos_value_->x + 100.f,
                           editor_state_data_->mouse_pos_value_->y - 50.f);

  std::stringstream ss;
  ss << "\nEnemy type: " << type_ << "\nEnemy amount: " << amount_
     << "\nTime to spawn: " << time_to_spawn_
     << "\nMax distance: " << distance_max_;

  cursor_text_.setString(ss.str());
}

void ModeEditorEnemy::Update(const float& dt) {
  UpdateInput(dt);
  UpdateGUI(dt);
}

void ModeEditorEnemy::RenderGUI(sf::RenderTarget& target) {
  target.setView(*editor_state_data_->view);
  target.draw(selector_rect_);
  target.draw(cursor_text_);

  target.setView(target.getDefaultView());
  target.draw(sidebar_);
}

void ModeEditorEnemy::Render(sf::RenderTarget& target) { RenderGUI(target); }
