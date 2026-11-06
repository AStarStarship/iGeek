// Copyright AStarship <https://astarship.net>.
#include "01.Core.hxx"
#include "02.Release.hxx"
namespace PolygonWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* PolygonWorld(const CHA* args) {
  #if _Seam == _iGeekPolygonWorldRelease
    return Release(args);
  #else
    return TTestTree<Core>(args);
  #endif
}
}
