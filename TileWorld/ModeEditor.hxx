// Copyright AStarship <https://astarship.net>.
#include "mode_editor.h"

ModeEditor::ModeEditor(StateData* state_data, TileMap* tiles,
                       EditorStateData* editor_state_data)
    : state_data_(state_data),
      tiles_(tiles),
      editor_state_data_(editor_state_data) {}

ModeEditor::~ModeEditor() {}

const bool ModeEditor::KeyTime() {
  if (*editor_state_data_->key_press_time_ >= *editor_state_data_->key_press_time_max_) {
    *editor_state_data_->key_press_time_ = 0.f;
    return true;
  }

  return false;
}
