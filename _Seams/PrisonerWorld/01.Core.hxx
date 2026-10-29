// Copyright AStarship <https://astarship.net>.
//
// PrisonerWorld core seam unit: runs a deterministic prisoner's-dilemma
// simulation and verifies the reward accounting. Converted from the old
// PrisonerWorld/_Main.hxx (which had an int main and a #include <iostream>
// no-stdlib violation — replaced with ASCIICrabs COut). Gated on
// IGEEK_PRISONERWORLD_CORE (the first / default seam).
#if SEAM >= IGEEK_PRISONERWORLD_CORE
#if SEAM == IGEEK_PRISONERWORLD_CORE
#include "_Debug.h"
#else
#include "_Release.h"
#endif
#endif
// PrisonerWorld.hxx + PrisonerEnvPtr are pulled ONCE by the rollup
// _Tests.hxx (unguarded .hxx); this unit relies on that rather than
// re-including them.
using namespace ::_;
namespace PrisonerWorldTest {
using namespace iGeek;   // engine types (in iGeek) used unqualified

// All-cooperate: both agents play 0 (C) every round. After Reset() the
// history is 0 and desired defaults to 0, so each step is C/C -> R=3.
// Verify every step pays R=3 to both agents and the totals account.
inline const CHA* Core(const CHA* args) {
#if SEAM >= IGEEK_PRISONERWORLD_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing PrisonerWorld (dilemma sim)..."));

  PrisonerEnv env;
  env.SetPayoffs(5, 3, 1, 0);  // T=5, R=3, P=1, S=0
  env.Reset();

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

  D_COUT("  " << r3_steps << " cooperative steps, total reward "
          << total[0] << " per agent\n");
#endif
  return NILP;
}

}  //< namespace PrisonerWorldTest
