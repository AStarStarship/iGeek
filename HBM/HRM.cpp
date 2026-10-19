// Copyright AStarship <https://astarship.net>.
/* Exported HRM functions for Sudoku solving.
 *
 * Provides project-interface-compatible HRM functions using only
 * ASCIICrabs and iGeek project types - no C++ standard library.
 */

#include "hrm.h"
#include <cstdio>     /* fopen, fscanf, fclose - C stdio project convention */
#include <string.h>   /* memset */

/* Default HRM configuration - project convention values */
const HRMConfig kDefaultHRMConfig = {4, 8, 64, 7e-5f, 100, false};

/* Create the HRM environment and initialize with a Sudoku puzzle.
 * In this simplified API, HRMSolve() handles the full solve cycle.
 * This function exists for interface compatibility with the iGeek framework.
 * @param board   Output board (81 IUD cells)
 * @param puzzle  Input puzzle (81 IUD cells, 0=empty)
 * @return true if puzzle solved within default steps
 */
bool HRMCreate(IUD* board, const IUD* puzzle) {
  /* Copy puzzle into board and solve with default config */
  for (ISC i = 0; i < 81; ++i) {
    board[i] = puzzle[i];
  }
  return HRMSolve(board, puzzle, kDefaultHRMConfig.max_steps);
}

/* Release HRM resources (no dynamic allocation in this simplified API) */
void HRMRelease(void) {
  /* No resources to release in simplified API */
}

/* Run the HRM solving loop until the puzzle is solved or max steps reached.
 * This is the main entry point - delegates to HRMSolve().
 * @param board   IUD* - board of 81 cells (input puzzle, output solution)
 * @param puzzle  const IUD* - input puzzle (81 values, 0=empty)
 * @param max_steps ISC - maximum reasoning steps
 * @return true if solved, false if max_steps reached
 *
 * Interface:
 * - Copies puzzle into board
 * - Calls HRMSolve() which performs H/L level recurrent processing
 * - Halts when all cells filled or max_steps reached
 * - Returns true/false based on success
 */
bool RunHRMSolving(IUD* board, const IUD* puzzle, ISC max_steps) {
  /* Copy puzzle into board first */
  for (ISC i = 0; i < 81; ++i) {
    board[i] = puzzle[i];
  }
  return HRMSolve(board, puzzle, max_steps);
}

/* Solve a Sudoku puzzle using Hierarchical Reasoning Model.
 * Performs H/L level recurrent processing:
 * - Alternates H-level (global constraint propagation, every 2 steps)
 *   and L-level (local cell deduction, every odd step)
 * - Halts when all cells filled (solved) or max_steps reached
 * @param board   Output: flat IUD array of 81 cells (will be filled with solution)
 * @param puzzle  Input: flat IUD array of 81 values (0-9, 0=empty)
 * @param max_steps Maximum reasoning steps before halting
 * @return true if puzzle solved, false if max_steps reached
 *
 * Implementation:
 * - H-level: global constraint propagation across rows/cols/boxes
 * - L-level: local cell deduction (forced digits in units)
 * - Recurrent processing: alternate H and L levels until halt
 * - Reward: FPC = correct_cells / 81
 */
