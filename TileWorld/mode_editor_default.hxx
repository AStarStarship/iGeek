// Copyright AStarship <https://astarship.net>.
#include "mode_editor_default.h"

void DefaultEditorMode::InitVariables() {
  textureRect = sf::IntRect(0, 0, static_cast<int>(state_data_->grid_size),
                            static_cast<int>(state_data_->grid_size));
  collision = false;
  type = TileTypes::DEFAULT;
  layer = 0;
  tileAddLock = false;
}

void DefaultEditorMode::InitGUI() {
  // Text
  cursorText.setFont(*editor_state_data_->font);
  cursorText.setFillColor(sf::Color::White);
  cursorText.setCharacterSize(12);
  cursorText.SetPosition(editor_state_data_->mouse_pos_value_->x,
                         editor_state_data_->mouse_pos_value_->y);

  // General GUI
  sidebar.setSize(sf::Vector2f(
      80.f, static_cast<float>(state_data_->gfxSettings->resolution.height)));
  sidebar.setFillColor(sf::Color(50, 50, 50, 100));
  sidebar.setOutlineColor(sf::Color(200, 200, 200, 150));
  sidebar.setOutlineThickness(1.f);

  selectorRect.setSize(
      sf::Vector2f(state_data_->grid_size, state_data_->grid_size));

  selectorRect.setFillColor(sf::Color(255, 255, 255, 150));
  selectorRect.setOutlineThickness(1.f);
  selectorRect.setOutlineColor(sf::Color::Green);

  selectorRect.setTexture(tiles_->TileSheet());
  selectorRect.setTextureRect(textureRect);

  textureSelector = new gui::TextureSelector(
      20.f, 20.f, 1000.f, 500.f, state_data_->grid_size, tiles_->TileSheet(),
      *editor_state_data_->font, "TS");

  // Buttons
}

DefaultEditorMode::DefaultEditorMode(StateData* state_data, TileMap* tiles,
                                     EditorStateData* editor_state_data)
    : ModeEditor(state_data, tiles, editor_state_data) {
  InitVariables();
  InitGUI();
}

DefaultEditorMode::~DefaultEditorMode() { delete textureSelector; }

void DefaultEditorMode::UpdateInput(const float& dt) {
  // Add a tile to the tilemap
  if (sf::Mouse::IsButtonPressed(sf::Mouse::Left) && KeyTime()) {
    if (!sidebar.GlobalBounds().contains(
            sf::Vector2f(*editor_state_data_->mouse_pos_window))) {
      if (!textureSelector->getActive()) {
        if (tileAddLock) {
          if (tiles_->TileEmpty(editor_state_data_->mouse_pos_grid->x,
                                editor_state_data_->mouse_pos_grid->y, 0)) {
            tiles_->AddTile(editor_state_data_->mouse_pos_grid->x,
                            editor_state_data_->mouse_pos_grid->y, 0,
                            textureRect, collision, type);
          }
        } else {
          tiles_->AddTile(editor_state_data_->mouse_pos_grid->x,
                          editor_state_data_->mouse_pos_grid->y, 0, textureRect,
                          collision, type);
        }
      } else {
        textureRect = textureSelector->getTextureRect();
      }
    }
  }
  // Remove a tile from the tilemap
  else if (sf::Mouse::IsButtonPressed(sf::Mouse::Right) && KeyTime()) {
    if (!sidebar.GlobalBounds().contains(
            sf::Vector2f(*editor_state_data_->mouse_pos_window))) {
      if (!textureSelector->getActive())
        tiles_->RemoveTile(editor_state_data_->mouse_pos_grid->x,
                           editor_state_data_->mouse_pos_grid->y, 0);
    }
  }

  // Toggle collision
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(
          editor_state_data_->keybinds->at("TOGGLE_COLLISION"))) &&
      KeyTime()) {
    if (collision)
      collision = false;
    else
      collision = true;
  } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(
                 editor_state_data_->keybinds->at("INCREASE_TYPE"))) &&
             KeyTime()) {
    // CHANGE TO LIMIT TO MAX TYPE!!!!
    ++type;
  } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(
                 editor_state_data_->keybinds->at("DECREASE_TYPE"))) &&
             KeyTime()) {
    if (type > 0) --type;
  }

  // Set lock on / off
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(
          editor_state_data_->keybinds->at("TOGGLE_TILE_LOCK"))) &&
      KeyTime()) {
    if (tileAddLock)
      tileAddLock = false;
    else
      tileAddLock = true;
  }
}

void DefaultEditorMode::UpdateGUI(const float& dt) {
  textureSelector->Update(*editor_state_data_->mouse_pos_window, dt);

  if (!textureSelector->getActive()) {
    selectorRect.setTextureRect(textureRect);
    selectorRect.SetPosition(
        editor_state_data_->mouse_pos_grid->x * state_data_->grid_size,
        editor_state_data_->mouse_pos_grid->y * state_data_->grid_size);
  }

  cursorText.SetPosition(editor_state_data_->mouse_pos_value_->x + 100.f,
                         editor_state_data_->mouse_pos_value_->y - 50.f);

  std::stringstream ss;
  ss << editor_state_data_->mouse_pos_value_->x << " "
     << editor_state_data_->mouse_pos_value_->y << "\n"
     << editor_state_data_->mouse_pos_grid->x << " "
     << editor_state_data_->mouse_pos_grid->y << "\n"
     << textureRect.left << " " << textureRect.Top << "\n"
     << "Collision: " << collision << "\n"
     << "Type: " << type << "\n"
     << "Tiles: "
     << tiles_->LayerSize(editor_state_data_->mouse_pos_grid->x,
                          editor_state_data_->mouse_pos_grid->y, layer)
     << "\n"
     << "Tile lock: " << tileAddLock;

  cursorText.setString(ss.str());
}

void DefaultEditorMode::Update(const float& dt) {
  UpdateInput(dt);
  UpdateGUI(dt);
}

void DefaultEditorMode::RenderGUI(sf::RenderTarget& target) {
  if (!textureSelector->getActive()) {
    target.setView(*editor_state_data_->view);
    target.draw(selectorRect);
  }

  target.setView(state_data_->window->getDefaultView());
  textureSelector->Render(target);
  target.draw(sidebar);

  target.setView(*editor_state_data_->view);
  target.draw(cursorText);
}

void DefaultEditorMode::Render(sf::RenderTarget& target) { RenderGUI(target); }
