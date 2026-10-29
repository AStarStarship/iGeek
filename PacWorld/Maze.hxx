// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_MAZE_HXX
#define IGEEK_MAZE_HXX
#include "Maze.h"

using namespace PacWorld;

Maze::Maze()
    : maze_size_(0, 0), pac_position_(0, 0), ghost_count_(0) {
  for (ISC i = 0; i < MaxW * MaxH; ++i) maze_data_[i] = kEmpty;
}

ISC2 Maze::Size() const { return maze_size_; }

/* Load a level from a raw grid buffer (data-driven). The buffer is a
   w*h array of ISC values, each encoding a CellData (0=kEmpty, 1=kWall,
   2=kDot, 3=kSuperDot, 4=kBonus). Red pac-spawn and blue ghost-spawn cells
   are encoded by the caller as kEmpty with the position recorded separately
   via the buffer convention: value 5 = pac spawn (becomes kEmpty),
   value 6 = ghost spawn (becomes kEmpty). */
Maze::Error Maze::LoadLevel(const ISC* data, ISC w, ISC h) {
  if (!data || w < 15 || h < 15) return kTooSmall;
  if (w > MaxW || h > MaxH) return kTooSmall;

  maze_size_.x = w;
  maze_size_.y = h;
  pac_position_ = ISC2(0, 0);
  ghost_count_ = 0;

  for (ISC i = 0; i < w * h; ++i) {
    ISC v = data[i];
    if (v == 5) {  // pac spawn
      pac_position_ = ISC2(i % w, i / w);
      maze_data_[i] = kEmpty;
    } else if (v == 6) {  // ghost spawn
      ghost_positions_[ghost_count_] = ISC2(i % w, i / w);
      ++ghost_count_;
      maze_data_[i] = kEmpty;
    } else if (v >= kEmpty && v <= kBonus) {
      maze_data_[i] = static_cast<CellData>(v);
    } else {
      maze_data_[i] = kEmpty;
    }
  }

  return kSuccess;
}

ISC2 Maze::PacPosition() const { return pac_position_; }

ISC Maze::GhostCount() const { return ghost_count_; }

ISC2 Maze::GhostPosition(ISC i) const {
  if (i < 0 || i >= ghost_count_) return ISC2(0, 0);
  return ghost_positions_[i];
}

ISC Maze::PositionToIndex(ISC2 position) const {
  return position.y * maze_size_.x + position.x;
}

ISC2 Maze::IndexToPosition(ISC index) const {
  return ISC2(index % maze_size_.x, index / maze_size_.x);
}

/* Pixel -> cell: explicit integer floor-division (handles negatives via
   truncation toward zero, which is correct for in-bounds pixels). */
ISC2 Maze::PixelToCell(FPC2 pixel) const {
  return ISC2(pixel.x / CellPx, pixel.y / CellPx);
}

/* Cell -> pixel: cell center (cell * 32 + 16). */
FPC2 Maze::CellToPixel(ISC2 cell) const {
  return FPC2(cell.x * CellPx + 16, cell.y * CellPx + 16);
}

bool Maze::IsWall(ISC2 position) const {
  if (position.x < 0 || position.y < 0 || position.x >= maze_size_.x ||
      position.y >= maze_size_.y)
    return true;
  return maze_data_[PositionToIndex(position)] == kWall;
}

bool Maze::IsDot(ISC2 position) const {
  if (position.x < 0 || position.y < 0 || position.x >= maze_size_.x ||
      position.y >= maze_size_.y)
    return false;
  return maze_data_[PositionToIndex(position)] == kDot;
}

bool Maze::IsSuperDot(ISC2 position) const {
  if (position.x < 0 || position.y < 0 || position.x >= maze_size_.x ||
      position.y >= maze_size_.y)
    return false;
  return maze_data_[PositionToIndex(position)] == kSuperDot;
}

bool Maze::IsBonus(ISC2 position) const {
  if (position.x < 0 || position.y < 0 || position.x >= maze_size_.x ||
      position.y >= maze_size_.y)
    return false;
  return maze_data_[PositionToIndex(position)] == kBonus;
}

void Maze::PickObject(ISC2 position) {
  if (position.x < 0 || position.y < 0 || position.x >= maze_size_.x ||
      position.y >= maze_size_.y)
    return;
  if (IsWall(position)) return;
  maze_data_[PositionToIndex(position)] = kEmpty;
}

ISC Maze::DotCount() const {
  ISC count = 0;
  for (ISC i = 0; i < maze_size_.x * maze_size_.y; ++i) {
    if (maze_data_[i] == kDot || maze_data_[i] == kSuperDot) ++count;
  }
  return count;
}

#endif  // IGEEK_MAZE_HXX
