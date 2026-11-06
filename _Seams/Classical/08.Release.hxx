// Copyright AStarship <https://astarship.net>.
// CLASSICAL release seam unit (_Seam >= _iGeekClassicalRelease).
#if _Seam >= _iGeekClassicalRelease
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace ClassicalTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Release(const CHA* args) {
#if _Seam >= _iGeekClassicalRelease
  _ATestBegin;
  // Classical release: runs the full sown tree. No checks of its own.
#endif
  return _NilP;
}

}  //< namespace ClassicalTest
