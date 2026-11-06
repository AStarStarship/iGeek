// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekSudokuWorldRelease
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace SudokuWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Release(const CHA* args) {
#if _Seam >= _iGeekSudokuWorldRelease
  _ATestBegin;
  _DROut(Headingf("SudokuWorld release seam (full suite)..."));
  core_fails = 0;
  // Re-run the same engine + gym checks as the core pass (the fully sown tree).
  {
    TBoard board;
    SudokuDefaultPuzzle(board);
    if (!SudokuSolve(board, 0) || !SudokuVerify(board)) {
      core_fails += 1;
      ::_::RefOut() << "\nrelease FAIL: default puzzle solve/verify\n";
    }
    ASudokuGym gym(1, 7, 40);
    ScriptedSolve(gym);
    if (gym.Terminals()[0] <= 0.5f || !SudokuVerify(gym.Board(0))) {
      core_fails += 1;
      ::_::RefOut() << "\nrelease FAIL: scripted agent did not reach a valid solution\n";
    }
  }
  if (core_fails != 0) {
    return "sudokuworld_release_test_failure";
  }
  _DROut("  SudokuWorld release: all checks passed\n");
#endif
  return _NilP;
}

}  //< namespace SudokuWorldTest
