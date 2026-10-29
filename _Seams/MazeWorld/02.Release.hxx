// Copyright AStarship <https://astarship.net>.
//
// MazeWorld release seam unit: the final sown test pass. Solves a seeded maze
// and prints the grid + path (converted from the old _Main.hxx print). Gated
// on IGEEK_MAZEWORLD_RELEASE (SEAM_N). Runs once all lower seams pass.
#if SEAM >= IGEEK_MAZEWORLD_RELEASE
#if SEAM == IGEEK_MAZEWORLD_RELEASE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif
// MazeWorld.hxx is pulled ONCE by the rollup _Tests.hxx (no include guard);
// this unit relies on that rather than re-including it.
using namespace ::_;
namespace MazeWorldTest {

inline const CHA* Release(const CHA* args) {
#if SEAM >= IGEEK_MAZEWORLD_RELEASE
  A_TEST_BEGIN;
  // Reproducible seed; solve and draw the grid via MazeWorld::Print()
  // (ASCIICrabs COut console primitive — no C/C++ stdlib).
  MazeWorld maze(31, 21, 42);
  ISC path_len = maze.Solve(1, 1, 29, 19);
  if (path_len > 0) maze.Print();
#endif
  return NILP;
}

}  //< namespace MazeWorldTest
