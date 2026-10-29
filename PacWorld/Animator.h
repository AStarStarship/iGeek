// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_PACMOD_ANIMATOR_DECL
#define IGEEK_PACMOD_ANIMATOR_DECL

#include "Maze.h"  // brings in _Config.h (ISC/FPC/BOL) + TVec.h (ISC4)

namespace PacWorld {

/* A headless frame animator. Holds a fixed array of texture sub-rects
   (ISC4) and advances through them on a timer (FPC seconds). The actual
   sprite update is a display concern (CImage later) — Animate() is a no-op.
   No SFML, no stdlib. */
class Animator {
  static const ISC FramesMax = 16;

  ISC4 frames_[FramesMax];  // texture sub-rects (left, top, w, h).
  ISC frame_count_;
  ISC current_frame_;
  BOL is_playing_;
  FPC duration_;  // total animation duration in seconds.
  FPC time_buffer_;  // accumulated time since last frame advance.
  BOL loop_;

 public:
  Animator();

  /* Add a frame (texture sub-rect). */
  void AddFrame(ISC4 frame);

  /* Start playing. `duration` is the total seconds for the full cycle. */
  void Play(FPC duration, BOL loop);

  BOL IsPlaying() const;

  /* Advance the animation by `delta` seconds. */
  void Update(FPC delta);

  /* The current frame index. */
  ISC CurrentFrame() const { return current_frame_; }

  /* The current frame's texture sub-rect. */
  ISC4 CurrentRect() const { return frames_[current_frame_]; }

  /* No-op for headless — the sprite update is a display concern. */
  void Animate() {}
};

}  // namespace PacWorld
#endif
