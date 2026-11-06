// Copyright AStarship <https://astarship.net>.
#include "01.Core.hxx"
#include "02.Release.hxx"
using namespace ::_;
namespace TileWorldTest {
using namespace iGeek;

inline const CHA* TileWorld(const CHA* args) {
  #if _Seam == _iGeekTileWorldRelease
    return Release(args);
  #else
    return TTestTree<Core>(args);
  #endif
}
}
