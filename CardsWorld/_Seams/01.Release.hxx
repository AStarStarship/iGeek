// Copyright AStarship <https://astarship.net>.

// The card world's RELEASE seam unit: the runnable demo. Mimics
// ASCIICrabs/_Seams/23.Release.hxx. Gated on CARDSWORLD_RELEASE.

#if SEAM >= CARDSWORLD_RELEASE
#if SEAM == CARDSWORLD_RELEASE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif

using namespace ::_;
namespace CWTest {

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

// The release/demo unit: runs a short blackjack demo. Returns NILP on success.
inline const CHA* Release(const CHA* args) {
  (void)args;
#if SEAM >= CARDSWORLD_RELEASE
  A_TEST_BEGIN;
  D_COUT("\n=====================================================\n");
  D_COUT(" iGeek Cards World — Blackjack (BlackjackEnv)\n");
  D_COUT(" Latest ASCIICrabs API — console only\n");
  D_COUT("=====================================================\n");

  // Create the environment (this deals the first round).
  BlackjackEnv env;

  // Print the initial observation.
  D_COUT("\nInitial table state:\n  " << env.Observe() << "\n");

  // Run 10 rounds with the basic hit-to-17 policy.
  D_COUT("\nRunning 10 rounds with the basic policy...\n");
  ISC total_reward = env.RunPolicy(&PolicyHitTo17Release, 10);

  D_COUT("\n-----------------------------------------------------\n");
  D_COUT(" Total reward over 10 rounds: " << total_reward << "\n");
  D_COUT(" Agent credit:                " << env.Credit() << "\n");
  D_COUT("=====================================================\n");
#endif
  return NILP;
}

}  //< namespace CWTest
