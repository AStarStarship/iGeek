// Copyright AStarship <https://astarship.net>.
#include "../../MazeWorld/MazeWorld.hxx"
#include "01.Core.hxx"
#include "02.Release.hxx"
namespace MazeWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* MazeWorld(const CHA* args) {
  #if _Seam == _iGeekMazeWorldRelease
    return Release(args);
  #else
    return TTestTree<Core>(args);
  #endif
}
}
