// Copyright AStarship <https://astarship.net>.
#ifndef TILEWORLD_MAINMENUSTATE
#define TILEWORLD_MAINMENUSTATE

#include "EditorState.h"
#include "GameState.h"
#include "SettingsState.h"
#include "GUI.h"

class MainMenuState : public State {
 private:
  // Variables
  sf::Texture backgroundTexture;
  sf::RectangleShape background;
  sf::Font font;

  sf::RectangleShape btnBackground;
  std::map<_::AString, gui::Button*> buttons;

  // Functions
  void InitVariables();
  void initFonts();
  void initKeybinds();
  void initGui();
  void resetGui();

 public:
  MainMenuState(StateData* state_data);
  virtual ~MainMenuState();

  // Functions
  void updateInput(const FPC& dt);
  void updateButtons();
  void Update(const FPC& dt);
  void renderButtons(sf::RenderTarget& target);
  void Render(sf::RenderTarget* target = nullptr);
};

#endif
