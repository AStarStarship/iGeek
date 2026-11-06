// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekPrisonerWorldRelease
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace PrisonerWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* Release(const CHA* args) {
#if _Seam >= _iGeekPrisonerWorldRelease
  _ATestBegin;
  _DROut("\nPrisoner's Dilemma Simulation (all-cooperate)\n");

  PrisonerEnvironment env;
  env.SetPayoffs(5, 3, 1, 0);
  env.Reset();

  IUD actions[2] = {0, 0};
  FPC rewards[2];
  BOL terminals[2];
  ISC episodes = 100;
  for (ISC ep = 0; ep < episodes; ++ep) {
    if (env.Done()) {
      _DROut("Episode " << ep << " done (history limit reached)\n");
      env.Reset();
    }
    env.Step(actions, rewards, terminals);
    if (ep % 10 == 0) {
      _DROut("Episode " << ep << ", Round " << env.Round() << "\n");
    }
  }
  _DROut("\nDone. Total episodes: " << episodes << "\n");
#endif
  return _NilP;
}

}  //< namespace PrisonerWorldTest
