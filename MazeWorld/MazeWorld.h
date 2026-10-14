// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_MAZEWORLD_DECL
#define IGEEK_MAZEWORLD_DECL
#include <_Config.h>
#include "../../ASCIICrabs/Random.h"
namespace _ {

/* Randomized ASCII terminal maze: perfect (recursive-backtracker) maze on a
   grid where odd rows are passages and even rows are wall rows; even columns
   are wall columns and odd columns are passage columns. Every passage pair
   (x,y) and (x+2,y) / (x,y+2) is connected by carving the wall between them.
   The result is a tree: exactly one path between any two cells, so A* finds
   the unique shortest path (no ties, no dead-end ambiguity for the solver).

   Terminal-friendly rendering (no colors, no 2-cell-wide sprites):
     '#' wall, '.' floor, 'S' start, 'E' exit, '*' path (A* trace),
     '@' the player / cursor (overlays S, E, or *).

   Grid invariants (verified by the probe):
     - width/height are always odd; border ring is all wall.
     - start is at (1,1), exit at (width-2, height-2).
     - every cell reachable; A* path length is odd (Manhattan parity).

   The maze is seeded: same seed, same maze. The recursive backtracker is
   driven by a deterministic LCG (not the std-backed ASCIICrabs Random, which
   only compiles under SEAM < CRABS_RANDOM) so the world itself is no-stdlib.
*/
struct MazeWorld {
  enum {
    MazeMaxCols = 201,  //< Max grid width (odd).
    MazeMaxRows = 61,   //< Max grid height (odd).
    CellWall = '#',
    CellFloor = '.',
    CellStart = 'S',
    CellExit = 'E',
    CellPath = '*',
    CellPlayer = '@',
  };
  enum {  //< A* neighbor order: Up, Down, Left, Right (deterministic).
    DirUp = 0,
    DirDown,
    DirLeft,
    DirRight,
    DirCount,
  };
  static const ISC Dx[DirCount];
  static const ISC Dy[DirCount];

  /* @param cols Grid width (odd; clamped to [7, MazeMaxCols]).
     @param rows Grid height (odd; clamped to [7, MazeMaxRows]).
     @param seed Generation seed (deterministic layout per seed). */
  MazeWorld(ISC cols, ISC rows, IUD seed);
  ~MazeWorld();

  /* Seeded recursive-backtracker maze; carves the perfect maze, then stamps
     S at (1,1) and E at (width-2, height-2). Returns false on bad args. */
  BOL Generate(IUD gen_seed);

  ISC Cols() const { return cols_; }
  ISC Rows() const { return rows_; }
  CHA Tile(ISC x, ISC y) const { return tiles_[y * cols_ + x]; }
  BOL InBounds(ISC x, ISC y) const;
  BOL IsWall(ISC x, ISC y) const { return InBounds(x, y) && Tile(x, y) == CellWall; }

  /* A* on the maze grid (4-neighborhood, unit cost). Fills parent_[] and
     returns the path length in cells (including start and exit); 0 when
     unreachable. Reconstructs the path into path_x_/path_y_ (start->exit)
     and stamps CellPath on the grid between start and exit. */
  ISC Solve(ISC start_x, ISC start_y, ISC goal_x, ISC goal_y);

  /* Re-draws the grid on the terminal: border, tiles, path, then the player.
     Pure COut (no ANSI cursor codes) — safe on any terminal, friendly for
     the no-color TUI constraint. */
  void Print(ISC player_x = -1, ISC player_y = -1) const;

  private:
  ISC cols_;
  ISC rows_;
  CHA tiles_[MazeMaxRows * MazeMaxCols];
  ISC parent_[MazeMaxRows * MazeMaxCols];  //< A* parent index, -1 = none.
  ISC path_x_[MazeMaxRows * MazeMaxCols];
  ISC path_y_[MazeMaxRows * MazeMaxCols];

  IUD rng_;
  IUD NextRand();  //< Next uint64 from the LCG.
  static ISC Abs(ISC v);  //< No <cstdlib> abs (no-stdlib rule).
  void ClearGrid();
};

}  // namespace _
#endif
