// Copyright AStarship <https://astarship.net>.
#include "../../ArmWorld/ArmGym.hxx"
#include "../../PPO.h"
#include "../../LinearPolicy.hxx"
#include "01.Core.hxx"
#include "02.Release.hxx"
namespace ArmWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* ArmWorld(const CHA* args) {
  #if _Seam == _iGeekArmWorldRelease
    return Release(args);
  #else
    return TTestTree<Core>(args);
  #endif
}
}
