// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekPolygonWorldCore
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace PolygonWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Core(const CHA* args) {
#if _Seam >= _iGeekPolygonWorldCore
  _ATestBegin;
  _DROut(Headingf("Testing PolygonWorld (polygon agent)..."));

  // Your PolygonWorld core test code here.

#endif
  return _NilP;
}

}  //< namespace PolygonWorldTest
