// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekCarWorldRelease
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace CarWorldTest {
using namespace ::_;
using namespace iGeek; 

inline const CHA* Release(const CHA* args) {
#if _Seam >= _iGeekCarWorldRelease
  _ATestBegin;
  // CarWorld release: runs the full sown tree. No checks of its own.
#endif
  return _NilP;
}

}  //< namespace CarWorldTest
