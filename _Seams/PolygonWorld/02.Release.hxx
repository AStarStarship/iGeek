// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekPolygonWorldRelease
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace PolygonWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Release(const CHA* args) {
#if _Seam >= _iGeekPolygonWorldRelease
  _ATestBegin;
  // PolygonWorld release: runs the full sown tree. No checks of its own.
#endif
  return _NilP;
}

}  //< namespace PolygonWorldTest
