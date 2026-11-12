// Copyright AStarship <https://astarship.net>.
// Compile against the verbatim pure-board slice of SudokuWorld.hxx.
// The loader generates SudokuBoardNative.hxx in scratch, never in this repo.
// Full Gym.h inclusion is blocked by unrelated ASCIICrabs dependencies.
#include <cstdint>
namespace iGeek {
using IUD = std::uint64_t;
using ISC = std::int32_t;
using BOL = bool;
#include "SudokuBoardNative.hxx"
}

extern "C" {
void SudokuBridgeDefault(std::uint64_t* board) {
  iGeek::SudokuDefaultPuzzle(board);
}
int SudokuBridgeValid(const std::uint64_t* board, int cell, int digit) {
  return cell >= 0 && cell < 81 && digit >= 1 && digit <= 9 &&
      iGeek::SudokuIsValid(board, cell, digit);
}
int SudokuBridgeVerify(const std::uint64_t* board) {
  return iGeek::SudokuVerify(board);
}
int SudokuBridgeSolve(std::uint64_t* board) {
  // Native SudokuSolve assumes consistent inputs; enforce that at the ABI.
  iGeek::TBoard work;
  for (int cell = 0; cell < 81; ++cell) {
    if (board[cell] > 9 || (board[cell] &&
        !iGeek::SudokuIsValid(board, cell, board[cell]))) return 0;
    work[cell] = board[cell];
  }
  if (!iGeek::SudokuSolve(work, 0) || !iGeek::SudokuVerify(work)) return 0;
  for (int cell = 0; cell < 81; ++cell) board[cell] = work[cell];
  return 1;
}
}
