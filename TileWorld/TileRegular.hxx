// Copyright AStarship <https://astarship.net>.
#include "TileRegular.h"

// PORT-NOTE: the original ctor set the sprite texture/texture-rect; those are
// data-only fields on the base Tile now.
TileRegular::TileRegular(_::ISM type, _::ISC grid_x, _::ISC grid_y, _::FPC grid_size_f_,
                         const _::TIntRect& texture_rect, _::BOL collision)
    : Tile(type, grid_x, grid_y, grid_size_f_, texture_rect, collision) {}

TileRegular::~TileRegular() {}

// PORT-NOTE: the original ToString() used std::stringstream + std::cout
// (no-stdlib violation) and was used only by the removed file-save path. The
// pure-virtual is dropped from the headless Tile; nothing needs it here.
void TileRegular::Update() {}
