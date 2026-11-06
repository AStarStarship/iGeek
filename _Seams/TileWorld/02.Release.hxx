// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekTileWorldRelease
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace TileWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Release(const CHA* args) {
#if _Seam >= _iGeekTileWorldRelease
  _ATestBegin;
  // TileWorld release: runs the full sown tree. No checks of its own.
#endif
  return _NilP;
}

}  //< namespace TileWorldTest
