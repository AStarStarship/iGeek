// Copyright AStarship <https://astarship.net>.
/* Hierarchical Reasoning Model for Sudoku solving.
 *
 * HRM Architecture (from 3P/HRM Python reference):
 * - High-level module (H_level): Global constraint propagation (slow, abstract)
 *   - H_cycles=4 iterations of constraint propagation across 9x9 board
 * - Low-level module (L_level): Local cell deduction (rapid, detailed)
 *   - L_cycles=8 iterations of candidate elimination per cell
 * - Recurrent processing: Alternating H and L level updates
 * - Halting: When puzzle solved or max_steps reached
 *
 * Sudoku format: 9x9 board, values 1-9 (0 = empty cell)
 * Board: flat 81-cell array, IUD bit-packed history
 * All types from iGeek/ASCIICrabs project convention
 */

#pragma once
#ifndef IGEEK_HBM_HRM_H
#define IGEEK_HBM_HRM_H

#include <_ConfigHeader.h>  /* Project types: IUD, ISC, FPC, CHA */

/* Type shortcuts from project */
typedef uint64_t IUD;     /* 8-byte unsigned integer (from ASCIICrabs/_ConfigHeader.h) */
typedef int32_t  ISC;     /* 4-byte signed integer */
typedef float    FPC;     /* 4-byte floating-point */

/* HRM Configuration - dual-level processing parameters */
struct HRMConfig {
  ISC H_cycles;    /* High-level planning cycles (abstract constraint propagation) */
  ISC L_cycles;    /* Low-level deduction cycles (local cell deduction) */
  ISC hidden_size; /* Internal representation size */
  FPC learning_rate;  /* Learning rate */
  ISC max_steps;   /* Maximum reasoning steps */
  bool use_rope;   /* Positional encoding flag */
};

/* HRM State - dual-level processing model */
struct HRMState {
  IUD board[81];     /* 9x9 Sudoku board (0 = empty, 1-9 = filled) */
  IUD possibilities[81]; /* Bitmask of possible digits per cell (bits 0-8 = digits 1-9) */
  IUD hidden_H[64];  /* High-level hidden state */
  IUD hidden_L[64];  /* Low-level hidden state */
  ISC steps;         /* Reasoning steps taken */
  ISC round;         /* Current H/L round */
};

/* Solve a Sudoku puzzle using Hierarchical Reasoning Model.
 * Performs H/L level recurrent processing:
 * - Alternates H-level (global constraint propagation, every 2 steps)
 *   and L-level (local cell deduction, every odd step)
 * - Halts when all cells filled (solved) or max_steps reached
 * @param board   Output: flat IUD array of 81 cells (will be filled with solution)
 * @param puzzle  Input: flat IUD array of 81 values (0-9, 0=empty)
 * @param max_steps Maximum reasoning steps before halting
 * @return true if puzzle solved, false if max_steps reached
 * Implementation is in HRM.cpp
 */
bool HRMSolve(IUD* board, const IUD* puzzle, ISC max_steps);

/* Read a Sudoku puzzle from a file.
 * File format: 81 integers (0-9, 0=empty) separated by whitespace,
 * in row-major order (row 0 first, then row 1, etc.)
 * @param filename  const CHA* - path to puzzle file
 * @param puzzle    IUD*      - output: 81-cell board
 * @return true if puzzle read successfully
 *
 * Uses C stdio (fopen/fread) - project convention for file I/O.
 * No C++ standard library streams used.
 */
bool HRMReadPuzzle(const CHA* filename, IUD* puzzle);

/* Print a Sudoku board from flat IUD array.
 * Uses std::cout for output (project convention when SEAM context not active)
 * @param board const IUD* - 81-cell board to print
 */
void HRMPrintBoard(const IUD* board);

#endif  // IGEEK_HBM_HRM_H