// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekTerminalWorldRelease
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace TerminalWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Release(const CHA* args) {
  _ATestBegin;
  // TerminalWorld release: runs the full sown tree. No checks of its own.
  return _NilP;
}

}  //< namespace TerminalWorldTest
