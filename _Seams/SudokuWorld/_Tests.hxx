// Copyright AStarship <https://astarship.net>.
#include "../../Worlds/Sudoku/SudokuWorld.hxx"
#include "01.Core.hxx"
#include "02.Release.hxx"
namespace SudokuWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* SudokuWorld(const CHA* args) {
  #if _Seam == _iGeekSudokuWorldRelease
    return Release(args);
  #else
    return TTestTree<Core>(args);
  #endif
}
}
