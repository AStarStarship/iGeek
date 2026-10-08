// Copyright AStarship <https://astarship.net>.
#include "gui.h"

const float gui::p2pX(const float perc, const sf::VideoMode& vm) {
  /*
   * Converts a percentage value to pixels relative to the current resolution in
   * the x-axis.
   *
   * @param		float perc				The percentage
   * value.
   * @param		sf::VideoMode& vm		The current videomode of
   * the window (resolution).
   *
   * @return		float					The calculated
   * pixel value.
   */

  return std::floor(static_cast<float>(vm.width) * (perc / 100.f));
}

const float gui::p2pY(const float perc, const sf::VideoMode& vm) {
  /*
   * Converts a percentage value to pixels relative to the current resolution in
   * the y-axis.
   *
   * @param		float perc				The percentage
   * value.
   * @param		sf::VideoMode& vm		The current videomode of
   * the window (resolution).
   *
   * @return		float					The calculated
   * pixel value.
   */

  return std::floor(static_cast<float>(vm.height) * (perc / 100.f));
}

const unsigned gui::CalcCharSize(const sf::VideoMode& vm,
                                 const unsigned modifier) {
  /*
   * Calculates the character size for text using the current resolution and a
   * constant.
   *
   * @param		sf::VideoMode& vm		The current videomode of
   * the window (resolution).
   * @param		unsigned modifier		Used to modify the
   * character size in a more custom way.
   *
   * @return		unsigned				The calculated
   * character size value.
   */

  return static_cast<unsigned>((vm.width + vm.height) / modifier);
}

gui::Button::Button(float x, float y, float width, float height, sf::Font* font,
                    _::AString text, unsigned character_size,
                    sf::Color text_idle_color, sf::Color text_hover_color,
                    sf::Color text_active_color, sf::Color idle_color,
                    sf::Color hover_color, sf::Color active_color,
                    sf::Color outline_idle_color, sf::Color outline_hover_color,
                    sf::Color outline_active_color, short unsigned id) {
  buttonState = kButtonIdle;
  id = id;

  shape.PositionSet(sf::Vector2f(x, y));
  shape.setSize(sf::Vector2f(width, height));
  shape.setFillColor(idle_color);
  shape.setOutlineThickness(1.f);
  shape.setOutlineColor(outline_idle_color);

  font = font;
  text.setFont(*font);
  text.setString(text);
  text.setFillColor(text_idle_color);
  text.setCharacterSize(character_size);
  std::cout << text.GlobalBounds().width << "\n";
  text.PositionSet(shape.getPosition().x + (shape.GlobalBounds().width / 2.f) -
                       text.GlobalBounds().width / 2.f,
                   shape.getPosition().y);

  textIdleColor = text_idle_color;
  textHoverColor = text_hover_color;
  textActiveColor = text_active_color;

  idleColor = idle_color;
  hoverColor = hover_color;
  activeColor = active_color;

  outlineIdleColor = outline_idle_color;
  outlineHoverColor = outline_hover_color;
  outlineActiveColor = outline_active_color;
}

gui::Button::~Button() {}

const bool gui::Button::IsPressed() const {
  if (buttonState == kButtonActive) return true;

  return false;
}

const _::AString gui::Button::Text() const { return text.getString(); }

const short unsigned& gui::Button::Id() const { return id_; }

void gui::Button::setText(const _::AString text) { text.setString(text); }

void gui::Button::setId(const short unsigned id) { id_ = id; }

void gui::Button::Update(const sf::Vector2i& mouse_pos_window) {
  buttonState = kButtonIdle;
  if (shape.GlobalBounds().contains(
          static_cast<sf::Vector2f>(mouse_pos_window))) {
    buttonState = kButtonHover;
    if (sf::Mouse::IsButtonPressed(sf::Mouse::Left)) {
      buttonState = kButtonActive;
    }
  }

  switch (buttonState) {
    case kButtonIdle:
      shape.setFillColor(idleColor);
      text.setFillColor(textIdleColor);
      shape.setOutlineColor(outlineIdleColor);
      break;

    case kButtonHover:
      shape.setFillColor(hoverColor);
      text.setFillColor(textHoverColor);
      shape.setOutlineColor(outlineHoverColor);
      break;

    case kButtonActive:
      shape.setFillColor(activeColor);
      text.setFillColor(textActiveColor);
      shape.setOutlineColor(outlineActiveColor);
      break;

    default:
      shape.setFillColor(sf::Color::Red);
      text.setFillColor(sf::Color::Blue);
      shape.setOutlineColor(sf::Color::Green);
      break;
  }
}

