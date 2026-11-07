// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekMazeWorldCore
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace MazeWorldTest {
using namespace ::_;
using namespace iGeek;

// The deterministic A* solve: the core unit test. Returns _NilP on success,
// an error string on failure (the CI exit-code signal).
inline const CHA* Core(const CHA* args) {
#if _Seam >= _iGeekMazeWorldCore
  _ATestBegin;
  _DROut(Headingf("Testing MazeWorld (A* pathfind)..."));

  // Deterministic seed so the layout + A* path are reproducible.
  const IUD seed = 42;
  const ISC columns = 31, rows = 21;
  MazeWorld maze(columns, rows, seed);

  // A* from start (1,1) to exit (columns-2, rows-2).
  ISC path_len = maze.Solve(1, 1, columns - 2, rows - 2);
  if (path_len <= 0) {
    return "MAZEWORLD_FAIL: A* found no path";
  }

  // The path must be at least the Manhattan distance (a valid path is never
  // shorter than the straight-line distance through open cells).
  ISC manhattan = (columns - 2 - 1) + (rows - 2 - 1);
  if (path_len < manhattan) {
    return "MAZEWORLD_FAIL: path shorter than Manhattan distance";
  }

  _DROut("  A* path length: " << path_len << " (Manhattan lower bound "
          << manhattan << ")\n");
  // --- Environment contract: every World carries a manifest ---
  iGeek::MazeEnv maze_env(columns, rows, seed);
  if (!EnvironmentManifestValidate(&maze_env.Manifest())) {
    return "MAZEWORLD_FAIL: manifest does not validate";
  }
  if (TSEquals(maze_env.Manifest().name, "mazeworld") == _NilP) {
    return "MAZEWORLD_FAIL: manifest name != mazeworld";
  }
  if (maze_env.Manifest().observation_count != ISC(MazeEnv::ObservationLength) ||
      maze_env.Manifest().action_count != ISC(MazeEnv::ActionCount)) {
    return "MAZEWORLD_FAIL: manifest shape mismatch";
  }
#endif
  return _NilP;
}

}  //< namespace MazeWorldTest
