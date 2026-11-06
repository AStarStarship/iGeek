// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekBikeWorldRelease
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
// PpoSmoke.hxx is pulled ONCE by the rollup _Tests.hxx (unguarded); this unit
// relies on that rather than re-including it.
namespace BikeWorldTest {
using namespace ::_;
using namespace iGeek; 

inline const CHA* Release(const CHA* args) {
#if _Seam >= _iGeekBikeWorldRelease
  _ATestBegin;
  _DROut(Headingf("BikeWorld release seam (PPO smoke)..."));

  if (::RunAllPpoSmoke() != 0) {
    return "BIKEWORLD_FAIL: PPO smoke failed";
  }
  _DROut("  PPO smoke clean\n");
#endif
  return _NilP;
}

}  //< namespace BikeWorldTest
