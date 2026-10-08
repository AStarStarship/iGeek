// Copyright AStarship <https://astarship.net>.

// The card world's test aggregator, mimicking ASCIICrabs/_Seams/_Tests.hxx.
// It pulls in the card game implementation headers (in dependency order),
// then the numbered seam units, then defines the top-level test tree.

#include <_Config.h>

// The canonical Crabs single-TU package: includes all the .hxx impls in the
// correct dependency order (Array first, which brings in TPtr/Linef/IsError).
#include <_Package.hxx>

// The card game, in dependency order.
#include <Card.h>
#include <Card.hxx>
#include <CardStack.h>
#include <CardStack.hxx>
#include <Deck.h>
#include <Deck.hxx>
#include <Hand.h>
#include <Hand.hxx>
#include <Player.h>
#include <Player.hxx>
#include <Dealer.h>
#include <Dealer.hxx>
#include <Blackjack.h>
#include <Blackjack.hxx>
#include <BlackjackEnv.h>
#include <BlackjackEnv.hxx>
// The Puffer-informed RL layer (iGeek tensor ops + the vectorized gym).
#include "../Tensor.h"
#include "../Tensor.hxx"
#include "../Policy.h"
#include "../PolicyNet.hxx"
#include "../Gym.hxx"
#include "../LinearPolicy.hxx"
#include "../PPO.hxx"
#include <BlackjackGym.h>
#include <BlackjackGym.hxx>

// The numbered seam units (00 = core unit tests, 01 = release/demo).
#include "00.Core.hxx"
#include "01.Release.hxx"
#include "03.Gym.hxx"
#include "04.Policy.hxx"
#include "05.PolicyBwd.hxx"
#include "06.PPO.hxx"

// The Crabs test harness (TestTree, TTestTree, TestEq, TestFail, ...).
#include "../../../ASCIICrabs/Test.hpp"
using namespace ::_;

// The top-level card-world test tree. Called by main() via
// TTestTree<CardsWorldTests>(arg_count, args) when SEAM != SEAM_N.
inline const CHA* CardsWorldTests(const CHA* args) {
  return TTestTree<CWTest::Core, CWTest::Gym, CWTest::Policy,
                   CWTest::PolicyBwd, CWTest::PPO, CWTest::Release>(args);
}
