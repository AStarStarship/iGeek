// Copyright AStarship <https://astarship.net>.
//
// BikeWorld release seam unit: the final sown test pass — the PPO training
// smoke (BikeWorld/PpoSmoke.hxx, old main() converted to RunAllPpoSmoke()).
// Runs once all lower seams pass. Gated on IGEEK_BIKEWORLD_RELEASE (SEAM_N).
#if SEAM >= IGEEK_BIKEWORLD_RELEASE
#if SEAM == IGEEK_BIKEWORLD_RELEASE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif
// PpoSmoke.hxx is pulled ONCE by the rollup _Tests.hxx (unguarded); this unit
// relies on that rather than re-including it.
using namespace ::_;
namespace BikeWorldTest {

inline const CHA* Release(const CHA* args) {
#if SEAM >= IGEEK_BIKEWORLD_RELEASE
  A_TEST_BEGIN;
  D_COUT(Headingf("BikeWorld release seam (PPO smoke)..."));

  if (::RunAllPpoSmoke() != 0) {
    return "BIKEWORLD_FAIL: PPO smoke failed";
  }
  D_COUT("  PPO smoke clean\n");
#endif
  return NILP;
}

}  //< namespace BikeWorldTest
