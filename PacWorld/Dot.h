// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef PACWORLD_DOT
#define PACWORLD_DOT

#include "Maze.h"  // brings in _Config.h (IUN) + TVec.h

namespace PacWorld {

/* A headless dot (or super-dot) — the render data (radius + packed color).
   The actual drawing is a display concern (CImage FillCircle later).
   `color_` is a packed RGBA (IUN), same layout as CrabsTK PackColor.
   No SFML, no stdlib. */
struct TDot {
  FPC radius_;
  IUN color_;  // packed RGBA.

  TDot() : radius_(0.f), color_(0xFFFFFFFFu) {}  // default: white, opaque.
  TDot(FPC radius, IUN color) : radius_(radius), color_(color) {}
};

/* A regular dot (small, white). */
TDot Dot();

/* A super-dot (larger, white — triggers the weak-ghost state). */
TDot SuperDot();

}  // namespace PacWorld
#endif
