// Copyright AStarship <https://astarship.net>.
#include "animator.h"

using namespace kabuki::arcade;

Animator::Animator()
    : current_frame_(),
      is_playing_(false),
      duration_(sf::Time::Zero),
      loop_(false) {}

void Animator::AddFrame(sf::IntRect frame) { frames_.push_back(frame); }

void Animator::Play(sf::Time duration, bool loop) {
  is_playing_ = true;
  duration_ = duration;
  loop_ = loop;
}
bool Animator::IsPlaying() const { return is_playing_; }

void Animator::Update(sf::Time delta) {
  if (!is_playing_) return;
  static sf::Time time_buffer = sf::Time::Zero;
  time_buffer += delta;

  sf::Time frame_duration = duration / static_cast<float>(frames_.size());

  while (time_buffer > frame_duration) {
    ++current_frame_;
    if (current_frame_ == frames_.size()) {
      if (!loop_) is_playing_ = false;
      current_frame_ = 0;
    }
    time_buffer -= frame_duration;
  }
}
void Animator::Animate(sf::Sprite& sprite) {
  sprite.setTextureRect(frames_[current_frame_]);
}
