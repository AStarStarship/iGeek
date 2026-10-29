// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_HRM_H
#define IGEEK_HRM_H

#include "_Config.h"  // HRM seam config (SEAM = HRM_MODEL, types)

namespace iGeek {

/* Hierarchical Reasoning Model (HRM) for Sudoku solving.
 *
 * HRM architecture:
 * - High-level module (H_level): global constraint propagation (slow,
 *   abstract) — H_cycles iterations across the 9x9 board.
 * - Low-level module (L_level): local cell deduction (rapid, detailed) —
 *   L_cycles iterations of candidate elimination per cell.
 * - Recurrent processing: alternating H and L level updates.
 * - Halting: puzzle solved, or max_steps reached.
 *
 * Sudoku format: 9x9 board, values 1-9 (0 = empty cell).
 * Board: flat 81-cell array.
 *
 * Single-TU build: main.cpp is the only translation unit and includes
 * _Package.hxx for the COut/StdOut + printer impls (the iGeek world pattern).
 */

/* HRM configuration — dual-level processing parameters. */
struct HRMConfig {
  ISC h_cycles;      //< High-level planning cycles (constraint propagation).
  ISC l_cycles;      //< Low-level deduction cycles (candidate elimination).
  ISC hidden_size;   //< Internal representation size.
  FPC learning_rate; //< Learning rate.
  ISC max_steps;     //< Maximum reasoning steps.
  BOL use_rope;      //< Positional encoding flag.
};

/* HRM state — dual-level processing model. */
struct HRMState {
  IUD board[81];         //< 9x9 Sudoku board (0 = empty, 1-9 = filled).
  IUD possibilities[81]; //< Bitmask of possible digits per cell.
  IUD hidden_h[64];      //< High-level hidden state.
  IUD hidden_l[64];      //< Low-level hidden state.
  ISC steps;             //< Reasoning steps taken.
  ISC round;             //< Current H/L round.
};

/* Solve a Sudoku puzzle using the Hierarchical Reasoning Model.
 * Alternates H-level (global constraint propagation) and L-level (local
 * cell deduction); halts when all cells are filled or max_steps is reached.
 * @param board     OUT: 81-cell solution (copied from puzzle, then solved).
 * @param puzzle    IN: 81-cell puzzle (0 = empty).
 * @param max_steps Maximum reasoning steps before halting.
 * @return true if the puzzle was solved, false if max_steps was reached. */
BOL HRMSolve(IUD* board, const IUD* puzzle, ISC max_steps);

/* Read a Sudoku puzzle from a text file (81 whitespace-separated integers,
 * row-major). STUB: returns false — the ASCIICrabs File stack has no read
 * side yet (upstream gap). Callers use the built-in sample puzzle. */
BOL HRMReadPuzzle(const CHA* filename, IUD* puzzle);

/* Print a Sudoku board (formatted 9x9, dots for empty cells) to StdOut.
 * @param board 81-cell board to print. */
void HRMPrintBoard(const IUD* board);

/* Create the HRM environment and solve with the default configuration. */
BOL HRMCreate(IUD* board, const IUD* puzzle);

/* Release HRM resources (no dynamic allocation in this API). */
void HRMRelease(void);

/* Run the HRM solving loop until solved or max_steps reached. */
BOL RunHRMSolving(IUD* board, const IUD* puzzle, ISC max_steps);

} //< namespace iGeek

#endif  // IGEEK_HRM_H
