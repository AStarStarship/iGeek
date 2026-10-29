// Copyright AStarship <https://astarship.net>.
//
// AUTO-GENERATED from __Seams.yaml (iGeek CLASSICAL).
// CLASSICAL release seam unit: the final sown test pass. Gated on
// IGEEK_CLASSICAL_RELEASE (SEAM_N). Runs once all lower seams pass.
#if SEAM >= IGEEK_CLASSICAL_RELEASE
#if SEAM == IGEEK_CLASSICAL_RELEASE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif

using namespace ::_;
namespace ClassicalTest {

inline const CHA* Release(const CHA* args) {
#if SEAM >= IGEEK_CLASSICAL_RELEASE
  A_TEST_BEGIN;
  // CLASSICAL release: runs the full sown tree. No checks of its own.
#endif
  return NILP;
}

}  //< namespace ClasicalTest
