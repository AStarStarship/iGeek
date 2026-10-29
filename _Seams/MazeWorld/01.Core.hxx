// Copyright AStarship <https://astarship.net>.
//
// MazeWorld core seam unit: generates a seeded maze, solves it with A*, and
// verifies the path. Converted from the old MazeWorld/_Seams/_Main.hxx
// (which had an int main). Gated on IGEEK_MAZEWORLD_CORE.
// No C/C++ standard library: I/O via COut (ASCIICrabs console primitive).
#if SEAM >= IGEEK_MAZEWORLD_CORE
#if SEAM == IGEEK_MAZEWORLD_CORE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif
// MazeWorld.hxx is pulled ONCE by the rollup _Tests.hxx (it has no include
// guard); this unit relies on that rather than re-including it.
using namespace ::_;
namespace MazeWorldTest {
using namespace iGeek;   // engine types (in iGeek) used unqualified

// The deterministic A* solve: the core unit test. Returns NILP on success,
// an error string on failure (the CI exit-code signal).
inline const CHA* Core(const CHA* args) {
#if SEAM >= IGEEK_MAZEWORLD_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing MazeWorld (A* pathfind)..."));

  // Deterministic seed so the layout + A* path are reproducible.
  const IUD seed = 42;
  const ISC cols = 31, rows = 21;
  MazeWorld maze(cols, rows, seed);

  // A* from start (1,1) to exit (cols-2, rows-2).
  ISC path_len = maze.Solve(1, 1, cols - 2, rows - 2);
  if (path_len <= 0) {
    return "MAZEWORLD_FAIL: A* found no path";
  }

  // The path must be at least the Manhattan distance (a valid path is never
  // shorter than the straight-line distance through open cells).
  ISC manhattan = (cols - 2 - 1) + (rows - 2 - 1);
  if (path_len < manhattan) {
    return "MAZEWORLD_FAIL: path shorter than Manhattan distance";
  }

  D_COUT("  A* path length: " << path_len << " (Manhattan lower bound "
          << manhattan << ")\n");
#endif
  return NILP;
}

}  //< namespace MazeWorldTest
