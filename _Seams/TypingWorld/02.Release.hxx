// Copyright AStarship <https://astarship.net>.
// 
// TypingWorld release seam unit: the final sown test pass. Gated on
// IGEEK_TYPINGWORLD_RELEASE (SEAM_N). Runs once all lower seams pass.
// No C/C++ standard library: I/O via COut (ASCIICrabs console primitive).
#if SEAM >= IGEEK_TYPINGWORLD_RELEASE
#if SEAM == IGEEK_TYPINGWORLD_RELEASE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif

using namespace ::_;
namespace TypingWorldTest {

inline const CHA* Release(const CHA* args) {
#if SEAM >= IGEEK_TYPINGWORLD_RELEASE
  A_TEST_BEGIN;
  // TypingWorld release: runs the full sown tree. No checks of its own.
#endif
  return NILP;
}

}  //< namespace TypingWorldTest