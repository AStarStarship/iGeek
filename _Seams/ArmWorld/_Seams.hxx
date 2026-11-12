// Copyright AStarship <https://astarship.net>.
#include "../../Worlds/Arm/ArmGym.hxx"
#include "../../PPO.h"
#include "../../LinearPolicy.hxx"
#include "01.Core.hxx"
#include "02.Release.hxx"
namespace ArmWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* ArmWorld(const CHA* args) {
  #if _Seam >= _iGeekArmWorldRelease
    /* At or past the module's release seam, the node's official output is the
     * Release() unit (the Arm2D demo). Below it, the core suite runs. */
    return Release(args);
  #else
    return TTestTree<Core>(args);
  #endif
}
}
