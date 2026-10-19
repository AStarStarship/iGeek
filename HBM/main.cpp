// Copyright AStarship <https://astarship.net>.
/* Main entry point for HRM Sudoku solver.
 *
 * Usage: ./hrm_sudoku [puzzle_file]
 *   Puzzle file format: 81 integers (0-9, 0=empty) in row-major order
 *   Or use hardcoded sample puzzle if no file specified
 */

#include "hrm.h"
#include <cstdio>

/* Entry point */
int main(int argc, char** argv) {
  /* Parse command line for puzzle file */
  const CHA* puzzle_file = "data/sample_puzzle.txt";
  if (argc > 1) {
    puzzle_file = argv[1];
  }

  /* Read the Sudoku puzzle (flat IUD array of 81 values) */
  IUD puzzle[81];
  if (!HRMReadPuzzle(puzzle_file, puzzle)) {
    /* Use a sample hardcoded puzzle if file not found */
    /* Classic Wikipedia example in row-major order:
     * Row 0: 5 3 . | . 7 . | . . .
     * Row 1: 6 . . | 1 9 5 | . . .
     * Row 2: . 9 8 | . . . | . 6 .
     * Row 3: 8 . . | . 6 . | . . 3
     * Row 4: 4 . . | 8 . 3 | . . 1
     * Row 5: 7 . . | . 2 . | . . 6
     * Row 6: . 6 . | . . . | 2 8 .
     * Row 7: . . . | 4 1 9 | . . 5
     * Row 8: . . . | . 8 . | . 7 9 */
    for (ISC i = 0; i < 81; ++i) {
      static const IUD sample[81] = {
        5, 3, 0, 0, 7, 0, 0, 0, 0,
        6, 0, 0, 1, 9, 5, 0, 0, 0,
        0, 9, 8, 0, 0, 0, 0, 6, 0,
        8, 0, 0, 0, 6, 0, 0, 0, 3,
        4, 0, 0, 8, 0, 3, 0, 0, 1,
        7, 0, 0, 0, 2, 0, 0, 0, 6,
        0, 6, 0, 0, 0, 0, 2, 8, 0,
        0, 0, 0, 4, 1, 9, 0, 0, 5,
        0, 0, 0, 8, 0, 7, 9, 0, 0
      };
      for (ISC i = 0; i < 81; ++i) puzzle[i] = sample[i];
    }
    fputs("Using sample puzzle (classic Wikipedia example)\n", stdout);
  }

  /* Allocate board for solution */
  IUD board[81];

  /* Print the input puzzle */
  fputs("Input Sudoku puzzle:\n", stdout);
  HRMPrintBoard(puzzle);

  /* Run the HRM solver */
  fputs("\nRunning HRM hierarchical reasoning...\n", stdout);
  bool solved = HRMSolve(board, puzzle, 100);

  if (solved) {
    fputs("\nHRM Solved Sudoku:\n", stdout);
    HRMPrintBoard(board);

    /* Verify the solution */
    bool valid = true;
    /* Check all cells filled */
    for (ISC i = 0; i < 81 && valid; ++i) {
      if (board[i] == 0) valid = false;
    }
    /* Check all placements valid */
    for (ISC r = 0; r < 9 && valid; ++r) {
      for (ISC c = 0; c < 9 && valid; ++c) {
        IUD val = board[r * 9 + c];
        if (val != 0) {
          /* Check row */
          for (ISC c2 = 0; c2 < 9; ++c2) {
            if (c2 != c && board[r * 9 + c2] == val) valid = false;
          }
          /* Check column */
          for (ISC r2 = 0; r2 < 9; ++r2) {
            if (r2 != r && board[r2 * 9 + c] == val) valid = false;
          }
          /* Check 3x3 box */
          ISC br = r - r % 3, bc = c - c % 3;
          for (ISC r2 = 0; r2 < 3; ++r2)
            for (ISC c2 = 0; c2 < 3; ++c2) {
              ISC j = (br + r2) * 9 + (bc + c2);
              if ((br + r2 != r || bc + c2 != c) && board[j] == val) valid = false;
            }
        }
      }
    }

    if (valid) {
      fputs("\nSolution verified as valid!\n", stdout);
    } else {
      fputs("\nSolution verification failed.\n", stdout);
    }
  } else {
    fputs("\nHRM could not solve the puzzle within step limit.\n", stdout);
  }

  return 0;
}