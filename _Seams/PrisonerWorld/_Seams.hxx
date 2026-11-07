// Copyright AStarship <https://astarship.net>.
#include "../../Worlds/Prisoner/PrisonerWorld.hxx"
#include "01.Core.hxx"
#include "02.Release.hxx"
namespace PrisonerWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* PrisonerWorld(const CHA* args) {
  #if _Seam == _iGeekPrisonerWorldRelease
    return Release(args);
  #else
    return TTestTree<Core>(args);
  #endif
}
}
