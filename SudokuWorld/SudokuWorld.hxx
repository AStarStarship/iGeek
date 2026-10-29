// Copyright AStarship <https://astarship.net>.
//
// SudokuWorld — swappable AI gym for 9x9 Sudoku. The environment is the
// board + constraint rules; the agent is whatever steps the Gym (PPO policy,
// search agent, HRM, ...). NO model is hard-coded here: the gym exposes the
// standard iGeek Gym batch interface (observations / actions / rewards /
// terminals / action-mask) so any agent can be compared against any other on
// the same puzzles.
//
// Pure board logic (constraint check, solver, verifier, generator) is
// deterministic and unit-tested in _Seams/SudokuWorld/01.Core.hxx.
// No C/C++ standard library (the legacy main.hxx used <cstdio>/fputs and a
// hard-coded hrm.h include — both removed).
#pragma once
#ifndef CRABS_SUDOKUWORLD_HXX
#define CRABS_SUDOKUWORLD_HXX
#include "SudokuWorld.h"
#include "../Gym.h"
#if SEAM >= CRABS_SUDOKUWORLD

namespace iGeek {

/* Flat 9x9 board: 81 IUD cells, 0 = empty, 1..9 = digit. Row-major,
   index = r * 9 + c. The classic Wikipedia puzzle (the deterministic test
   fixture). */
enum {
  SudokuCells = 81,
  SudokuSide = 9,
  SudokuBox = 3,
  SudokuActionCount = 81,   // cell index
  SudokuDigitMax = 9
};

typedef IUD TBoard[81];

/* The classic Wikipedia puzzle (default fixture / test vector). Canonical
   grid (0 = empty):
   5 3 0 | 0 7 0 | 0 0 0
   6 0 0 | 1 9 5 | 0 0 0
   0 9 8 | 0 0 0 | 0 6 0
   8 0 0 | 0 6 0 | 0 0 3
   4 0 0 | 8 0 3 | 0 0 1
   7 0 0 | 0 2 0 | 0 0 6
   0 6 0 | 0 0 0 | 2 8 0
   0 0 0 | 4 1 9 | 0 0 5
   0 0 0 | 0 8 0 | 0 7 9 */
inline void SudokuDefaultPuzzle(TBoard board) {
  static const IUD p[81] = {
    5, 3, 0, 0, 7, 0, 0, 0, 0,
    6, 0, 0, 1, 9, 5, 0, 0, 0,
    0, 9, 8, 0, 0, 0, 0, 6, 0,
    8, 0, 0, 0, 6, 0, 0, 0, 3,
    4, 0, 0, 8, 0, 3, 0, 0, 1,
    7, 0, 0, 0, 2, 0, 0, 0, 6,
    0, 6, 0, 0, 0, 0, 2, 8, 0,
    0, 0, 0, 4, 1, 9, 0, 0, 5,
    0, 0, 0, 0, 8, 0, 0, 7, 9
  };
  for (ISC i = 0; i < 81; ++i) board[i] = p[i];
}

/* True when digit d may be placed at cell index i (row + col + 3x3 box
   have no d elsewhere). */
inline BOL SudokuIsValid(const TBoard board, ISC i, IUD d) {
  ISC r = i / SudokuSide;
  ISC c = i % SudokuSide;
  for (ISC c2 = 0; c2 < SudokuSide; ++c2) {
    ISC j = r * SudokuSide + c2;
    if (c2 != c && board[j] == d) return false;
  }
  for (ISC r2 = 0; r2 < SudokuSide; ++r2) {
    ISC j = r2 * SudokuSide + c;
    if (r2 != r && board[j] == d) return false;
  }
  ISC br = r - r % SudokuBox;
  ISC bc = c - c % SudokuBox;
  for (ISC r2 = 0; r2 < SudokuBox; ++r2) {
    for (ISC c2 = 0; c2 < SudokuBox; ++c2) {
      ISC j = (br + r2) * SudokuSide + (bc + c2);
      if ((br + r2 != r || bc + c2 != c) && board[j] == d) return false;
    }
  }
  return true;
}

/* Count legal digits for cell i (0..9). Used to build the action mask. */
inline ISC SudokuCandidates(const TBoard board, ISC i) {
  ISC n = 0;
  for (IUD d = 1; d <= SudokuDigitMax; ++d) {
    if (SudokuIsValid(board, i, d)) ++n;
  }
  return n;
}

/* True when the board is fully filled AND every row/col/box has 1..9 exactly
   once. */
inline BOL SudokuVerify(const TBoard board) {
  for (ISC i = 0; i < SudokuCells; ++i) {
    if (board[i] < 1 || board[i] > SudokuDigitMax) return false;
  }
  for (ISC r = 0; r < SudokuSide; ++r) {
    IUD rowmask = 0;
    for (ISC c = 0; c < SudokuSide; ++c) {
      IUD bit = IUD(1) << (board[r * SudokuSide + c] - 1);
      if (rowmask & bit) return false;
      rowmask |= bit;
    }
  }
  for (ISC c = 0; c < SudokuSide; ++c) {
    IUD colmask = 0;
    for (ISC r = 0; r < SudokuSide; ++r) {
      IUD bit = IUD(1) << (board[r * SudokuSide + c] - 1);
      if (colmask & bit) return false;
      colmask |= bit;
    }
  }
  for (ISC br = 0; br < SudokuSide; br += SudokuBox) {
    for (ISC bc = 0; bc < SudokuSide; bc += SudokuBox) {
      IUD boxmask = 0;
      for (ISC r2 = 0; r2 < SudokuBox; ++r2) {
        for (ISC c2 = 0; c2 < SudokuBox; ++c2) {
          IUD bit = IUD(1) << (board[(br + r2) * SudokuSide + (bc + c2)] - 1);
          if (boxmask & bit) return false;
          boxmask |= bit;
        }
      }
    }
  }
  return true;
}

/* Deterministic backtracking solver. Returns true on a solution (board is
   filled), false when unsolvable. */
inline BOL SudokuSolve(TBoard board, ISC i) {
  if (i >= SudokuCells) return true;
  if (board[i] != 0) return SudokuSolve(board, i + 1);
  for (IUD d = 1; d <= SudokuDigitMax; ++d) {
    if (SudokuIsValid(board, i, d)) {
      board[i] = d;
      if (SudokuSolve(board, i + 1)) return true;
      board[i] = 0;
    }
  }
  return false;
}

/* Number of empty cells (episode length to completion). */
inline ISC SudokuEmptyCount(const TBoard board) {
  ISC n = 0;
  for (ISC i = 0; i < SudokuCells; ++i) {
    if (board[i] == 0) ++n;
  }
  return n;
}

/* Generate a full valid grid by solving an all-empty board, then seed
   `holes` DISTINCT random (seeded LCG) cells to 0. Deterministic per seed.
   (Rejects duplicate cell picks so the hole count is exactly `holes`.) */
inline void SudokuGenerate(TBoard board, ISC holes, IUD seed) {
  for (ISC i = 0; i < SudokuCells; ++i) board[i] = 0;
  SudokuSolve(board, 0);  // full valid grid
  IUD state = seed == 0 ? 1 : seed;
  ISC placed = 0;
  ISC guard = 0;
  while (placed < holes && guard < 10000) {
    state = state * 6364136223846793005ULL + 1442695040888963407ULL;
    ISC i = ISC((state >> 33) % SudokuCells);
    if (board[i] != 0) {  // only blank filled cells (no dupes)
      board[i] = 0;
      ++placed;
    }
    ++guard;
  }
}

/* ASudokuGym — the swappable gym. One env = one 9x9 board. An action is a
   (cell, digit) pair encoded as cell * 10 + digit; the gym places the digit
   when legal, rewards +1 for a correct placement (matching a hidden target
   solution), -1 for an illegal placement, and terminates when the board is
   fully filled (reward reflects whether it is a valid solution). Any agent
   (PPO policy, search agent, HRM) steps this via StepBatch() — the gym knows
   nothing about which agent is driving it. */
class ASudokuGym : public Gym {
 public:
  ASudokuGym(ISC count = 1, IUD seed = 7, ISC holes = 40)
      : Gym("sudoku"), seed_(seed), holes_(holes) {
    env_count_ = count < 1 ? 1 : count > GymEnvMax ? GymEnvMax : count;
    obs_len_ = SudokuCells;
    action_count_ = SudokuCells * (SudokuDigitMax + 1);  // cell*10 + digit
    observations_ = new FPC[env_count_ * obs_len_]();
    actions_ = new ISC[env_count_]();
    rewards_ = new FPC[env_count_]();
    terminals_ = new FPC[env_count_]();
    action_mask_ = new FPC[env_count_ * action_count_]();
    boards_ = new TBoard[env_count_];
    targets_ = new TBoard[env_count_];
    for (ISC b = 0; b < env_count_; ++b) ResetEnv(b);
  }
  ~ASudokuGym() override {
    delete[] observations_;
    delete[] actions_;
    delete[] rewards_;
    delete[] terminals_;
    delete[] action_mask_;
    delete[] boards_;
    delete[] targets_;
  }
  ASudokuGym(const ASudokuGym&) = delete;
  ASudokuGym& operator=(const ASudokuGym&) = delete;

