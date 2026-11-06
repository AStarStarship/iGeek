// Copyright AStarship <https://astarship.net>.
#include "../../PacWorld/PacWorld.hxx"
#include "01.Core.hxx"
#include "02.Release.hxx"
namespace PacWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* PacWorld(const CHA* args) {
  #if _Seam == _iGeekPackWorldRelease
    return Release(args);
  #else
    return TTestTree<Core>(args);
  #endif
}
}
