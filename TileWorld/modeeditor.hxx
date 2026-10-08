// Copyright AStarship <https://astarship.net>.
#include "stdafx.h"
//
#include "mode_editor.h"

EditorMode::EditorMode(StateData* state_data, TileMap* tile_map,
                       EditorStateData* editor_state_data)
    : stateData(state_data),
      tileMap(tile_map),
      editor_state_data_(editor_state_data) {}

EditorMode::~EditorMode() {}

const bool EditorMode::KeyTime() {
  if (*editor_state_data_->keytime >= *editor_state_data_->keytimeMax) {
    *editor_state_data_->keytime = 0.f;
    return true;
  }

  return false;
}
