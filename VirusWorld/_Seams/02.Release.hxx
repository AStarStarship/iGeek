// Copyright AStarship <https://astarship.net>.
#if SEAM >= VIREWWORLD_RELEASE
#if SEAM == VIREWWORLD_RELEASE
#include "../_Debug.h"
#else
#include "../_Release.h"
#endif
#endif
namespace VireworldTest {

inline const CHA* Release(const CHA* args) {
#if SEAM == VIREWWORLD_RELEASE
#endif
  return NILP;
}

}  //< namespace VireworldTest
