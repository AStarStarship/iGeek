// Copyright AStarship <https://astarship.net>.
#include "01.Core.hxx"
#include "02.Release.hxx"
namespace CarWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* CarWorld(const CHA* args) {
  #if _Seam == _iGeekCarWorldRelease
    return Release(args);
  #else
    return TTestTree<Core>(args);
  #endif
}
}
