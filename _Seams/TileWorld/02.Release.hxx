// Copyright AStarship <https://astarship.net>.
//
// AUTO-GENERATED from __Seams.yaml (iGeek TileWorld).
// TileWorld release seam unit: the final sown test pass. Gated on
// IGEEK_TILEWORLD_RELEASE (SEAM_N). Runs once all lower seams pass.
#if SEAM >= IGEEK_TILEWORLD_RELEASE
#if SEAM == IGEEK_TILEWORLD_RELEASE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif

using namespace ::_;
namespace TileWorldTest {

inline const CHA* Release(const CHA* args) {
#if SEAM >= IGEEK_TILEWORLD_RELEASE
  A_TEST_BEGIN;
  // TileWorld release: runs the full sown tree. No checks of its own.
#endif
  return NILP;
}

}  //< namespace TileWorldTest
