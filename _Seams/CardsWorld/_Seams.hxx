// Copyright AStarship <https://astarship.net>.
#include "../../CardsWorld/Card.hxx"
#include "../../CardsWorld/CardStack.hxx"
#include "../../CardsWorld/Deck.hxx"
#include "../../CardsWorld/Hand.hxx"
#include "../../CardsWorld/Player.hxx"
#include "../../CardsWorld/Dealer.hxx"
#include "../../CardsWorld/Blackjack.hxx"
#include "../../CardsWorld/BlackjackGym.hxx"
#include "../../CardsWorld/BlackjackEnv.hxx"
#include "01.Core.hxx"
#include "07.Release.hxx"
namespace CardsWorldTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* CardsWorld(const CHA* args) {
  #if _Seam == _iGeekCardsWorldRelease
    return Release(args);
  #else
    return TTestTree<Core>(args);
  #endif
}
}
