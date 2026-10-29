// Copyright AStarship <https://astarship.net>.
//
// PrisonerWorld release seam unit: the final sown test pass. Runs the
// all-cooperate dilemma simulation (converted from the old _Main.hxx sim
// loop, with the #include <iostream> no-stdlib violation removed in favor
// of ASCIICrabs COut). Gated on IGEEK_PRISONERWORLD_RELEASE (SEAM_N).
#if SEAM >= IGEEK_PRISONERWORLD_RELEASE
#if SEAM == IGEEK_PRISONERWORLD_RELEASE
#include "_Debug.h"
#else
#include "_Release.h"
#endif
#endif
// PrisonerWorld.hxx is pulled ONCE by the rollup _Tests.hxx.
using namespace ::_;
namespace PrisonerWorldTest {

inline const CHA* Release(const CHA* args) {
#if SEAM >= IGEEK_PRISONERWORLD_RELEASE
  A_TEST_BEGIN;
  D_COUT("\nPrisoner's Dilemma Simulation (all-cooperate)\n");

  PrisonerEnv env;
  env.SetPayoffs(5, 3, 1, 0);
  env.Reset();

  IUD actions[2] = {0, 0};
  FPC rewards[2];
  BOL terminals[2];
  ISC episodes = 100;
  for (ISC ep = 0; ep < episodes; ++ep) {
    if (env.Done()) {
      D_COUT("Episode " << ep << " done (history limit reached)\n");
      env.Reset();
    }
    env.Step(actions, rewards, terminals);
    if (ep % 10 == 0) {
      D_COUT("Episode " << ep << ", Round " << env.Round() << "\n");
    }
  }
  D_COUT("\nDone. Total episodes: " << episodes << "\n");
#endif
  return NILP;
}

}  //< namespace PrisonerWorldTest
