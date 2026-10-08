// Copyright AStarship <https://astarship.net>.
//
// TileRegular.h — headless regular tile (TileWorld core, SFML stripped).
// Render() deleted (headless). No-stdlib.
#pragma once
#include <_ConfigHeader.h>
#include <TVec.h>

#ifndef TILEWORLD_REGULARTILE
#define TILEWORLD_REGULARTILE

#include "Tile.h"

class TileRegular : public Tile {
 public:
  TileRegular(_::ISM type, _::ISC grid_x, _::ISC grid_y, _::FPC grid_size_f_,
              const _::TIntRect& texture_rect, _::BOL collision = false);

  virtual ~TileRegular();

  virtual void Update();
};

#endif
