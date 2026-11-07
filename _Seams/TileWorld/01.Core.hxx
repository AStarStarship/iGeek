// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekTileWorldCore
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#include "../../Worlds/Tile/Tile.hxx"
#include "../../Worlds/Tile/Entity.hxx"
#include "../../Worlds/Tile/TileMap.hxx"
#include "../../Worlds/Tile/TileEnv.hxx"
namespace TileWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Core(const CHA* args) {
#if _Seam >= _iGeekTileWorldCore
  _ATestBegin;
  _DROut(Headingf("Testing TileWorld (tile entities + systems)..."));

  // --- Environment contract: every World carries a manifest ---
  iGeek::TileEnv tile_env(7);
  _DCheck(EnvironmentManifestValidate(&tile_env.Manifest()),
          "TileWorld manifest validates");
  _DCheck(TSEquals(tile_env.Manifest().name, "tileworld") != _NilP,
          "TileWorld manifest name == tileworld");
  // The env must actually run: take a few steps and stay inside the grid.
  for (ISC i = 0; i < 64; ++i) tile_env.Step(i % ISC(TileEnv::ActionCount));
  _DCheck(tile_env.Steps() >= 64, "TileWorld env stepped");
  _DCheck(tile_env.Map().GridSize() > 0.0f, "TileWorld map has a cell size");


#endif
  return _NilP;
}

}  //< namespace TileWorldTest
