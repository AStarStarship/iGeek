// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekPacModAnimatorH
#define _iGeekPacModAnimatorH
#include "Maze.h"
namespace PacWorld {

/* A headless frame animator. 
Holds a fixed array of texture sub-rects (ISC4) and advances through them on a 
timer (FPC seconds). The actual sprite update is a display concern (CImage 
later) — Animate() is a no-op. */
class Animator {
  static const ISC FramesMax = 16;

  ISC4 frames_[FramesMax];  //< Texture sub-rects (left, top, w, h).
  ISC frame_count_;         //< The number of frames added (<= FramesMax).
  ISC current_frame_;       //< The current frame index.
  BOL is_playing_;          //< Whether the animation is currently playing.
  FPC duration_;            //< Total animation duration in seconds.
  FPC time_buffer_;         //< Accumulated time since last frame advance.
  BOL loop_;                //< Whether the animation should loop.

 public:
  Animator();

  /* Add a frame (texture sub-rect). */
  void AddFrame(ISC4 frame);

  /* Start playing. `duration` is the total seconds for the full cycle. */
  void Play(FPC duration, BOL loop);

  BOL IsPlaying();

  /* Advance the animation by `delta` seconds. */
  void Update(FPC delta);

  /* The current frame index. */
  ISC CurrentFrame() { return current_frame_; }

  /* The current frame's texture sub-rect. */
  ISC4 CurrentRect() { return frames_[current_frame_]; }

  /* No-op for headless — the sprite update is a display concern. */
  void Animate() {}
};

}  //< namespace PacWorld
#endif
