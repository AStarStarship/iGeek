// Copyright AStarship <https://astarship.net>.
#include "../../Worlds/Bike/Tests.hxx"
#include "../../Worlds/Bike/PpoSmoke.hxx"
#include "01.Core.hxx"
#include "02.Release.hxx"
namespace BikeWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* BikeWorld(const CHA* args) {
  #if _Seam == _iGeekBikeWorldRelease
    return Release(args);
  #else
    return TTestTree<Core>(args);
  #endif
}
}
