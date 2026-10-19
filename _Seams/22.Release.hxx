// Copyright AStarship <https://astarship.net>.
// iGeek core seam unit: the no-op Release (SEAM == IGEEK_RELEASE).
#include <_Config.h>
#if SEAM >= IGEEK_RELEASE
#include "../../ASCIICrabs/_Release.h"
#endif
namespace _ {
inline const CHA* Release(const CHA* args) {
  (void)args;
  A_TEST_BEGIN;
  return NILP;
}
}  //< namespace _
