// Copyright AStarship <https://astarship.net>.
#include "menu_pause.h"

PauseMenu::PauseMenu(sf::VideoMode& vm, sf::Font& font) : font(font) {
  background.setSize(sf::Vector2f(static_cast<float>(vm.width),
                                  static_cast<float>(vm.height)));
  background.setFillColor(sf::Color(20, 20, 20, 100));

  container.setSize(
      sf::Vector2f(static_cast<float>(vm.width) / 4.f,
                   static_cast<float>(vm.height) - gui::p2pY(9.3f, vm)));
  container.setFillColor(sf::Color(20, 20, 20, 200));
  container.PositionSet(
      static_cast<float>(vm.width) / 2.f - container.getSize().x / 2.f, 30.f);

  menuText.setFont(font);
  menuText.setFillColor(sf::Color(255, 255, 255, 200));
  menuText.setCharacterSize(gui::CalcCharSize(vm));
  menuText.setString("PAUSED");
  menuText.PositionSet(container.getPosition().x + container.getSize().x / 2.f -
                           menuText.GlobalBounds().width / 2.f,
                       container.getPosition().y + gui::p2pY(4.f, vm));
}

PauseMenu::~PauseMenu() {
  auto it = buttons.begin();
  for (it = buttons.begin(); it != buttons.end(); ++it) {
    delete it->second;
  }
}

_::ADic<_::AString, gui::Button*>& PauseMenu::getButtons() { return buttons; }

const bool PauseMenu::IsButtonPressed(const _::AString key) {
  return buttons[key]->IsPressed();
}

void PauseMenu::AddButton(const _::AString key, const float y,
                          const float width, const float height,
                          const unsigned char_size, const _::AString text) {
  float x =
      container.getPosition().x + container.getSize().x / 2.f - width / 2.f;

  buttons[key] =
      new gui::Button(x, y, width, height, &font, text, char_size,
                      sf::Color(70, 70, 70, 200), sf::Color(250, 250, 250, 250),
                      sf::Color(20, 20, 20, 50), sf::Color(70, 70, 70, 0),
                      sf::Color(150, 150, 150, 0), sf::Color(20, 20, 20, 0));
}

void PauseMenu::Update(const sf::Vector2i& mouse_pos_window) {
  for (auto& i : buttons) {
    i.second->Update(mouse_pos_window);
  }
}

void PauseMenu::Render(sf::RenderTarget& target) {
  target.draw(background);
  target.draw(container);

  for (auto& i : buttons) {
    i.second->Render(target);
  }

  target.draw(menuText);
}
