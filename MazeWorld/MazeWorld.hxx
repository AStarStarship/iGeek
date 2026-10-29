// Copyright AStarship <https://astarship.net>.
// MazeWorld.hxx — maze generation (recursive backtracker) + A* solver.
// No-stdlib: no std containers, no exceptions, no heap (all fixed-size
// arrays). The only "system" surface is COut for Print(), which is the
// ASCIICrabs console primitive (see AGENTS.md no-stdlib rule).
#include "MazeWorld.h"
#include "../../ASCIICrabs/COut.h"
#if SEAM >= IGEEK_MAZEWORLD
namespace iGeek {

const ISC MazeWorld::Dx[DirCount] = {0, 0, -1, 1};
const ISC MazeWorld::Dy[DirCount] = {-1, 1, 0, 0};

/* 64-bit LCG (Numerical Recipes constants). Deterministic per seed — the
   maze generator must not depend on std::random (see Random.hxx, which only
   compiles under SEAM < CRABS_RANDOM). */
IUD MazeWorld::NextRand() {
  rng_ = rng_ * 6364136223846793005ULL + 1442695040888963407ULL;
  return rng_;
}

MazeWorld::MazeWorld(ISC cols, ISC rows, IUD seed)
    : cols_(0), rows_(0), rng_(seed) {
  if (cols < 7) cols = 7;
  if (rows < 7) rows = 7;
  if (cols > MazeMaxCols) cols = MazeMaxCols;
  if (rows > MazeMaxRows) rows = MazeMaxRows;
  if ((cols & 1) == 0) cols--;
  if ((rows & 1) == 0) rows--;
  cols_ = cols;
  rows_ = rows;
  ClearGrid();
  Generate(seed);
}

MazeWorld::~MazeWorld() {}

BOL MazeWorld::InBounds(ISC x, ISC y) const {
  return x >= 0 && y >= 0 && x < cols_ && y < rows_;
}

void MazeWorld::ClearGrid() {
  for (ISC i = 0; i < MazeMaxRows * MazeMaxCols; ++i) {
    tiles_[i] = CellWall;
    parent_[i] = -1;
  }
}

BOL MazeWorld::Generate(IUD gen_seed) {
  rng_ = gen_seed ? gen_seed : 1;
  // Carve a perfect maze with a recursive-backtracker, iteratively (explicit
  // stack — no recursion, no heap). Passage cells are odd/odd; walls between
  // them are even in exactly one coordinate.
  enum { StackMax = (MazeMaxCols * MazeMaxRows) / 4 + 8 };
  ISC stack_x[StackMax], stack_y[StackMax];
  ISC stack_height = 0;
  ISC cell_x = 1, cell_y = 1;
  tiles_[1 * cols_ + 1] = CellFloor;
  stack_x[0] = 1;
  stack_y[0] = 1;
  stack_height = 1;
  while (stack_height > 0) {
    cell_x = stack_x[stack_height - 1];
    cell_y = stack_y[stack_height - 1];
    // Shuffle the 4 directions with 3 LCG draws (deterministic Fisher-Yates).
    ISC dir[DirCount] = {DirUp, DirDown, DirLeft, DirRight};
    for (ISC d = DirCount - 1; d > 0; --d) {
      ISC pick = (ISC)(NextRand() % (IUD)(d + 1));
      ISC swap = dir[d];
      dir[d] = dir[pick];
      dir[pick] = swap;
    }
    BOL carved = false;
    for (ISC d = 0; d < DirCount; ++d) {
      ISC nx = cell_x + 2 * Dx[dir[d]];
      ISC ny = cell_y + 2 * Dy[dir[d]];
      if (!InBounds(nx, ny)) continue;
      if (tiles_[ny * cols_ + nx] != CellWall) continue;  // Already visited.
      // Carve the wall between and the destination cell.
      tiles_[(cell_y + Dy[dir[d]]) * cols_ + (cell_x + Dx[dir[d]])] = CellFloor;
      tiles_[ny * cols_ + nx] = CellFloor;
      stack_x[stack_height] = nx;
      stack_y[stack_height] = ny;
      ++stack_height;
      carved = true;
      break;
    }
    if (!carved) --stack_height;  // Dead end: backtrack.
  }
  tiles_[1 * cols_ + 1] = CellStart;
  tiles_[(rows_ - 2) * cols_ + (cols_ - 2)] = CellExit;
  return true;
}

ISC MazeWorld::Abs(ISC v) { return v < 0 ? -v : v; }

/* A* with a linear-scan open list (grid is small: 201*61 = 12,261 cells max,
   open list is at most the frontier — fine for a TUI game). f = g + h with
   Manhattan heuristic (admissible + consistent for unit costs). Ties break
   on lower f, then on insertion order — deterministic. */
ISC MazeWorld::Solve(ISC start_x, ISC start_y, ISC goal_x, ISC goal_y) {
  if (!InBounds(start_x, start_y) || !InBounds(goal_x, goal_y)) return 0;
  if (IsWall(start_x, start_y) || IsWall(goal_x, goal_y)) return 0;
  enum { OpenMax = MazeMaxCols * MazeMaxRows };
  ISC open[OpenMax];
  ISC g[OpenMax];
  ISC h[OpenMax];
  ISC open_count = 0;
  for (ISC i = 0; i < OpenMax; ++i) g[i] = -1;
  ISC start_index = start_y * cols_ + start_x;
  g[start_index] = 0;
  open[0] = start_index;
  open_count = 1;
  parent_[start_index] = -1;
  ISC goal_index = goal_y * cols_ + goal_x;
  while (open_count > 0) {
    // Pop the lowest-f node (linear scan; f = g + h computed on the fly).
    ISC best = 0;
    ISC best_x = open[0] % cols_;
    ISC best_y = open[0] / cols_;
    ISC best_f = g[open[0]] +
        (Abs(best_x - goal_x) + Abs(best_y - goal_y));
    for (ISC i = 1; i < open_count; ++i) {
      ISC x = open[i] % cols_;
      ISC y = open[i] / cols_;
      ISC f = g[open[i]] + (Abs(x - goal_x) + Abs(y - goal_y));
      if (f < best_f) {
        best_f = f;
        best = i;
      }
    }
    ISC current = open[best];
    open[best] = open[open_count - 1];
    --open_count;
    if (current == goal_index) {
      // Reconstruct the path start->goal.
      ISC path_len = 0;
      ISC cursor = current;
      ISC path_stack[OpenMax];
      while (cursor != -1) {
        path_stack[path_len++] = cursor;
        cursor = parent_[cursor];
      }
      // Walk the grid, stamping CellPath (keep S/E markers intact).
      for (ISC i = path_len - 1; i >= 0; --i) {
        ISC x = path_stack[i] % cols_;
        ISC y = path_stack[i] / cols_;
        path_x_[i] = x;
        path_y_[i] = y;
        if (tiles_[y * cols_ + x] != CellStart &&
            tiles_[y * cols_ + x] != CellExit)
          tiles_[y * cols_ + x] = CellPath;
      }
      return path_len;
    }
    ISC cx = current % cols_;
    ISC cy = current / cols_;
    for (ISC d = 0; d < DirCount; ++d) {
      ISC nx = cx + Dx[d];
      ISC ny = cy + Dy[d];
      if (!InBounds(nx, ny) || IsWall(nx, ny)) continue;
      ISC n_index = ny * cols_ + nx;
      ISC n_g = g[current] + 1;
      if (g[n_index] != -1 && n_g >= g[n_index]) continue;
      g[n_index] = n_g;
      parent_[n_index] = current;
      BOL found = false;
      for (ISC i = 0; i < open_count; ++i)
        if (open[i] == n_index) {
          found = true;
          break;
        }
      if (!found) {
        open[open_count] = n_index;
        ++open_count;
      }
    }
  }
  return 0;  // Unreachable.
}

void MazeWorld::Print(ISC player_x, ISC player_y) const {
  COut o("\n");
  for (ISC y = 0; y < rows_; ++y) {
    o << '+';
    for (ISC x = 0; x < cols_; ++x) {
      if (x == player_x && y == player_y) {
        o << CellPlayer;
        continue;
      }
      o << Tile(x, y);
      if (x == cols_ - 1) o << '+';
      else o << '.';
    }
  }
  o.NL();
}

} //< namespace iGeek
#endif
