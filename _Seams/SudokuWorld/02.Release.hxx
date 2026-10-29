// Copyright AStarship <https://astarship.net>.
//
// SudokuWorld release seam unit: the final sown test pass. Re-runs the full
// core suite (engine + swappable gym) once all lower seams pass. Gated on
// IGEEK_SUDOKUWORLD_RELEASE (SEAM_N).
#if SEAM >= IGEEK_SUDOKUWORLD_RELEASE
#if SEAM == IGEEK_SUDOKUWORLD_RELEASE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif
// SudokuWorld.hxx is pulled ONCE by the rollup _Tests.hxx (unguarded); this
// unit relies on that rather than re-including it.
using namespace ::_;
namespace SudokuWorldTest {

inline const CHA* Release(const CHA* args) {
#if SEAM >= IGEEK_SUDOKUWORLD_RELEASE
  A_TEST_BEGIN;
  D_COUT(Headingf("SudokuWorld release seam (full suite)..."));
  core_fails = 0;
  // Re-run the same engine + gym checks as the core pass (the fully sown tree).
  {
    TBoard board;
    SudokuDefaultPuzzle(board);
    if (!SudokuSolve(board, 0) || !SudokuVerify(board)) {
      core_fails += 1;
      ::_::StdOut() << "\nrelease FAIL: default puzzle solve/verify\n";
    }
    ASudokuGym gym(1, 7, 40);
    ScriptedSolve(gym);
    if (gym.Terminals()[0] <= 0.5f || !SudokuVerify(gym.Board(0))) {
      core_fails += 1;
      ::_::StdOut() << "\nrelease FAIL: scripted agent did not reach a valid solution\n";
    }
  }
  if (core_fails != 0) {
    return "sudokuworld_release_test_failure";
  }
  D_COUT("  SudokuWorld release: all checks passed\n");
#endif
  return NILP;
}

}  //< namespace SudokuWorldTest
