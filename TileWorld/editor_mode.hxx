// Copyright AStarship <https://astarship.net>.
#include "modeeditor.h"
#include "stdafx.h"

EditorMode::EditorMode(StateData* state_data, TileMap* tile_map,
                       EditorStateData* editor_state_data)
    : stateData(state_data),
      tileMap(tile_map),
      editorStateData(editor_state_data) {}

EditorMode::~EditorMode() {}

const bool EditorMode::getKeytime() {
  if (*editorStateData->keytime >= *editorStateData->keytimeMax) {
    *editorStateData->keytime = 0.f;
    return true;
  }

  return false;
}