  void ResetBatch() override {
    for (ISC b = 0; b < env_count_; ++b) ResetEnv(b);
  }
  void StepBatch() override {
    for (ISC b = 0; b < env_count_; ++b) StepEnv(b);
  }
  ISC ActionCount() const { return action_count_; }
  ISC ObsLength() const { return obs_len_; }
  const TBoard& Board(ISC b) const { return boards_[b]; }
  const TBoard& Target(ISC b) const { return targets_[b]; }

 private:
  TBoard* boards_ = NILP;
  TBoard* targets_ = NILP;
  IUD seed_ = 7;
  ISC holes_ = 40;

  void ResetEnv(ISC b) {
    // Target = the full solution (solved from the generated puzzle).
    SudokuGenerate(boards_[b], holes_, seed_ + IUD(b) * 7919ULL);
    TBoard solved;
    for (ISC i = 0; i < SudokuCells; ++i) solved[i] = boards_[b][i];
    SudokuSolve(solved, 0);
    for (ISC i = 0; i < SudokuCells; ++i) targets_[b][i] = solved[i];
    for (ISC i = 0; i < SudokuCells; ++i) observations_[b * obs_len_ + i] = FPC(boards_[b][i]);
    terminals_[b] = 0.0f;
    UpdateMask(b);
  }
  void UpdateMask(ISC b) {
    for (ISC cell = 0; cell < SudokuCells; ++cell) {
      for (ISC d = 0; d <= SudokuDigitMax; ++d) {
        ISC idx = cell * (SudokuDigitMax + 1) + d;
        BOL legal = false;
        if (boards_[b][cell] == 0 && d >= 1 && d <= SudokuDigitMax) {
          legal = SudokuIsValid(boards_[b], cell, IUD(d));
        }
        action_mask_[b * action_count_ + idx] = legal ? 1.0f : 0.0f;
      }
    }
  }
  void StepEnv(ISC b) {
    ISC a = actions_[b];
    if (a < 0 || a >= action_count_) { rewards_[b] = 0.0f; return; }
    ISC cell = a / (SudokuDigitMax + 1);
    ISC d = a % (SudokuDigitMax + 1);
    if (boards_[b][cell] != 0 || d < 1 || d > SudokuDigitMax) {
      rewards_[b] = -1.0f;  // illegal (filled cell or bad digit)
    } else if (!SudokuIsValid(boards_[b], cell, IUD(d))) {
      rewards_[b] = -1.0f;  // constraint violation
    } else {
      boards_[b][cell] = IUD(d);
      rewards_[b] = (IUD(d) == targets_[b][cell]) ? 1.0f : 0.0f;
    }
    if (SudokuEmptyCount(boards_[b]) == 0) {
      terminals_[b] = 1.0f;
      if (SudokuVerify(boards_[b])) rewards_[b] += 5.0f;  // solution bonus
    }
    for (ISC i = 0; i < SudokuCells; ++i) observations_[b * obs_len_ + i] = FPC(boards_[b][i]);
    UpdateMask(b);
  }
};

} //< namespace iGeek
#endif
#endif  // CRABS_SUDOKUWORLD_HXX
