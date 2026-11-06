// Copyright AStarship <https://astarship.net>.
#include "../../TypingWorld/TypingWorld.hxx"
#include "01.Core.hxx"
#include "02.Release.hxx"
using namespace ::_;
namespace TypingWorldTest {
using namespace iGeek;

inline const CHA* TypingWorld(const CHA* args) {
  #if _Seam == _iGeekTypingWorldRelease
    return Release(args);
  #else
    return TTestTree<Core>(args);
  #endif
}
}