void gui::Button::Render(sf::RenderTarget& target) {
  target.draw(shape);
  target.draw(text);
}

gui::DropDownList::DropDownList(float x, float y, float width, float height,
                                sf::Font& font, _::AString list[],
                                unsigned nr_of_elements, unsigned default_index)
    : font(font),
      showList(false),
      key_press_time_max_(1.f),
      key_press_time_(0.f) {
  // unsigned nr_of_elements = sizeof(list) / sizeof(_::AString);

  activeElement =
      new gui::Button(x, y, width, height, &font, list[default_index], 14,
                      sf::Color(255, 255, 255, 150),
                      sf::Color(255, 255, 255, 200), sf::Color(20, 20, 20, 50),
                      sf::Color(70, 70, 70, 200), sf::Color(150, 150, 150, 200),
                      sf::Color(20, 20, 20, 200), sf::Color(255, 255, 255, 200),
                      sf::Color(255, 255, 255, 255), sf::Color(20, 20, 20, 50));

  for (unsigned i = 0; i < nr_of_elements; i++) {
    list.Push(new gui::Button(
        x, y + ((i + 1) * height), width, height, &font, list[i], 14,
        sf::Color(255, 255, 255, 150), sf::Color(255, 255, 255, 255),
        sf::Color(20, 20, 20, 50), sf::Color(70, 70, 70, 200),
        sf::Color(150, 150, 150, 200), sf::Color(20, 20, 20, 200),
        sf::Color(255, 255, 255, 0), sf::Color(255, 255, 255, 0),
        sf::Color(20, 20, 20, 0), i));
  }
}

gui::DropDownList::~DropDownList() {
  delete activeElement;
  for (size_t i = 0; i < list.size(); i++) {
    delete list[i];
  }
}

const bool gui::DropDownList::KeyTime() {
  if (key_press_time_ >= key_press_time_max_) {
    key_press_time_ = 0.f;
    return true;
  }

  return false;
}

const unsigned short& gui::DropDownList::GetActiveElementId() const {
  return activeElement->Id();
}

// Functions
void gui::DropDownList::UpdateKeyTime(const float& dt) {
  if (key_press_time_ < key_press_time_max_) key_press_time_ += 10.f * dt;
}

void gui::DropDownList::Update(const sf::Vector2i& mouse_pos_window,
                               const float& dt) {
  UpdateKeyTime(dt);

  activeElement->Update(mouse_pos_window);

  // Show and hide the list
  if (activeElement->IsPressed() && KeyTime()) {
    if (showList)
      showList = false;
    else
      showList = true;
  }

  if (showList) {
    for (auto& i : list) {
      i->Update(mouse_pos_window);

      if (i->IsPressed() && KeyTime()) {
        showList = false;
        activeElement->setText(i->Text());
        activeElement->setId(i->Id());
      }
    }
  }
}

void gui::DropDownList::Render(sf::RenderTarget& target) {
  activeElement->Render(target);

  if (showList) {
    for (auto& i : list) {
      i->Render(target);
    }
  }
}

gui::TextureSelector::TextureSelector(float x, float y, float width,
                                      float height, float grid_size,
                                      const sf::Texture* texture_sheet,
                                      sf::Font& font, _::AString text)
    : key_press_time_max_(1.f), key_press_time_(0.f) {
  grid_size = grid_size;
  active_ = false;
  hidden = false;
  float offset = grid_size;

  bounds_.setSize(sf::Vector2f(width, height));
  bounds_.PositionSet(x + offset, y);
  bounds_.setFillColor(sf::Color(50, 50, 50, 100));
  bounds_.setOutlineThickness(1.f);
  bounds_.setOutlineColor(sf::Color(255, 255, 255, 200));

  sheet_.setTexture(*texture_sheet);
  sheet_.PositionSet(x + offset, y);

  if (sheet_.GlobalBounds().width > bounds_.GlobalBounds().width) {
    sheet_.setTextureRect(
        sf::IntRect(0, 0, static_cast<int>(bounds_.GlobalBounds().width),
                    static_cast<int>(sheet_.GlobalBounds().height)));
  }
  if (sheet_.GlobalBounds().height > bounds_.GlobalBounds().height) {
    sheet_.setTextureRect(
        sf::IntRect(0, 0, static_cast<int>(sheet_.GlobalBounds().width),
                    static_cast<int>(bounds_.GlobalBounds().height)));
  }

  selector_.PositionSet(x + offset, y);
  selector_.setSize(sf::Vector2f(grid_size, grid_size));
  selector_.setFillColor(sf::Color::Transparent);
  selector_.setOutlineThickness(1.f);
  selector_.setOutlineColor(sf::Color::Red);

  texture_rect_.width = static_cast<int>(grid_size);
  texture_rect_.height = static_cast<int>(grid_size);

  hide_button_ = new gui::Button(
      y, x, 50.f, 50.f, &font, text, 16, sf::Color(255, 255, 255, 200),
      sf::Color(255, 255, 255, 250), sf::Color(255, 255, 255, 50),
      sf::Color(70, 70, 70, 200), sf::Color(150, 150, 150, 250),
      sf::Color(20, 20, 20, 50));
}