bool HRMSolve(IUD* board, const IUD* puzzle, ISC max_steps) {
  /* Initialize state */
  HRMState state;

  /* Copy puzzle into state board */
  for (ISC i = 0; i < 81; ++i) {
    state.board[i] = puzzle[i];
    /* Initialize possibilities: 0x1FF = bits 0-8 for digits 1-9 */
    state.possibilities[i] = 0x1FF;
    if (puzzle[i] == 0) {
      state.possibilities[i] = 0x1FF;  /* Empty cell: all digits possible */
    } else {
      state.possibilities[i] = 0;      /* Filled cell: no possibilities */
    }
  }

  /* Propagate filled cell values to peers (row/col/box constraints) */
  for (ISC i = 0; i < 81; ++i) {
    if (puzzle[i] != 0) {
      IUD val = puzzle[i];  /* digit 1-9 */
      IUD bit = 1 << (val - 1);  /* corresponding bit */
      ISC r = i / 9;  /* row */
      ISC c = i % 9;  /* column */

      /* Remove from peers in same row */
      for (ISC c2 = 0; c2 < 9; ++c2) {
        ISC j = r * 9 + c2;
        if (j != i) {
          state.possibilities[j] &= ~bit;
        }
      }

      /* Remove from peers in same column */
      for (ISC r2 = 0; r2 < 9; ++r2) {
        ISC j = r2 * 9 + c;
        if (j != i) {
          state.possibilities[j] &= ~bit;
        }
      }

      /* Remove from peers in same 3x3 box */
      ISC br = r - r % 3;
      ISC bc = c - c % 3;
      for (ISC r2 = 0; r2 < 3; ++r2) {
        for (ISC c2 = 0; c2 < 3; ++c2) {
          ISC j = (br + r2) * 9 + (bc + c2);
          if (j != i) {
            state.possibilities[j] &= ~bit;
          }
        }
      }
    }
  }

  /* Initialize hidden states to zero */
  for (ISC i = 0; i < 64; ++i) {
    state.hidden_H[i] = 0;
    state.hidden_L[i] = 0;
  }
  state.steps = 0;
  state.round = 0;

  /* Main reasoning loop - alternate H and L level processing */
  while (state.steps < max_steps) {
    bool changed = false;

    /* H-level: Abstract planning - global constraint propagation */
    /* Every 2 steps (round % 2 == 0), do global constraint propagation */
    if (state.round % 2 == 0) {
      for (ISC i = 0; i < 81 && !changed; ++i) {
        if (state.board[i] == 0 && state.possibilities[i] != 0) {
          /* Count remaining possibilities (bits 0-8 = digits 1-9) */
          ISC count = 0;
          for (ISC b = 0; b < 9; ++b) {
            if (state.possibilities[i] & (1 << b)) ++count;
          }
          /* If only one possibility remains, place it */
          if (count == 1) {
            IUD v = 1;
            for (ISC b = 0; b < 9; ++b) {
              if (state.possibilities[i] & v) {
                state.board[i] = b + 1;  /* Store digit 1-9 */
                changed = true;
                break;
              }
              v <<= 1;  /* Shift to next bit */
            }
            /* Update possibilities for all cells - remove placed digit */
            for (ISC j = 0; j < 81; ++j) {
              if (j != i) {
                IUD r = state.possibilities[j];
                IUD c = 1 << (state.board[i] - 1);  /* Bit for placed digit */
                r &= ~c;  /* Clear that bit */
                state.possibilities[j] = r;
              }
            }
          }
        }
      }
    }

    /* L-level: Detailed computation - local cell deduction */
    /* Every odd step (round % 2 == 1), do local cell deduction */
    if (state.round % 2 == 1) {
      for (ISC i = 0; i < 81; ++i) {
        if (state.board[i] == 0 && state.possibilities[i] != 0) {
          IUD p = state.possibilities[i];
          ISC count = 0;
          for (ISC b = 0; b < 9; ++b) if (p & (1 << b)) ++count;
          if (count <= 2) {
            /* Look for forced digit in row/col/box */
            for (ISC d = 1; d <= 9; ++d) {
              if (p & (1 << (d - 1))) {  /* digit d -> bit (d-1) */
                /* Check if digit can only go in one cell within its unit */

                /* Check row */
                ISC row_count = 0;
                ISC row_col_idx = -1;
                ISC r = i / 9;
                for (ISC c2 = 0; c2 < 9; ++c2) {
                  ISC j = r * 9 + c2;
                  if (state.board[j] == 0 && (state.possibilities[j] & (1 << (d - 1)))) {
                    row_count++;
                    row_col_idx = j;
                  }
                }
                if (row_count == 1 && row_col_idx != -1) {
                  state.board[row_col_idx] = d;
                  state.possibilities[row_col_idx] = 0;
                  changed = true;
                  break;
                }

                /* Check column */
                ISC col_count = 0;
                ISC col_row_idx = -1;
                ISC c = i % 9;
                for (ISC r2 = 0; r2 < 9; ++r2) {
                  ISC j = r2 * 9 + c;
                  if (state.board[j] == 0 && (state.possibilities[j] & (1 << (d - 1)))) {
                    col_count++;
                    col_row_idx = j;
                  }
                }
                if (col_count == 1 && col_row_idx != -1) {
                  state.board[col_row_idx] = d;
                  state.possibilities[col_row_idx] = 0;
                  changed = true;
                  break;
                }

                /* Check 3x3 box */
                ISC br = r - r % 3;
                ISC bc = c - c % 3;
                ISC box_count = 0;
                ISC box_idx = -1;
                for (ISC r2 = 0; r2 < 3; ++r2) {
                  for (ISC c2 = 0; c2 < 3; ++c2) {
                    ISC j = (br + r2) * 9 + (bc + c2);
                    if (j != i && state.board[j] == 0 && (state.possibilities[j] & (1 << (d - 1)))) {
                      box_count++;
                      box_idx = j;
                    }
                  }
                }
                if (box_count == 1 && box_idx != -1) {
                  state.board[box_idx] = d;
                  state.possibilities[box_idx] = 0;
                  changed = true;
                  break;
                }
              }
            }
            if (changed) break;
          }
        }
      }
    }

    /* Update state */
    state.steps++;
    state.round++;

    /* Check if puzzle is solved */
    bool solved = true;
    for (ISC i = 0; i < 81; ++i) {
      if (state.board[i] == 0) {
        solved = false;
        break;
      }
    }
    if (solved) {
      /* Copy solution back to board parameter */
      for (ISC i = 0; i < 81; ++i) board[i] = state.board[i];
      return true;
    }
  }

  /* Puzzle not solved within step limit - copy current state */
  for (ISC i = 0; i < 81; ++i) board[i] = state.board[i];
  return false;
}

