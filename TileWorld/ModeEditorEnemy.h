// Copyright AStarship <https://astarship.net>.
#pragma once
#include <_Config.h>

#ifndef TILEWORLD_ENEMYEDITORMODE
#define TILEWORLD_ENEMYEDITORMODE

#include "ModeEditor.h"
#include "TileSpawner.h"

class ModeEditorEnemy : public ModeEditor {
  sf::Text cursor_text_;        //<
  sf::RectangleShape sidebar_,  //<
      selector_rect_;           //<
  sf::IntRect texture_rect_;    //<
  ISN type_, amount_,           //<
      time_to_spawn_;           //<
  FPC distance_max_;          //<

  void InitVariables();
  void InitGUI();

 public:
  ModeEditorEnemy(StateData* state_data, TileMap* tiles,
                  EditorStateData* editor_state_data);
  virtual ~ModeEditorEnemy();

  void UpdateInput(const FPC& dt);
  void UpdateGUI(const FPC& dt);
  void Update(const FPC& dt);

  void RenderGUI(sf::RenderTarget& target);
  void Render(sf::RenderTarget& target);
};

#endif  //! ENEMYEDITORMODE_H
