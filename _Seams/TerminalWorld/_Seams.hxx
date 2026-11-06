// Copyright AStarship <https://astarship.net>.
//
// The TerminalWorld engine, pulled ONCE here (unguarded .hxx);
// the seam units rely on this rather than re-including it.
#include "../../TerminalWorld/TerminalEnvironment.hxx"
#include "../../EnvironmentManifest.hxx"
#include "01.Core.hxx"
#include "02.Release.hxx"
using namespace ::_;
namespace TerminalWorldTest {
using namespace iGeek;

inline const CHA* TerminalWorld(const CHA* args) {
  #if _Seam == _iGeekTerminalWorldRelease
    return Release(args);
  #else
    return TTestTree<Core>(args);
  #endif
}
}
