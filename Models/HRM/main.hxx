// Copyright AStarship <https://astarship.net>.
/* Main entry point for the HRM (Hierarchical Reasoning Model) Sudoku solver.
 *
 * Usage: ./HRM [puzzle_file]
 *   Puzzle file format: 81 integers (0-9, 0=empty) in row-major order.
 *   (File loading is pending the ASCIICrabs File read-side gap — until
 *   then the built-in sample puzzle is used; see HRMReadPuzzle.)
 *
 * Single-TU build: this file is the only place the ASCIICrabs _Package.hxx
 * impls are compiled (COut/StdOut + printer chain); HRM.cpp is declarations
 * plus logic only.
 */

#include "HRM.h"
#include "../../../ASCIICrabs/_Package.hxx"  // COut/StdOut + printer impls (single-TU)
#include "HRM.hxx"  // HRM solver impls (same TU)

namespace iGeek {

/* Built-in sample puzzle (classic Wikipedia example, row-major, 0 = empty). */
const IUD kSamplePuzzle[81] = {
    5, 3, 0, 0, 7, 0, 0, 0, 0,
    6, 0, 0, 1, 9, 5, 0, 0, 0,
    0, 9, 8, 0, 0, 0, 0, 6, 0,
    8, 0, 0, 0, 6, 0, 0, 0, 3,
    4, 0, 0, 8, 0, 3, 0, 0, 1,
    7, 0, 0, 0, 2, 0, 0, 0, 6,
    0, 6, 0, 0, 0, 0, 2, 8, 0,
    0, 0, 0, 4, 1, 9, 0, 0, 5,
    0, 0, 0, 8, 0, 7, 9, 0, 0};

/* Verify a completed board: no row/col/box repeats. */
BOL VerifyBoard(const IUD* board) {
  for (ISC i = 0; i < 81; ++i) {
    if (board[i] == 0) return false;
  }
  for (ISC r = 0; r < 9; ++r) {
    for (ISC c = 0; c < 9; ++c) {
      IUD val = board[r * 9 + c];
      for (ISC c2 = 0; c2 < 9; ++c2) {
        if (c2 != c && board[r * 9 + c2] == val) return false;
      }
      for (ISC r2 = 0; r2 < 9; ++r2) {
        if (r2 != r && board[r2 * 9 + c] == val) return false;
      }
      ISC br = r - r % 3, bc = c - c % 3;
      for (ISC r2 = 0; r2 < 3; ++r2) {
        for (ISC c2 = 0; c2 < 3; ++c2) {
          ISC j = (br + r2) * 9 + (bc + c2);
          if ((br + r2 != r || bc + c2 != c) && board[j] == val) return false;
        }
      }
    }
  }
  return true;
}

} //< namespace iGeek

int main(int argc, char** argv) {
  using namespace _;
  COut& out = StdOut();

  IUD puzzle[81];
  BOL loaded = false;
  if (argc > 1) {
    loaded = HRMReadPuzzle(argv[1], puzzle);
  }
  if (!loaded) {
    for (ISC i = 0; i < 81; ++i) puzzle[i] = kSamplePuzzle[i];
    out << "Using sample puzzle (classic Wikipedia example)\n";
  }

  IUD board[81];

  out << "Input Sudoku puzzle:\n";
  HRMPrintBoard(puzzle);

  out << "\nRunning HRM hierarchical reasoning...\n";
  BOL solved = HRMSolve(board, puzzle, 100);

  if (solved) {
    out << "\nHRM Solved Sudoku:\n";
    HRMPrintBoard(board);
    if (VerifyBoard(board)) {
      out << "\nSolution verified as valid!\n";
    } else {
      out << "\nSolution verification failed.\n";
    }
  } else {
    out << "\nHRM could not solve the puzzle within step limit.\n";
  }

  return 0;
}
