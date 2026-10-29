// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_DOT_HXX
#define IGEEK_DOT_HXX
#include "Dot.h"

using namespace PacWorld;

/* A regular dot: radius 4, white (packed RGBA 0xFFFFFFFF). */
TDot PacWorld::Dot() {
  return TDot(4.f, 0xFFFFFFFFu);
}

/* A super-dot: radius 8, white. */
TDot PacWorld::SuperDot() {
  return TDot(8.f, 0xFFFFFFFFu);
}

#endif  // IGEEK_DOT_HXX
