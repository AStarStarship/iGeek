// Copyright AStarship <https://astarship.net>.
#include "01.Core.hxx"
#include "08.Release.hxx"
#include "03.Search.hxx"
#include "04.Csp.hxx"
#include "05.Mdp.hxx"
#include "06.Sat.hxx"
#include "07.Game.hxx"
namespace ClassicalTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Core(const CHA* args) {
  return TTestTree<iGeek::Core>(args);
}

inline const CHA* Classical(const CHA* args) {
  #if _Seam == _iGeekClassicalRelease
    return Release(args);
  #else
    return TTestTree<iGeek::Core, iGeek::Search, iGeek::Csp, iGeek::Mdp, iGeek::Sat, iGeek::GameTest>(args);
  #endif
}
}
