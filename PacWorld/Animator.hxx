// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_ANIMATOR_HXX
#define IGEEK_ANIMATOR_HXX
#include "Animator.h"

using namespace PacWorld;

Animator::Animator()
    : frame_count_(0),
      current_frame_(0),
      is_playing_(false),
      duration_(0.f),
      time_buffer_(0.f),
      loop_(false) {}

void Animator::AddFrame(ISC4 frame) {
  if (frame_count_ >= FramesMax) return;
  frames_[frame_count_] = frame;
  ++frame_count_;
}

void Animator::Play(FPC duration, BOL loop) {
  is_playing_ = true;
  duration_ = duration;
  loop_ = loop;
}

BOL Animator::IsPlaying() const { return is_playing_; }

void Animator::Update(FPC delta) {
  if (!is_playing_ || frame_count_ == 0) return;
  time_buffer_ += delta;

  FPC frame_duration = duration_ / FPC(frame_count_);

  while (time_buffer_ >= frame_duration && frame_duration > 0.f) {
    ++current_frame_;
    if (current_frame_ == frame_count_) {
      if (!loop_) is_playing_ = false;
      current_frame_ = 0;
    }
    time_buffer_ -= frame_duration;
  }
}

#endif  // IGEEK_ANIMATOR_HXX
