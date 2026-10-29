// Copyright AStarship <https://astarship.net>.
//
// ArmWorld release seam unit: the final sown test pass. Runs the full ArmWorld
// test suite once all lower seams pass (SEAM == SEAM_N). The heavy PPO training
// tool (ArmworldTrain.hxx → RunTrain) is a separate CLI, not run here — the
// test suite's own PpoSmoke() covers the PPO path.
#if SEAM >= IGEEK_ARMWORLD_RELEASE
#if SEAM == IGEEK_ARMWORLD_RELEASE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif
// ArmworldTests.hxx is pulled ONCE by the rollup _Tests.hxx (unguarded); this
// unit relies on that rather than re-including it.
using namespace ::_;
namespace ArmWorldTest {

inline const CHA* Release(const CHA* args) {
#if SEAM >= IGEEK_ARMWORLD_RELEASE
  A_TEST_BEGIN;
  D_COUT(Headingf("ArmWorld release seam (full suite)..."));

  if (RunAllTests() != 0) {
    return "ARMWORLD_FAIL: release suite reported failures";
  }
  D_COUT("  ArmWorld release suite clean\n");
#endif
  return NILP;
}

}  //< namespace ArmWorldTest
