// Copyright AStarship <https://astarship.net>.
//
// AUTO-GENERATED from __Seams.yaml (iGeek PacWorld).
// PacWorld core seam unit: the unit tests for the deterministic parts of the
// PacWorld engine. Gated on IGEEK_PACWORLD_CORE (the first / default seam).
#if SEAM >= IGEEK_PACWORLD_CORE
#if SEAM == IGEEK_PACWORLD_CORE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif

using namespace ::_;
namespace PacWorldTest {
using namespace iGeek;   // engine types (in iGeek) used unqualified

inline const CHA* Core(const CHA* args) {
#if SEAM >= IGEEK_PACWORLD_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing PacWorld (maze + ghosts + pellets)..."));

  // Your PacWorld core test code here.

#endif
  return NILP;
}

}  //< namespace PacWorldTest
