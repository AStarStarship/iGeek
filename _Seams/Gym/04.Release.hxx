// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekEnvironmentRelease
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace GymTest {
using namespace ::_;
using namespace iGeek;   // engine types (in iGeek) used unqualified

inline const CHA* Release(const CHA* args) {
#if _Seam >= _iGeekEnvironmentRelease
  _ATestBegin;
  _DROut(Headingf("Testing Gym Release..."));

  // Your SpaceWorld core test code here.

#endif
  return _NilP;
}

}  //< namespace GymTest
