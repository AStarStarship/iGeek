// Copyright AStarship <https://astarship.net>.
//
// AUTO-GENERATED from __Seams.yaml (iGeek PolygonWorld).
// PolygonWorld core seam unit: the unit tests for the deterministic parts of the
// PolygonWorld engine. Gated on IGEEK_POLYGONWORLD_CORE (the first / default seam).
#if SEAM >= IGEEK_POLYGONWORLD_CORE
#if SEAM == IGEEK_POLYGONWORLD_CORE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif

using namespace ::_;
namespace PolygonWorldTest {
using namespace iGeek;   // engine types (in iGeek) used unqualified

inline const CHA* Core(const CHA* args) {
#if SEAM >= IGEEK_POLYGONWORLD_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing PolygonWorld (polygon agent)..."));

  // Your PolygonWorld core test code here.

#endif
  return NILP;
}

}  //< namespace PolygonWorldTest
