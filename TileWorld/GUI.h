// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef TILEWORLD_GUI
#define TILEWORLD_GUI
#include <_Config.h>
enum button_states {
  kButtonIdle = 0,  //<
  kButtonHover,     //<
  kButtonActive     //<
};

namespace gui {
const FPC p2pX(const FPC perc, const sf::VideoMode& vm);
const FPC p2pY(const FPC perc, const sf::VideoMode& vm);
const unsigned CalcCharSize(const sf::VideoMode& vm,
                            const unsigned modifier = 60);

class Button {
 private:
  short unsigned buttonState;
  short unsigned id;

  sf::RectangleShape shape;
  sf::Font* font;
  sf::Text text;

  sf::Color textIdleColor;
  sf::Color textHoverColor;
  sf::Color textActiveColor;

  sf::Color idleColor;
  sf::Color hoverColor;
  sf::Color activeColor;

  sf::Color outlineIdleColor;
  sf::Color outlineHoverColor;
  sf::Color outlineActiveColor;

 public:
  Button(FPC x, FPC y, FPC width, FPC height, sf::Font* font,
         _::AString text, unsigned character_size, sf::Color text_idle_color,
         sf::Color text_hover_color, sf::Color text_active_color,
         sf::Color idle_color, sf::Color hover_color, sf::Color active_color,
         sf::Color outline_idle_color = sf::Color::Transparent,
         sf::Color outline_hover_color = sf::Color::Transparent,
         sf::Color outline_active_color = sf::Color::Transparent,
         short unsigned id = 0);
  ~Button();

  const bool IsPressed() const;
  const _::AString Text() const;
  const short unsigned& Id() const;

  // Modifiers
  void setText(const _::AString text);
  void setId(const short unsigned id);

  void Update(const sf::Vector2i& mouse_pos_window);
  void Render(sf::RenderTarget& target);
};

class DropDownList {
 private:
  FPC keytime_;
  FPC keytime_max_;

  sf::Font& font_;
  gui::Button* active_element_;
  _::Array<gui::Button*> list_;
  bool show_list_;

 public:
  DropDownList(FPC x, FPC y, FPC width, FPC height, sf::Font& font,
               _::AString list[], unsigned nr_of_elements,
               unsigned default_index = 0);
  ~DropDownList();

  const unsigned short& GetActiveElementId() const;

  const bool KeyTime();

  void UpdateKeyTime(const FPC& dt);
  void Update(const sf::Vector2i& mouse_pos_window, const FPC& dt);
  void Render(sf::RenderTarget& target);
};

class TextureSelector {
  FPC key_press_time_;
  const FPC key_press_time_max_;
  FPC grid_size_;
  bool active_  //<
      hidden_;
  gui::Button* hide_button_;
  sf::RectangleShape bounds_;
  sf::Sprite sheet_;
  sf::RectangleShape selector_;
  sf::Vector2u mouse_pos_grid_;
  sf::IntRect texture_rect_;

 public:
  TextureSelector(FPC x, FPC y, FPC width, FPC height, FPC grid_size,
                  const sf::Texture* texture_sheet, sf::Font& font,
                  _::AString text);
  ~TextureSelector();

  const bool& getActive() const;
  const sf::IntRect& getTextureRect() const;

  const bool KeyTime();
  void UpdateKeyTime(const FPC& dt);
  void Update(const sf::Vector2i& mouse_pos_window, const FPC& dt);
  void Render(sf::RenderTarget& target);
};

class ProgressBar {
 private:
  _::AString barString;
  sf::Text text;
  FPC maxWidth;
  ISN maxValue;
  sf::RectangleShape back;
  sf::RectangleShape inner;

 public:
  ProgressBar(FPC x, FPC y, FPC width, FPC height, ISN max_value,
              sf::Color inner_color, unsigned character_size, sf::VideoMode& vm,
              sf::Font* font = nullptr);
  ~ProgressBar();

  void Update(const ISN current_value);
  void Render(sf::RenderTarget& target);
};
}  //< namespace gui

#endif
