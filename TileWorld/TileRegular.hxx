// Copyright AStarship <https://astarship.net>.
//
// TileRegular.hxx — headless regular tile (TileWorld core, SFML stripped).
// Matches the SFML-stripped TileRegular.h: the tile no longer owns an
// sf::Sprite / sf::Texture; the texture sub-rect is data-only. Render()
// deleted (headless). No-stdlib.
#include "TileRegular.h"

TileRegular::TileRegular(_::ISM type, _::ISC grid_x, _::ISC grid_y,
                         _::FPC grid_size_f_, const _::TIntRect& texture_rect,
                         _::BOL collision)
    : Tile(type, grid_x, grid_y, grid_size_f_, texture_rect, collision) {}

TileRegular::~TileRegular() {}

void TileRegular::Update() {}
