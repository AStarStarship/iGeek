// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekSpaceWorldCore
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace SpaceWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Core(const CHA* args) {
#if _Seam >= _iGeekSpaceWorldCore
  _ATestBegin;
  _DROut(Headingf("Testing SpaceWorld (spacecraft autopilot)..."));

  // Your SpaceWorld core test code here.

#endif
  return _NilP;
}

}  //< namespace SpaceWorldTest
