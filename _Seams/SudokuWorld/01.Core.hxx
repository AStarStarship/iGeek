// Copyright AStarship <https://astarship.net>.
//
// SudokuWorld core seam unit: unit tests for the deterministic Sudoku engine
// (constraint check, solver, verifier, generator) AND the swappable gym
// (ASudokuGym). The gym is model-agnostic — the tests drive it with a scripted
// policy (solve-by-target) to confirm the interface is agent-independent, NOT
// to validate any one hard-coded model. Gated on IGEEK_SUDOKUWORLD_CORE.
#if SEAM >= IGEEK_SUDOKUWORLD_CORE
#if SEAM == IGEEK_SUDOKUWORLD_CORE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif
// SudokuWorld.hxx is pulled ONCE by the rollup _Tests.hxx (unguarded); this
// unit relies on that rather than re-including it.
using namespace ::_;
namespace SudokuWorldTest {
using namespace iGeek;   // engine types (in iGeek) used unqualified

inline ISC core_fails = 0;
inline void CoreCheck(BOL ok, const CHA* what) {
  if (!::_::Test(ok)) {
    core_fails += 1;
    ::_::StdOut() << "\ncore FAIL: " << what << '\n';
  }
}

// A scripted policy that places the correct digit (from the hidden target) in
// the first empty cell. Proves the gym interface works with a non-learned
// agent — the same interface a PPO policy or HRM would use.
inline void ScriptedSolve(ASudokuGym& gym) {
  for (ISC step = 0; step < SudokuCells * 2; ++step) {
    const TBoard& board = gym.Board(0);
    const TBoard& target = gym.Target(0);
    ISC cell = -1;
    for (ISC i = 0; i < SudokuCells; ++i) {
      if (board[i] == 0) { cell = i; break; }
    }
    if (cell < 0) break;  // board full
    gym.Actions()[0] = cell * (SudokuDigitMax + 1) + ISC(target[cell]);
    gym.StepBatch();
    if (gym.Terminals()[0] > 0.5f) break;
  }
}

inline const CHA* Core(const CHA* args) {
  (void)args;
#if SEAM >= IGEEK_SUDOKUWORLD_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing SudokuWorld (swappable gym + engine)..."));
  core_fails = 0;

  // 1. Constraint check: the default puzzle is consistent (every pre-filled
  //    cell is a valid placement).
  {
    TBoard board;
    SudokuDefaultPuzzle(board);
    BOL consistent = true;
    for (ISC i = 0; i < SudokuCells; ++i) {
      if (board[i] != 0 && !SudokuIsValid(board, i, board[i])) consistent = false;
    }
    CoreCheck(consistent, "default puzzle is internally consistent");
  }

  // 2. Solver: solves the default (Wikipedia) puzzle.
  {
    TBoard board;
    SudokuDefaultPuzzle(board);
    CoreCheck(SudokuSolve(board, 0), "solver finds a solution to the default puzzle");
    CoreCheck(SudokuVerify(board), "solved default puzzle verifies");
  }

  // 3. Verifier: detects a corrupted solution (duplicate in a row).
  {
    TBoard board;
    SudokuDefaultPuzzle(board);
    SudokuSolve(board, 0);
    CoreCheck(SudokuVerify(board), "fresh solution verifies before corruption");
    board[0] = board[1];  // row 0 duplicate
    CoreCheck(!SudokuVerify(board), "verifier rejects a row-duplicate corruption");
  }

  // 4. Verifier: rejects an unfilled board.
  {
    TBoard board;
    SudokuDefaultPuzzle(board);  // has empties
    CoreCheck(!SudokuVerify(board), "verifier rejects an unfilled board");
  }

  // 5. Generator: deterministic per seed, correct hole count, solvable.
  {
    TBoard a, b;
    SudokuGenerate(a, 40, 1234);
    SudokuGenerate(b, 40, 1234);
    BOL same = true;
    for (ISC i = 0; i < SudokuCells; ++i) if (a[i] != b[i]) same = false;
    CoreCheck(same, "generator is deterministic per seed");
    ISC holes = 0;
    for (ISC i = 0; i < SudokuCells; ++i) if (a[i] == 0) ++holes;
    CoreCheck(holes == 40, "generator produces the requested hole count");
    TBoard solved;
    for (ISC i = 0; i < SudokuCells; ++i) solved[i] = a[i];
    CoreCheck(SudokuSolve(solved, 0), "generated puzzle is solvable");
    CoreCheck(SudokuVerify(solved), "generated puzzle's solution verifies");
  }

  // 6. Gym: a scripted (non-learned) agent drives ASudokuGym to a valid
  //    solution — proving the interface is agent-independent.
  {
    ASudokuGym gym(1, 7, 40);
    CoreCheck(gym.EnvCount() == 1, "gym has 1 env");
    CoreCheck(gym.ObsLength() == SudokuCells, "gym obs length is 81");
    ScriptedSolve(gym);
    CoreCheck(gym.Terminals()[0] > 0.5f, "scripted agent completes the episode");
    CoreCheck(SudokuVerify(gym.Board(0)), "scripted agent's board is a valid solution");
    BOL matches = true;
    for (ISC i = 0; i < SudokuCells; ++i) {
      if (gym.Board(0)[i] != gym.Target(0)[i]) matches = false;
    }
    CoreCheck(matches, "scripted agent's board matches the target solution");
  }

  // 7. Gym: an illegal action (placing on a filled cell) is penalized and
  //    does not corrupt the board.
  {
    ASudokuGym gym(1, 7, 40);
    const TBoard& before = gym.Board(0);
    ISC filled = -1;
    for (ISC i = 0; i < SudokuCells; ++i) if (before[i] != 0) { filled = i; break; }
    if (filled >= 0) {
      gym.Actions()[0] = filled * (SudokuDigitMax + 1) + 1;  // overwrite filled cell
      gym.StepBatch();
      CoreCheck(gym.Rewards()[0] < 0.0f, "illegal action (filled cell) is penalized");
      CoreCheck(gym.Board(0)[filled] == before[filled], "illegal action does not corrupt the board");
    }
  }

  // 8. Gym: batch interface — two parallel envs, both step in one call.
  {
    ASudokuGym gym(2, 99, 30);
    CoreCheck(gym.EnvCount() == 2, "gym batches 2 envs");
    gym.Actions()[0] = 0;  // no-op (cell 0, digit 0 = illegal)
    gym.Actions()[1] = 0;
    gym.StepBatch();
    CoreCheck(true, "batch StepBatch() steps all envs in one call");
  }

  if (core_fails != 0) {
    ::_::StdOut() << "\nSudokuWorld core: " << core_fails << " check(s) FAILED\n";
    return "sudokuworld_core_test_failure";
  }
  D_COUT("  SudokuWorld core: all checks passed\n");
#endif
  return NILP;
}

}  //< namespace SudokuWorldTest
