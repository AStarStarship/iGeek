// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekSpaceWorldCore
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#include "../../Worlds/Space/SpaceConfig.h"
#include "../../Worlds/Space/SpaceMath.hxx"
#include "../../Worlds/Space/SpaceRcs.hxx"
#include "../../Worlds/Space/SpacePhysics.hxx"
#include "../../Worlds/Space/SpaceCaps.hxx"
#include "../../Worlds/Space/SpaceEnv.hxx"
namespace SpaceWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Core(const CHA* args) {
#if _Seam >= _iGeekSpaceWorldCore
  _ATestBegin;
  _DROut(Headingf("Testing SpaceWorld (spacecraft autopilot)..."));

  // --- Environment contract: every World carries a manifest ---
  iGeek::SpaceEnv space_env(7);
  _DCheck(EnvironmentManifestValidate(&space_env.Manifest()),
          "SpaceWorld manifest validates");
  _DCheck(TSEquals(space_env.Manifest().name, "spaceworld") != _NilP,
          "SpaceWorld manifest name == spaceworld");
  // The env must actually run the rigid-body step.
  for (ISC i = 0; i < 120; ++i) space_env.Step(i % ISC(SpaceEnv::ActionCount));
  _DCheck(space_env.Steps() >= 120, "SpaceWorld env stepped");
  _DCheck(TSpaceStateValid(space_env.State()), "SpaceWorld state stays finite");


#endif
  return _NilP;
}

}  //< namespace SpaceWorldTest
