// Copyright AStarship <https://astarship.net>.
#include "player_gui.h"

void PlayerGUI::InitFont() { font.LoadFromFile("Fonts/Dosis-Light.ttf"); }

void PlayerGUI::InitLevelBar() {
  float width = gui::p2pX(1.6f, vm);
  float height = gui::p2pY(2.8f, vm);
  float x = gui::p2pX(1.f, vm);
  float y = gui::p2pY(1.9f, vm);

  levelBarBack.setSize(sf::Vector2f(width, height));
  levelBarBack.setFillColor(sf::Color(50, 50, 50, 200));
  levelBarBack.PositionSet(x, y);

  levelBarText.setFont(font);
  levelBarText.setCharacterSize(gui::CalcCharSize(vm, 150));
  levelBarText.PositionSet(levelBarBack.getPosition().x + gui::p2pX(0.53f, vm),
                           levelBarBack.getPosition().y + gui::p2pY(0.5f, vm));
}

void PlayerGUI::InitExpBar() {
  expBar = new gui::ProgressBar(1.f, 5.6f, 10.4f, 1.9f,
                                player->GetAttributeComponent()->next_exp,
                                sf::Color::Blue, 220, vm, &font);
}

void PlayerGUI::InitHPBar() {
  hpBar = new gui::ProgressBar(1.f, 8.3f, 10.4f, 2.8f,
                               player->GetAttributeComponent()->hp_max,
                               sf::Color::Red, 180, vm, &font);
}

PlayerGUI::PlayerGUI(Player* player, sf::VideoMode& vm) : vm(vm) {
  player = player;

  InitFont();
  InitLevelBar();
  InitExpBar();
  InitHPBar();
}

PlayerGUI::~PlayerGUI() {
  delete hpBar;
  delete expBar;
}

// Functions
void PlayerGUI::UpdateLevelBar() {
  levelBarString = std::to_string(player->GetAttributeComponent()->level);
  levelBarText.setString(levelBarString);
}

void PlayerGUI::UpdateExpBar() {
  expBar->Update(player->GetAttributeComponent()->exp);
}

void PlayerGUI::UpdateHPBar() {
  hpBar->Update(player->GetAttributeComponent()->hp);
}

void PlayerGUI::Update(const float& dt) {
  UpdateLevelBar();
  UpdateExpBar();
  UpdateHPBar();
}

void PlayerGUI::RenderLevelBar(sf::RenderTarget& target) {
  target.draw(levelBarBack);
  target.draw(levelBarText);
}

void PlayerGUI::RenderExpBar(sf::RenderTarget& target) {
  expBar->Render(target);
}

void PlayerGUI::RenderHPBar(sf::RenderTarget& target) { hpBar->Render(target); }

void PlayerGUI::Render(sf::RenderTarget& target) {
  RenderLevelBar(target);
  RenderExpBar(target);
  RenderHPBar(target);
}
