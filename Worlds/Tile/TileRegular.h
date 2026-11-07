// Copyright AStarship <https://astarship.net>.
#pragma once
#include <_Config.h>
#ifndef _iGeekTileWorldRegularH
#define _iGeekTileWorldRegularH
#include <Homotuples.h>
#include "Tile.h"

class TileRegular : public Tile {
 public:
  TileRegular(_::ISM type, _::ISC grid_x, _::ISC grid_y, _::FPC grid_size_f_,
              const _::TIntRect& texture_rect, _::BOL collision = false);

  virtual ~TileRegular();

  virtual void Update();
};

#endif
