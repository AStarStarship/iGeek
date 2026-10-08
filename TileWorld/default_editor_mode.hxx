// Copyright AStarship <https://astarship.net>.
#include "DefaultEditorMode.h"
#include "stdafx.h"

void DefaultEditorMode::InitVariables() {
  textureRect = sf::IntRect(0, 0, static_cast<int>(stateData->gridSize),
                            static_cast<int>(stateData->gridSize));
  collision = false;
  type = TileTypes::kDefault;
  layer = 0;
  tileAddLock = false;
}

void DefaultEditorMode::initGui() {
  // Text
  cursorText.setFont(*editorStateData->font);
  cursorText.setFillColor(sf::Color::White);
  cursorText.setCharacterSize(12);
  cursorText.setPosition(editorStateData->mousePosView->x,
                         editorStateData->mousePosView->y);

  // General GUI
  sidebar.setSize(sf::Vector2f(
      80.f, static_cast<float>(stateData->gfxSettings->resolution.height)));
  sidebar.setFillColor(sf::Color(50, 50, 50, 100));
  sidebar.setOutlineColor(sf::Color(200, 200, 200, 150));
  sidebar.setOutlineThickness(1.f);

  selectorRect.setSize(sf::Vector2f(stateData->gridSize, stateData->gridSize));

  selectorRect.setFillColor(sf::Color(255, 255, 255, 150));
  selectorRect.setOutlineThickness(1.f);
  selectorRect.setOutlineColor(sf::Color::Green);

  selectorRect.setTexture(tileMap->getTileSheet());
  selectorRect.setTextureRect(textureRect);

  textureSelector = new gui::TextureSelector(
      20.f, 20.f, 1000.f, 500.f, stateData->gridSize, tileMap->getTileSheet(),
      *editorStateData->font, "TS");

  // Buttons
}

DefaultEditorMode::DefaultEditorMode(StateData* state_data, TileMap* tile_map,
                                     EditorStateData* editor_state_data)
    : EditorMode(state_data, tile_map, editor_state_data) {
  InitVariables();
  initGui();
}

DefaultEditorMode::~DefaultEditorMode() { delete textureSelector; }

// Functions
void DefaultEditorMode::updateInput(const float& dt) {
  // Add a tile to the tilemap
  if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && getKeytime()) {
    if (!sidebar.getGlobalBounds().contains(
            sf::Vector2f(*editorStateData->mousePosWindow))) {
      if (!textureSelector->getActive()) {
        if (tileAddLock) {
          if (tileMap->tileEmpty(editorStateData->mousePosGrid->x,
                                 editorStateData->mousePosGrid->y, 0)) {
            tileMap->addTile(editorStateData->mousePosGrid->x,
                             editorStateData->mousePosGrid->y, 0, textureRect,
                             collision, type);
          }
        } else {
          tileMap->addTile(editorStateData->mousePosGrid->x,
                           editorStateData->mousePosGrid->y, 0, textureRect,
                           collision, type);
        }
      } else {
        textureRect = textureSelector->getTextureRect();
      }
    }
  }
  // Remove a tile from the tilemap
  else if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && getKeytime()) {
    if (!sidebar.getGlobalBounds().contains(
            sf::Vector2f(*editorStateData->mousePosWindow))) {
      if (!textureSelector->getActive())
        tileMap->removeTile(editorStateData->mousePosGrid->x,
                            editorStateData->mousePosGrid->y, 0);
    }
  }

  // Toggle collision
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(
          editorStateData->keybinds->at("TOGGLE_COLLISION"))) &&
      getKeytime()) {
    if (collision)
      collision = false;
    else
      collision = true;
  } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(
                 editorStateData->keybinds->at("INCREASE_TYPE"))) &&
             getKeytime()) {
    // CHANGE TO LIMIT TO MAX TYPE!!!!
    ++type;
  } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(
                 editorStateData->keybinds->at("DECREASE_TYPE"))) &&
             getKeytime()) {
    if (type > 0) --type;
  }

  // Set lock on / off
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(
          editorStateData->keybinds->at("TOGGLE_TILE_LOCK"))) &&
      getKeytime()) {
    if (tileAddLock)
      tileAddLock = false;
    else
      tileAddLock = true;
  }
}

void DefaultEditorMode::updateGui(const float& dt) {
  textureSelector->Update(*editorStateData->mousePosWindow, dt);

  if (!textureSelector->getActive()) {
    selectorRect.setTextureRect(textureRect);
    selectorRect.setPosition(
        editorStateData->mousePosGrid->x * stateData->gridSize,
        editorStateData->mousePosGrid->y * stateData->gridSize);
  }

  cursorText.setPosition(editorStateData->mousePosView->x + 100.f,
                         editorStateData->mousePosView->y - 50.f);

  std::stringstream ss;
  ss << editorStateData->mousePosView->x << " "
     << editorStateData->mousePosView->y << "\n"
     << editorStateData->mousePosGrid->x << " "
     << editorStateData->mousePosGrid->y << "\n"
     << textureRect.left << " " << textureRect.top << "\n"
     << "Collision: " << collision << "\n"
     << "Type: " << type << "\n"
     << "Tiles: "
     << tileMap->getLayerSize(editorStateData->mousePosGrid->x,
                              editorStateData->mousePosGrid->y, layer)
     << "\n"
     << "Tile lock: " << tileAddLock;

  cursorText.setString(ss.str());
}

void DefaultEditorMode::Update(const float& dt) {
  updateInput(dt);
  updateGui(dt);
}

void DefaultEditorMode::renderGui(sf::RenderTarget& target) {
  if (!textureSelector->getActive()) {
    target.setView(*editorStateData->view);
    target.draw(selectorRect);
  }

  target.setView(stateData->window->getDefaultView());
  textureSelector->Render(target);
  target.draw(sidebar);

  target.setView(*editorStateData->view);
  target.draw(cursorText);
}

void DefaultEditorMode::Render(sf::RenderTarget& target) { renderGui(target); }
