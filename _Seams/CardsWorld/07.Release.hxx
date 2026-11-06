// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekCardsWorldRelease
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
namespace CardsWorldTest {
using namespace ::_;
using namespace iGeek; 

// A demo policy: hit until the hand value is >= 17, else stand.
ISC PolicyHitTo17Release(const CHA* percept) {
  ISC player_val = 0;
  const CHA* cursor = percept;
  while (*cursor != 0) {
    if (*cursor == 'P' && cursor[1] == ':') {
      ++cursor;
      while (*cursor == ':') ++cursor;
      while (*cursor >= '0' && *cursor <= '9') {
        player_val = player_val * 10 + (*cursor - '0');
        ++cursor;
      }
      continue;
    }
    ++cursor;
  }
  return player_val < 17 ? 0 : 1;  // 0=hit, 1=stand.
}

// The release/demo unit: runs a short blackjack demo. Returns _NilP on success.
inline const CHA* Release(const CHA* args) {
  using namespace iGeek::CardsWorld;
#if _Seam >= _iGeekCardsWorldRelease
  _ATestBegin;
  _DROut("\n=====================================================\n");
  _DROut(" iGeek Cards World — Blackjack (BlackjackEnv)\n");
  _DROut(" Latest ASCIICrabs API — console only\n");
  _DROut("=====================================================\n");

  // Create the environment (this deals the first round).
  BlackjackEnv env;

  // Print the initial observation.
  _DROut("\nInitial table state:\n  " << env.Observe() << "\n");

  // Run 10 rounds with the basic hit-to-17 policy.
  _DROut("\nRunning 10 rounds with the basic policy...\n");
  ISC total_reward = env.RunPolicy(&PolicyHitTo17Release, 10);

  _DROut("\n-----------------------------------------------------\n");
  _DROut(" Total reward over 10 rounds: " << total_reward << "\n");
  _DROut(" Agent credit:                " << env.Credit() << "\n");
  _DROut("=====================================================\n");
#endif
  return _NilP;
}

}  //< namespace CardsWorldTest
