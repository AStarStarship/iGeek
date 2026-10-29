// Copyright AStarship <https://astarship.net>.
//
// AUTO-GENERATED from __Seams.yaml (iGeek SpaceWorld).
// SpaceWorld release seam unit: the final sown test pass. Gated on
// IGEEK_SPACEWORLD_RELEASE (SEAM_N). Runs once all lower seams pass.
#if SEAM >= IGEEK_SPACEWORLD_RELEASE
#if SEAM == IGEEK_SPACEWORLD_RELEASE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif

using namespace ::_;
namespace SpaceWorldTest {

inline const CHA* Release(const CHA* args) {
#if SEAM >= IGEEK_SPACEWORLD_RELEASE
  A_TEST_BEGIN;
  // SpaceWorld release: runs the full sown tree. No checks of its own.
#endif
  return NILP;
}

}  //< namespace SpaceWorldTest
