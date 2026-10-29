// Copyright AStarship <https://astarship.net>.
//
// AUTO-GENERATED from __Seams.yaml (iGeek SpaceWorld).
// SpaceWorld core seam unit: the unit tests for the deterministic parts of the
// SpaceWorld engine. Gated on IGEEK_SPACEWORLD_CORE (the first / default seam).
#if SEAM >= IGEEK_SPACEWORLD_CORE
#if SEAM == IGEEK_SPACEWORLD_CORE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif

using namespace ::_;
namespace SpaceWorldTest {
using namespace iGeek;   // engine types (in iGeek) used unqualified

inline const CHA* Core(const CHA* args) {
#if SEAM >= IGEEK_SPACEWORLD_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing SpaceWorld (spacecraft autopilot)..."));

  // Your SpaceWorld core test code here.

#endif
  return NILP;
}

}  //< namespace SpaceWorldTest
