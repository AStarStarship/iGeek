// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekPolygonWorldCore
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#if _Seam >= _iGeekPolygonWorldCore
#include "../../Worlds/Polygon/GeneticPolygon.hxx"
#endif
namespace PolygonWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Core(const CHA* args) {
#if _Seam >= _iGeekPolygonWorldCore
  _ATestBegin;
  _DROut(Headingf("Testing PolygonWorld (GA polygon vs. target)..."));

  // --- manifest sanity -----------------------------------------------------
  {
    PolygonWorld::APolygonEnv env(1);
    _DCheck(EnvironmentManifestValidate(&env.Manifest()), "manifest validates");
    _DCheck(TSEquals(env.Manifest().name, "polygonworld") != _NilP, "name == polygonworld");
    _DCheck(env.Manifest().observation_count == PolygonWorld::PolygonWorldObservationCount,
           "observation_count == 32");
    _DCheck(env.Manifest().action_count == PolygonWorld::PolygonWorldActionCount,
           "action_count == 4");
  }

  // --- the GA actually converges to the target -----------------------------
  {
    PolygonWorld::APolygonEnv env(42);
    ISC total_reward = 0;
    ISC steps = 0;
    while (!env.Done() && steps < 200) {
      total_reward += (env.Step(0) > 0.0f) ? 1 : 0;
      ++steps;
    }
    _DCheck(env.Done(), "episode terminates within the step budget");
    _DCheck(env.Population().BestFitness() > 0.9f, "GA converges to the target");
    _DCheck(steps <= PolygonWorld::PolygonWorldMaxSteps, "step budget respected");
  }

  // --- determinism: same seed, same run ------------------------------------
  {
    PolygonWorld::APolygonEnv a(7), b(7);
    for (ISC i = 0; i < 20 && !a.Done(); ++i) { a.Step(0); b.Step(0); }
    FPC da = 0.0f;
    for (ISC i = 0; i < PolygonWorld::PolygonWorldObservationCount; ++i)
      da += (a.Observation()[i] - b.Observation()[i]);
    _DCheck(da < 0.001f && da > -0.001f, "same seed reproduces the same run");
  }

#endif
  return _NilP;
}

}  //< namespace PolygonWorldTest
