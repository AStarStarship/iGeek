// Copyright AStarship <https://astarship.net>.
/* Hierarchical Reasoning Model (HRM) for Sudoku solving — implementation.
 *
 * H/L level recurrent processing:
 * - H-level (even rounds): global constraint propagation — when a cell's
 *   candidates are reduced to one, place it and clear the digit from peers.
 * - L-level (odd rounds): local cell deduction — when a digit has exactly
 *   one possible cell in a row/column/box, place it there.
 * - Halts when all cells are filled (solved) or max_steps is reached.
 *
 * I/O: COut/StdOut for output (ASCIICrabs). Puzzle input is a flat IUD
 * array; the HRMReadPuzzle file path is not wired until the ASCIICrabs
 * File stack exposes a read side (see gap note below) — callers pass the
 * board directly, or fall back to a built-in sample.
 */

#include "HRM.h"

namespace iGeek {

/* Default HRM configuration. */
const HRMConfig kDefaultHRMConfig = {4, 8, 64, 7e-4f, 100, false};

/* Create the HRM environment and solve with the default configuration. */
BOL HRMCreate(IUD* board, const IUD* puzzle) {
  return HRMSolve(board, puzzle, kDefaultHRMConfig.max_steps);
}

/* Release HRM resources (no dynamic allocation in this API). */
void HRMRelease(void) {}

/* Run the HRM solving loop until solved or max_steps reached. */
BOL RunHRMSolving(IUD* board, const IUD* puzzle, ISC max_steps) {
  return HRMSolve(board, puzzle, max_steps);
}

/* Solve a Sudoku puzzle using the Hierarchical Reasoning Model. */
BOL HRMSolve(IUD* board, const IUD* puzzle, ISC max_steps) {
  HRMState state;

  /* Copy puzzle into state; init candidates (bits 0-8 = digits 1-9). */
  for (ISC i = 0; i < 81; ++i) {
    state.board[i] = puzzle[i];
    state.possibilities[i] = (puzzle[i] == 0) ? IUD(0x1FF) : IUD(0);
  }

  /* Propagate filled-cell values to peers (row/col/box constraints). */
  for (ISC i = 0; i < 81; ++i) {
    if (puzzle[i] != 0) {
      IUD bit = IUD(1) << (puzzle[i] - 1);
      ISC r = i / 9, c = i % 9;
      for (ISC c2 = 0; c2 < 9; ++c2) {
        ISC j = r * 9 + c2;
        if (j != i) state.possibilities[j] &= ~bit;
      }
      for (ISC r2 = 0; r2 < 9; ++r2) {
        ISC j = r2 * 9 + c;
        if (j != i) state.possibilities[j] &= ~bit;
      }
      ISC br = r - r % 3, bc = c - c % 3;
      for (ISC r2 = 0; r2 < 3; ++r2) {
        for (ISC c2 = 0; c2 < 3; ++c2) {
          ISC j = (br + r2) * 9 + (bc + c2);
          if (j != i) state.possibilities[j] &= ~bit;
        }
      }
    }
  }

  for (ISC i = 0; i < 64; ++i) {
    state.hidden_h[i] = 0;
    state.hidden_l[i] = 0;
  }
  state.steps = 0;
  state.round = 0;

  /* Main reasoning loop — alternate H and L level processing. */
  while (state.steps < max_steps) {
    BOL changed = false;

    /* H-level: global constraint propagation (even rounds). */
    if (state.round % 2 == 0) {
      for (ISC i = 0; i < 81 && !changed; ++i) {
        if (state.board[i] == 0 && state.possibilities[i] != 0) {
          ISC count = 0;
          for (ISC b = 0; b < 9; ++b) {
            if (state.possibilities[i] & (IUD(1) << b)) ++count;
          }
          /* Naked single: exactly one candidate remains — place it. */
          if (count == 1) {
            IUD v = 1;
            for (ISC b = 0; b < 9; ++b) {
              if (state.possibilities[i] & v) {
                state.board[i] = IUD(b + 1);
                changed = true;
                break;
              }
              v <<= 1;
            }
            /* Clear the placed digit from every other cell. */
            IUD clear_bit = IUD(1) << (state.board[i] - 1);
            for (ISC j = 0; j < 81; ++j) {
              if (j != i) state.possibilities[j] &= ~clear_bit;
            }
          }
        }
      }
    }

    /* L-level: local cell deduction (odd rounds). */
    if (state.round % 2 == 1) {
      for (ISC i = 0; i < 81 && !changed; ++i) {
        if (state.board[i] == 0 && state.possibilities[i] != 0) {
          IUD p = state.possibilities[i];
          ISC count = 0;
          for (ISC b = 0; b < 9; ++b) {
            if (p & (IUD(1) << b)) ++count;
          }
          if (count <= 2) {
            ISC r = i / 9, c = i % 9;
            ISC br = r - r % 3, bc = c - c % 3;
            for (ISC d = 1; d <= 9 && !changed; ++d) {
              if (!(p & (IUD(1) << (d - 1)))) continue;
              IUD d_bit = IUD(1) << (d - 1);
              /* Hidden single: digit d fits in exactly one cell of a unit. */
              ISC row_count = 0, row_idx = -1;
              for (ISC c2 = 0; c2 < 9; ++c2) {
                ISC j = r * 9 + c2;
                if (state.board[j] == 0 && (state.possibilities[j] & d_bit)) {
                  ++row_count;
                  row_idx = j;
                }
              }
              if (row_count == 1 && row_idx >= 0) {
                state.board[row_idx] = IUD(d);
                state.possibilities[row_idx] = 0;
                changed = true;
                break;
              }
              ISC col_count = 0, col_idx = -1;
              for (ISC r2 = 0; r2 < 9; ++r2) {
                ISC j = r2 * 9 + c;
                if (state.board[j] == 0 && (state.possibilities[j] & d_bit)) {
                  ++col_count;
                  col_idx = j;
                }
              }
              if (col_count == 1 && col_idx >= 0) {
                state.board[col_idx] = IUD(d);
                state.possibilities[col_idx] = 0;
                changed = true;
                break;
              }
              ISC box_count = 0, box_idx = -1;
              for (ISC r2 = 0; r2 < 3; ++r2) {
                for (ISC c2 = 0; c2 < 3; ++c2) {
                  ISC j = (br + r2) * 9 + (bc + c2);
                  if (j != i && state.board[j] == 0 && (state.possibilities[j] & d_bit)) {
                    ++box_count;
                    box_idx = j;
                  }
                }
              }
              if (box_count == 1 && box_idx >= 0) {
                state.board[box_idx] = IUD(d);
                state.possibilities[box_idx] = 0;
                changed = true;
                break;
              }
            }
          }
        }
      }
    }

    state.steps++;
    state.round++;

    /* Halt check: all cells filled? */
    BOL solved = true;
    for (ISC i = 0; i < 81; ++i) {
      if (state.board[i] == 0) {
        solved = false;
        break;
      }
    }
    if (solved) {
      for (ISC i = 0; i < 81; ++i) board[i] = state.board[i];
      return true;
    }
  }

  /* Not solved within the step limit — copy current state out. */
  for (ISC i = 0; i < 81; ++i) board[i] = state.board[i];
  return false;
}

/* Read a Sudoku puzzle from a text file.
 * GAP: the ASCIICrabs File/TextFile stack is write-side only as of this
 * commit — File::Open does directory validation but never loads bytes, and
 * TextFile has no read accessor (logged as an upstream gap). This function
 * therefore returns false and callers use the built-in sample puzzle.
 * The data files in Models/HRM/data/ remain the documented puzzle format
 * (81 whitespace-separated integers, row-major). */
BOL HRMReadPuzzle(const CHA* filename, IUD* puzzle) {
  (void)filename;
  (void)puzzle;
  return false;
}

/* Print a Sudoku board (formatted 9x9, dots for empty cells) to StdOut. */
void HRMPrintBoard(const IUD* board) {
  COut& out = StdOut();
  for (ISC r = 0; r < 9; ++r) {
    for (ISC c = 0; c < 9; ++c) {
      IUD val = board[r * 9 + c];
      out << CHA(val == 0 ? '.' : CHA('0' + val));
      if ((c + 1) % 3 == 0) out << CHA(' ');
    }
    out << CHA('\n');
    if ((r + 1) % 3 == 0) out << CHA('\n');
  }
  out << CHA('\n');
}

} //< namespace iGeek
