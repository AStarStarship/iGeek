// Copyright AStarship <https://astarship.net>.
//
// AUTO-GENERATED from __Seams.yaml (iGeek PolygonWorld).
// PolygonWorld release seam unit: the final sown test pass. Gated on
// IGEEK_POLYGONWORLD_RELEASE (SEAM_N). Runs once all lower seams pass.
#if SEAM >= IGEEK_POLYGONWORLD_RELEASE
#if SEAM == IGEEK_POLYGONWORLD_RELEASE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif

using namespace ::_;
namespace PolygonWorldTest {

inline const CHA* Release(const CHA* args) {
#if SEAM >= IGEEK_POLYGONWORLD_RELEASE
  A_TEST_BEGIN;
  // PolygonWorld release: runs the full sown tree. No checks of its own.
#endif
  return NILP;
}

}  //< namespace PolygonWorldTest
