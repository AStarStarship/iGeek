// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef PACWORLD_MAZE
#define PACWORLD_MAZE

#include "../../ASCIICrabs/_Config.h"
#include "../../ASCIICrabs/TVec.h"

namespace PacWorld {

// The homotuple types (VHT) live in namespace _ (ASCIICrabs TVec.h). Bring
// the 4 common ones into PacWorld so game-logic code can use ISC2/FPC2/
// ISC4/FPC4 unqualified (matching the codebase's _::TVec2F convention,
// just scoped to PacWorld).
using _::ISC2;
using _::FPC2;
using _::ISC4;
using _::FPC4;

/* A headless maze grid. The grid is a fixed-size CellData array (standard
   Pac-Man 28x31). No SFML, no stdlib — pure data + queries. The grid is
   loaded from a raw CellData buffer (data-driven), not a texture. */
class Maze {
  enum CellData {
    kEmpty = 0,
    kWall,
    kDot,
    kSuperDot,
    kBonus,
  };

  static const ISC MaxW = 28;  // standard Pac-Man grid width (cells).
  static const ISC MaxH = 31;  // standard Pac-Man grid height (cells).
  static const ISC CellPx = 32;  // pixels per cell.

  ISC2 maze_size_;             // grid dimensions (cells).
  CellData maze_data_[MaxW * MaxH];  // the grid (row-major).
  ISC2 pac_position_;          // pac's spawn cell.
  ISC2 ghost_positions_[MaxW * MaxH];  // ghost spawn cells.
  ISC ghost_count_;            // number of ghost spawns.

 public:
  enum Error {
    kSuccess = 0,
    kInvalidFile,
    kTooSmall,
  };

  Maze();

  ISC2 Size() const;

  /* Load a level from a raw grid buffer (data-driven). `data` is a
     w*h CellData buffer (row-major). Colors map: kWall/kDot/kSuperDot/
     kEmpty/kBonus are set directly in the buffer by the caller.
     @return kSuccess on success, or an Error code. */
  Error LoadLevel(const ISC* data, ISC w, ISC h);

  ISC2 PacPosition() const;

  ISC GhostCount() const;
  ISC2 GhostPosition(ISC i) const;

  inline ISC PositionToIndex(ISC2 position) const;

  inline ISC2 IndexToPosition(ISC index) const;

  /* Convert pixel coords to cell coords (explicit integer math, no floor). */
  ISC2 PixelToCell(FPC2 pixel) const;

  /* Convert cell coords to pixel coords (cell center). */
  FPC2 CellToPixel(ISC2 cell) const;

  bool IsWall(ISC2 position) const;
  bool IsDot(ISC2 position) const;
  bool IsSuperDot(ISC2 position) const;
  bool IsBonus(ISC2 position) const;

  /* Clear a picked object (dot/superdot/bonus) at the given cell. */
  void PickObject(ISC2 position);

  /* Number of dots + superdots remaining. */
  ISC DotCount() const;
};
}  // namespace PacWorld
#endif
