// Copyright AStarship <https://astarship.net>.
#include "../../Worlds/Cards/Card.hxx"
#include "../../Worlds/Cards/CardStack.hxx"
#include "../../Worlds/Cards/Deck.hxx"
#include "../../Worlds/Cards/Hand.hxx"
#include "../../Worlds/Cards/Player.hxx"
#include "../../Worlds/Cards/Dealer.hxx"
#include "../../Worlds/Cards/Blackjack.hxx"
#include "../../Worlds/Cards/BlackjackGym.hxx"
#include "../../Worlds/Cards/BlackjackEnv.hxx"
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
