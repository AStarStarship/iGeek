// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef TILEWORLD_ANIMATIONCOMPONENT_DECL
#define TILEWORLD_ANIMATIONCOMPONENT_DECL
#include <_Config.h>
//
namespace TileWorld {

class AnimationComponent {
  private:
  class Animation {
   public:

    Animation(sf::Sprite& sprite, sf::Texture& texture_sheet,
              FPC animation_timer, ISN start_frame_x, ISN start_frame_y,
              ISN frames_x, ISN frames_y, ISN width, ISN height)
        : sprite(sprite),
          textureSheet(texture_sheet),
          animationTimer(animation_timer),
          timer(0.f),
          done(false),
          width(width),
          height(height) {
      start_rect = sf::IntRect(start_frame_x * width, start_frame_y * height,
                               width, height);
      current_rect = start_rect;
      endRect = sf::IntRect(frames_x * width, frames_y * height, width, height);

      sprite.setTexture(textureSheet, true);
      sprite.setTextureRect(start_rect);
    }

    const bool& IsDone() const;

    const bool& Play(const FPC& dt);

    const bool& Play(const FPC& dt, FPC mod_percent);

    void Reset();
    
    private:

    sf::Sprite& sprite_;
    sf::Texture& texture_sheet_;
    FPC animation_timer_, timer_;
    bool done_;
    ISN width_;
    ISN height_;
    sf::IntRect start_rect_, current_rect_, end_rect_;
  };

  sf::Sprite& sprite_;
  sf::Texture& texture_sheet_;
  _::ADic<_::AString, Animation*> animations;
  Animation* last_animation_;
  Animation* priority_animation_;

  public:

  AnimationComponent(sf::Sprite& sprite, sf::Texture& texture_sheet);
  virtual ~AnimationComponent();

  const bool& IsDone(const _::AString key);

  void AddAnimation(const _::AString key, FPC animation_timer,
                    ISN start_frame_x, ISN start_frame_y, ISN frames_x,
                    ISN frames_y, ISN width, ISN height);

  const bool& Play(const _::AString key, const FPC& dt,
                   const bool priority = false);
  const bool& Play(const _::AString key, const FPC& dt, const FPC& modifier,
                   const FPC& modifier_max, const bool priority = false);
};
}  //< namespace TileWorld
#endif
