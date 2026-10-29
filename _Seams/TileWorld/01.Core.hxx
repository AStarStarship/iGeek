// Copyright AStarship <https://astarship.net>.
//
// AUTO-GENERATED from __Seams.yaml (iGeek TileWorld).
// TileWorld core seam unit: the unit tests for the deterministic parts of the
// TileWorld engine. Gated on IGEEK_TILEWORLD_CORE (the first / default seam).
#if SEAM >= IGEEK_TILEWORLD_CORE
#if SEAM == IGEEK_TILEWORLD_CORE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif

using namespace ::_;
namespace TileWorldTest {
using namespace iGeek;   // engine types (in iGeek) used unqualified

inline const CHA* Core(const CHA* args) {
#if SEAM >= IGEEK_TILEWORLD_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing TileWorld (tile entities + systems)..."));

  // Your TileWorld core test code here.

#endif
  return NILP;
}

}  //< namespace TileWorldTest
