// Copyright AStarship <https://astarship.net>.
#pragma once
#include "TVec.h"
#ifndef TILEWORLD_GUI
#define TILEWORLD_GUI
#include <_Config.h>
enum button_states {
  kButtonIdle = 0,  //<
  kButtonHover,     //<
  kButtonActive     //<
};

namespace gui {
const FPC p2pX(const FPC perc, const _::TVideoMode& vm);
const FPC p2pY(const FPC perc, const _::TVideoMode& vm);
const unsigned CalcCharSize(const _::TVideoMode& vm,
                            const unsigned modifier = 60);

class Button {
 private:
  short unsigned buttonState;
  short unsigned id;

  sf::RectangleShape shape;
  sf::Font* font;
  sf::Text text;

  IUN textIdleColor;
  IUN textHoverColor;
  IUN textActiveColor;

  IUN idleColor;
  IUN hoverColor;
  IUN activeColor;

  IUN outlineIdleColor;
  IUN outlineHoverColor;
  IUN outlineActiveColor;

 public:
  Button(FPC x, FPC y, FPC width, FPC height, sf::Font* font,
         _::AString text, unsigned character_size, IUN text_idle_color,
         IUN text_hover_color, IUN text_active_color,
         IUN idle_color, IUN hover_color, IUN active_color,
         IUN outline_idle_color = IUN::Transparent,
         IUN outline_hover_color = IUN::Transparent,
         IUN outline_active_color = IUN::Transparent,
         short unsigned id = 0);
  ~Button();

  const bool IsPressed() const;
  const _::AString Text() const;
  const short unsigned& Id() const;

  // Modifiers
  void setText(const _::AString text);
  void setId(const short unsigned id);

  void Update(const _::ISC2& mouse_pos_window);
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
  void Update(const _::ISC2& mouse_pos_window, const FPC& dt);
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
  _::ISC2 mouse_pos_grid_;
  _::ISC4 texture_rect_;

 public:
  TextureSelector(FPC x, FPC y, FPC width, FPC height, FPC grid_size,
                  const sf::Texture* texture_sheet, sf::Font& font,
                  _::AString text);
  ~TextureSelector();

  const bool& getActive() const;
  const _::ISC4& getTextureRect() const;

  const bool KeyTime();
  void UpdateKeyTime(const FPC& dt);
  void Update(const _::ISC2& mouse_pos_window, const FPC& dt);
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
              IUN inner_color, unsigned character_size, _::TVideoMode& vm,
              sf::Font* font = nullptr);
  ~ProgressBar();

  void Update(const ISN current_value);
  void Render(sf::RenderTarget& target);
};
}  //< namespace gui

#endif
