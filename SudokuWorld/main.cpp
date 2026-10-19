// Copyright AStarship <https://astarship.net>.
/* Sudoku World - Main entry point.
 * Uses project-types only (IUD, ISC from ASCIICrabs/iGeek).
 * No C++ standard library code.
 * Solves Sudoku using backtracking algorithm.
 */

#include "hrm.h"
#include <cstdio>

/* 9x9 Sudoku board: flat IUD array, 0 = empty cell */
typedef IUD Board[81];

/* Check if placing digit d at cell index i is valid */
bool IsValidPlacement(const Board board, ISC i, IUD d) {
  ISC r = i / 9;
  ISC c = i % 9;

  /* Check row */
  for (ISC c2 = 0; c2 < 9; ++c2) {
    ISC j = r * 9 + c2;
    if (c2 != c && board[j] == d) return false;
  }

  /* Check column */
  for (ISC r2 = 0; r2 < 9; ++r2) {
    ISC j = r2 * 9 + c;
    if (r2 != r && board[j] == d) return false;
  }

  /* Check 3x3 box */
  ISC br = r - r % 3;
  ISC bc = c - c % 3;
  for (ISC r2 = 0; r2 < 3; ++r2) {
    for (ISC c2 = 0; c2 < 3; ++c2) {
      ISC j = (br + r2) * 9 + (bc + c2);
      if ((br + r2 != r || bc + c2 != c) && board[j] == d) return false;
    }
  }

  return true;
}

/* Solve Sudoku using backtracking */
bool SolveSudoku(Board board, ISC i) {
  /* Move to next cell */
  if (i >= 81) return true;

  /* If cell is already filled, move to next */
  if (board[i] != 0) return SolveSudoku(board, i + 1);

  /* Try digits 1-9 */
  for (IUD d = 1; d <= 9; ++d) {
    if (IsValidPlacement(board, i, d)) {
      board[i] = d;
      if (SolveSudoku(board, i + 1)) return true;
      board[i] = 0;  /* backtrack */
    }
  }

  return false;  /* trigger backtracking */
}

/* Print a Sudoku board */
void PrintBoard(const Board board) {
  for (ISC r = 0; r < 9; ++r) {
    for (ISC c = 0; c < 9; ++c) {
      IUD val = board[r * 9 + c];
      if (val == 0) {
        fputs(".", stdout);
      } else {
        /* Print digit character */
        char buf[2];
        buf[0] = char('0' + val);
        buf[1] = '\0';
        fputs(buf, stdout);
      }
      if ((c + 1) % 3 == 0) fputs(" ", stdout);
    }
    fputs("\n", stdout);
    if ((r + 1) % 3 == 0) fputs("\n", stdout);
  }
  fputs("\n", stdout);
}

/* Verify the solution is valid */
bool VerifySolution(const Board board) {
  bool valid = true;

  /* Check all cells filled */
  for (ISC i = 0; i < 81 && valid; ++i) {
    if (board[i] == 0) valid = false;
  }

  /* Check all placements valid */
  if (valid) {
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
          ISC br = r - r % 3;
          ISC bc = c - c % 3;
          for (ISC r2 = 0; r2 < 3; ++r2) {
            for (ISC c2 = 0; c2 < 3; ++c2) {
              ISC j = (br + r2) * 9 + (bc + c2);
              if ((br + r2 != r || bc + c2 != c) && board[j] == val) valid = false;
            }
          }
        }
      }
    }
  }

  return valid;
}

/* Entry point */
int main(int argc, char** argv) {
  /* Use classic Wikipedia puzzle as default */
  Board board;

  /* Classic Wikipedia puzzle values */
  for (ISC i = 0; i < 81; ++i) {
    static const IUD default_puzzle[81] = {
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
    board[i] = default_puzzle[i];
  }

  fputs("Input Sudoku puzzle:\n", stdout);

  /* Print the puzzle */
  for (ISC r = 0; r < 9; ++r) {
    for (ISC c = 0; c < 9; ++c) {
      IUD val = board[r * 9 + c];
      if (val == 0) {
        fputs(".", stdout);
      } else {
        char buf[2];
        buf[0] = char('0' + val);
        buf[1] = '\0';
        fputs(buf, stdout);
      }
      if ((c + 1) % 3 == 0) fputs(" ", stdout);
    }
    fputs("\n", stdout);
    if ((r + 1) % 3 == 0) fputs("\n", stdout);
  }
  fputs("\n", stdout);

  fputs("\nSolving Sudoku using backtracking...\n", stdout);

  /* Solve the puzzle using inline backtracking */
  bool solved = false;
  ISC i = 0;
  while (i < 81) {
    if (board[i] != 0) {
      i++;
    } else {
      bool placed = false;
      for (IUD d = 1; d <= 9 && !placed; ++d) {
        ISC r = i / 9;
        ISC c = i % 9;
        bool valid = true;

        /* Check row */
        for (ISC c2 = 0; c2 < 9 && valid; ++c2) {
          if (c2 != c && board[r * 9 + c2] == d) valid = false;
        }
        /* Check column */
        for (ISC r2 = 0; r2 < 9 && valid; ++r2) {
          if (r2 != r && board[r2 * 9 + c] == d) valid = false;
        }
        /* Check 3x3 box */
        ISC br = r - r % 3;
        ISC bc = c - c % 3;
        for (ISC r2 = 0; r2 < 3 && valid; ++r2) {
          for (ISC c2 = 0; c2 < 3 && valid; ++c2) {
            ISC j = (br + r2) * 9 + (bc + c2);
            if ((br + r2 != r || bc + c2 != c) && board[j] == d) valid = false;
          }
        }
        if (valid) {
          board[i] = d;
          placed = true;
        }
      }
      if (!placed) {
        board[i] = 0;
        i--;
        if (i < 0) break;
      } else {
        i++;
        if (i >= 81) solved = true;
      }
    }
  }

  if (solved) {
    fputs("\nSolved Sudoku:\n", stdout);
    /* Print the solved board */
    for (ISC r = 0; r < 9; ++r) {
      for (ISC c = 0; c < 9; ++c) {
        IUD val = board[r * 9 + c];
        if (val == 0) {
          fputs(".", stdout);
        } else {
          char buf[2];
          buf[0] = char('0' + val);
          buf[1] = '\0';
          fputs(buf, stdout);
        }
        if ((c + 1) % 3 == 0) fputs(" ", stdout);
      }
      fputs("\n", stdout);
      if ((r + 1) % 3 == 0) fputs("\n", stdout);
    }
    fputs("\n", stdout);

    /* Verify the solution */
    bool valid = VerifySolution(board);

    if (valid) {
      fputs("\nSolution verified as valid!\n", stdout);
    } else {
      fputs("\nSolution verification failed.\n", stdout);
    }
  } else {
    fputs("\nNo solution found.\n", stdout);
  }

  return 0;
}