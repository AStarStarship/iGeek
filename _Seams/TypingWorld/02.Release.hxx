// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekTypingWorldRelease
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace TypingWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Release(const CHA* args) {
#if _Seam >= _iGeekTypingWorldRelease
  _ATestBegin;
  // TypingWorld release: runs the full sown tree. No checks of its own.
#endif
  return _NilP;
}

}  //< namespace TypingWorldTest