gui::TextureSelector::~TextureSelector() { delete hide_button_; }

const bool& gui::TextureSelector::getActive() const { return active_; }

const sf::IntRect& gui::TextureSelector::getTextureRect() const {
  return texture_rect_;
}

const bool gui::TextureSelector::KeyTime() {
  if (key_press_time_ >= key_press_time_max_) {
    key_press_time_ = 0.f;
    return true;
  }

  return false;
}

// Functions
void gui::TextureSelector::UpdateKeyTime(const float& dt) {
  if (key_press_time_ < key_press_time_max_) key_press_time_ += 10.f * dt;
}

void gui::TextureSelector::Update(const sf::Vector2i& mouse_pos_window,
                                  const float& dt) {
  UpdateKeyTime(dt);
  hide_button_->Update(mouse_pos_window);

  if (hide_button_->IsPressed() && KeyTime()) {
    if (hidden)
      hidden = false;
    else
      hidden = true;
  }

  if (!hidden) {
    active_ = false;

    if (bounds_.GlobalBounds().contains(
            static_cast<sf::Vector2f>(mouse_pos_window))) {
      active_ = true;

      mouse_pos_grid_.x =
          (mouse_pos_window.x - static_cast<int>(bounds_.getPosition().x)) /
          static_cast<unsigned>(grid_size_);
      mouse_pos_grid_.y =
          (mouse_pos_window.y - static_cast<int>(bounds_.getPosition().y)) /
          static_cast<unsigned>(grid_size_);

      selector_.PositionSet(
          bounds_.getPosition().x + mouse_pos_grid_.x * grid_size_,
          bounds_.getPosition().y + mouse_pos_grid_.y * grid_size_);

      // Update texture rectangle
      texture_rect_.left =
          static_cast<int>(selector_.getPosition().x - bounds_.getPosition().x);
      texture_rect_.Top =
          static_cast<int>(selector_.getPosition().y - bounds_.getPosition().y);
    }
  }
}

void gui::TextureSelector::Render(sf::RenderTarget& target) {
  if (!hidden) {
    target.draw(bounds_);
    target.draw(sheet_);

    if (active_) target.draw(selector_);
  }

  hide_button_->Render(target);
}

// PROGRESSBAR
// =============================================================================
// PROGRESSBAR

gui::ProgressBar::ProgressBar(float _x, float _y, float _width, float _height,
                              int max_value, sf::Color inner_color,
                              unsigned character_size, sf::VideoMode& vm,
                              sf::Font* font) {
  float width = gui::p2pX(_width, vm);
  float height = gui::p2pY(_height, vm);
  float x = gui::p2pX(_x, vm);
  float y = gui::p2pY(_y, vm);

  maxWidth = width;
  maxValue = max_value;

  back.setSize(sf::Vector2f(width, height));
  back.setFillColor(sf::Color(50, 50, 50, 200));
  back.PositionSet(x, y);

  inner.setSize(sf::Vector2f(width, height));
  inner.setFillColor(inner_color);
  inner.PositionSet(back.getPosition());

  if (font) {
    text.setFont(*font);
    text.setCharacterSize(gui::CalcCharSize(vm, character_size));
    text.PositionSet(inner.getPosition().x + gui::p2pX(0.53f, vm),
                     inner.getPosition().y + gui::p2pY(0.5f, vm));
  }
}

gui::ProgressBar::~ProgressBar() {}

void gui::ProgressBar::Update(const int current_value) {
  float percent =
      static_cast<float>(current_value) / static_cast<float>(maxValue);

  inner.setSize(sf::Vector2f(static_cast<float>(std::floor(maxWidth * percent)),
                             inner.getSize().y));

  barString = std::to_string(current_value) + " / " + std::to_string(maxValue);
  text.setString(barString);
}

void gui::ProgressBar::Render(sf::RenderTarget& target) {
  target.draw(back);
  target.draw(inner);
  target.draw(text);
}