/* Read a Sudoku puzzle from a file.
 * File format: 81 integers (0-9, 0=empty) separated by whitespace,
 * in row-major order (row 0 first, then row 1, etc.)
 * @param filename  const CHA* - path to puzzle file
 * @param puzzle    IUD*      - output: 81-cell board
 * @return true if puzzle read successfully
 *
 * Uses C stdio (fopen/fscanf) - project convention for file I/O.
 * No C++ standard library streams used.
 */
bool HRMReadPuzzle(const CHA* filename, IUD* puzzle) {
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    /* File not found - return false, caller should handle */
    return false;
  }

  /* Read 81 integer values from file */
  for (ISC i = 0; i < 81; ++i) {
    int val;
    if (fscanf(file, "%d", &val) != 1) {
      /* Failed to read - close and return false */
      fclose(file);
      return false;
    }
    puzzle[i] = val;  /* 0-9, 0 = empty */
  }

  fclose(file);
  return true;
}

/* Print a Sudoku board from flat IUD array.
 * Uses project output conventions
 * @param board const IUD* - 81-cell board to print
 */
void HRMPrintBoard(const IUD* board) {
  for (ISC r = 0; r < 9; ++r) {
    for (ISC c = 0; c < 9; ++c) {
      IUD val = board[r * 9 + c];
      if (val == 0) {
        fputs(".", stdout);
      } else {
        /* Print digit character */
        char digit = char('0' + val);
        char buf[2];
        buf[0] = digit;
        buf[1] = '\0';
        fputs(buf, stdout);
      }
      if ((c + 1) % 3 == 0) {
        fputs(" ", stdout);
      }
    }
    fputs("\n", stdout);
    if ((r + 1) % 3 == 0) {
      fputs("\n", stdout);
    }
  }
  fputs("\n", stdout);
}