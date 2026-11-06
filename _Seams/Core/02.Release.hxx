// Copyright AStarship <https://astarship.net>.
// iGeek core seam unit: the no-op Release (_Seam >= _iGeekRelease).
#include <_Config.h>
#if _Seam >= _iGeekRelease
#include <ASCIICrabs/_Release.h>
#endif
namespace iGeekTest {
using namespace ::_;
using namespace iGeek; 

inline const CHA* Release(const CHA* args) {
  _ATestBegin;
  return _NilP;
}
}  //< namespace iGeekTest
