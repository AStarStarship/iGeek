// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekPrisonerWorldCore
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace PrisonerWorldTest {
using namespace ::_;
using namespace iGeek;

// All-cooperate: both agents play 0 (C) every round. After Reset() the
// history is 0 and desired defaults to 0, so each step is C/C -> R=3.
// Verify every step pays R=3 to both agents and the totals account.
inline const CHA* Core(const CHA* args) {
#if _Seam >= _iGeekPrisonerWorldCore
  _ATestBegin;
  _DROut(Headingf("Testing PrisonerWorld (dilemma sim)..."));

  PrisonerEnvironment env;
  env.SetPayoffs(5, 3, 1, 0);  // T=5, R=3, P=1, S=0
  env.Reset();

  // The manifest is the in-Room contract a joining agent reads before it plays.
  {
    const EnvironmentManifest& m = env.Manifest();
    if (!EnvironmentManifestValidate(&m))
      return "PRISONERWORLD_FAIL: manifest does not validate";
    if (!TSEquals(m.name, "prisoner"))
      return "PRISONERWORLD_FAIL: manifest name != prisoner";
    if (m.action_count != 2 || m.observation_count != 2)
      return "PRISONERWORLD_FAIL: manifest obs/action shape wrong";
    if (m.reward_low != FPC(0) || m.reward_high != FPC(5))
      return "PRISONERWORLD_FAIL: manifest reward range != [0,5]";
    if (m.max_episode_steps != ISC(PrisonerEnvironment::MaxHistory))
      return "PRISONERWORLD_FAIL: manifest max_episode_steps wrong";
  }

  IUD actions[2] = {0, 0};
  FPC rewards[2];
  BOL terminals[2];
  ISC episodes = 100;
  ISC r3_steps = 0;
  FPC total[2] = {0, 0};
  for (ISC ep = 0; ep < episodes; ++ep) {
    if (env.Done()) env.Reset();
    env.Step(actions, rewards, terminals);
    if (rewards[0] == 3 && rewards[1] == 3) ++r3_steps;
    total[0] += rewards[0];
    total[1] += rewards[1];
  }

  // Every cooperative step must have paid R=3 to both agents.
  if (r3_steps == 0) {
    return "PRISONERWORLD_FAIL: no cooperative (R=3) steps observed";
  }
  // Total reward per agent must equal 3 * (number of R=3 steps).
  if (total[0] != 3 * r3_steps || total[1] != 3 * r3_steps) {
    return "PRISONERWORLD_FAIL: reward accounting mismatch";
  }

  _DROut("  " << r3_steps << " cooperative steps, total reward "
          << total[0] << " per agent\n");
#endif
  return _NilP;
}

}  //< namespace PrisonerWorldTest
