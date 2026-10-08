// Copyright AStarship <https://astarship.net>.
#include "component_animation.h"

AnimationComponent::AnimationComponent(sf::Sprite& sprite,
                                       sf::Texture& texture_sheet)
    : sprite_(sprite),
      texture_sheet_(texture_sheet),
      lastAnimation_(nullptr),
      priority_animation_(nullptr) {}

AnimationComponent::~AnimationComponent() {
  for (auto& i : animations_) {
    delete i.second;
  }
}

const bool& AnimationComponent::IsDone(const _::AString key) {
  return animations_[key]->IsDone();
}

void AnimationComponent::AddAnimation(const _::AString key,
                                      float animation_timer, int start_frame_x,
                                      int start_frame_y, int frames_x,
                                      int frames_y, int width, int height) {
  animations_[key] =
      new Animation(sprite_, texture_sheet_, animation_timer, start_frame_x,
                    start_frame_y, frames_x, frames_y, width, height);
}

const bool& AnimationComponent::Play(const _::AString key, const float& dt,
                                     const bool priority) {
  if (priority_animation_) {
    if (priority_animation_ == animations_[key]) {
      if (lastAnimation_ != animations_[key]) {
        if (lastAnimation_ == nullptr)
          lastAnimation_ = animations_[key];
        else {
          lastAnimation_->reset();
          lastAnimation_ = animations_[key];
        }
      }
      if (animations_[key]->Play(dt)) {
        priority_animation_ = nullptr;
      }
    }
  } else {
    if (priority) {
      priority_animation_ = animations_[key];
    }

    if (lastAnimation_ != animations_[key]) {
      if (lastAnimation_ == nullptr)
        lastAnimation_ = animations_[key];
      else {
        lastAnimation_->reset();
        lastAnimation_ = animations_[key];
      }
    }

    animations_[key]->Play(dt);
  }

  return animations_[key]->IsDone();
}

const bool& AnimationComponent::Play(const _::AString key, const float& dt,
                                     const float& modifier,
                                     const float& modifier_max,
                                     const bool priority) {
  if (priority_animation_) {  // If there is a priority animation
    if (priority_animation_ == animations_[key]) {
      if (lastAnimation_ != animations_[key]) {
        if (lastAnimation_ == nullptr)
          lastAnimation_ = animations_[key];
        else {
          lastAnimation_->reset();
          lastAnimation_ = animations_[key];
        }
      }
      if (animations_[key]->Play(dt, abs(modifier / modifier_max))) {
        priority_animation_ = nullptr;
      }
    }
  } else {
    if (priority) {
      priority_animation_ = animations_[key];
    }

    if (lastAnimation_ != animations_[key]) {
      if (lastAnimation_ == nullptr)
        lastAnimation_ = animations_[key];
      else {
        lastAnimation_->reset();
        lastAnimation_ = animations_[key];
      }
    }

    animations_[key]->Play(dt, abs(modifier / modifier_max));
  }

  return animations_[key]->IsDone();
}

AnimationComponent::Animation::Animation(sf::Sprite& sprite,
                                         sf::Texture& texture_sheet,
                                         float animation_timer,
                                         int start_frame_x, int start_frame_y,
                                         int frames_x, int frames_y, int width,
                                         int height)
    : sprite(sprite),
      textureSheet(texture_sheet),
      animationTimer(animation_timer),
      timer(0.f),
      done(false),
      width(width),
      height(height) {
  start_rect =
      sf::IntRect(start_frame_x * width, start_frame_y * height, width, height);
  current_rect = start_rect;
  endRect = sf::IntRect(frames_x * width, frames_y * height, width, height);

  sprite.setTexture(textureSheet, true);
  sprite.setTextureRect(start_rect);
}

const bool& AnimationComponent::Animation::IsDone() const { return done; }

const bool& AnimationComponent::Animation::Play(const float& dt) {
  done = false;
  timer += 100.f * dt;
  if (timer >= animationTimer) {
    timer = 0.f;

    if (current_rect != endRect) {
      current_rect.left += width;
    } else {
      current_rect.left = start_rect.left;
      done = true;
    }

    sprite.setTextureRect(current_rect);
  }

  return done;
}

const bool& AnimationComponent::Animation::Play(const float& dt,
                                                float mod_percent) {
  if (mod_percent < 0.5f) mod_percent = 0.5f;

  done = false;
  timer += mod_percent * 100.f * dt;
  if (timer >= animationTimer) {
    timer = 0.f;

    if (current_rect != endRect) {
      current_rect.left += width;
    } else {
      current_rect.left = start_rect.left;
      done = true;
    }

    sprite.setTextureRect(current_rect);
  }

  return done;
}

void AnimationComponent::Animation::Reset() {
  timer = animationTimer;
  current_rect = start_rect;
}
