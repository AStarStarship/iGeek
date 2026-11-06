// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekTileWorldCore
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace TileWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Core(const CHA* args) {
#if _Seam >= _iGeekTileWorldCore
  _ATestBegin;
  _DROut(Headingf("Testing TileWorld (tile entities + systems)..."));

  // Your TileWorld core test code here.

#endif
  return _NilP;
}

}  //< namespace TileWorldTest
