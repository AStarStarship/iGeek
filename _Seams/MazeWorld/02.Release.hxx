// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekMazeWorldRelease
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace MazeWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Release(const CHA* args) {
#if _Seam >= _iGeekMazeWorldRelease
  _ATestBegin;
  // Reproducible seed; solve and draw the grid via MazeWorld::Print()
  // (ASCIICrabs ROut console primitive — no C/C++ stdlib).
  MazeWorld maze(31, 21, 42);
  ISC path_len = maze.Solve(1, 1, 29, 19);
  if (path_len > 0) maze.Print();
#endif
  return _NilP;
}

}  //< namespace MazeWorldTest
