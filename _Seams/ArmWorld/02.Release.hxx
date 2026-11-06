// Copyright AStarship <https://astarship.net>.
//
// ArmWorld release seam unit: the final sown test pass. Runs the full ArmWorld
// test suite once all lower seams pass (_Seam == _SeamN). The heavy PPO training
// tool (ArmworldTrain.hxx → RunTrain) is a separate CLI, not run here — the
// test suite's own PpoSmoke() covers the PPO path.
#if _Seam >= _iGeekArmWorldRelease
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
// ArmworldTests.hxx is pulled ONCE by the rollup _Tests.hxx (unguarded); this
// unit relies on that rather than re-including it.
namespace ArmWorldTest {
using namespace ::_;
using namespace iGeek; 

inline const CHA* Release(const CHA* args) {
#if _Seam >= _iGeekArmWorldRelease
  _ATestBegin;
  _DROut(Headingf("ArmWorld release seam (full suite)..."));

  if (RunAllTests() != 0) {
    return "ARMWORLD_FAIL: release suite reported failures";
  }
  _DROut("  ArmWorld release suite clean\n");
#endif
  return _NilP;
}

}  //< namespace ArmWorldTest
