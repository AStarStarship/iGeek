// Copyright AStarship <https://astarship.net>.
#include "01.Core.hxx"
#include "02.Release.hxx"
namespace SpaceWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* SpaceWorld(const CHA* args) {
  #if _Seam == _iGeekSpaceWorldRelease
    return Release(args);
  #else
    return TTestTree<Core>(args);
  #endif
}
}
