// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekCardsWorldCore
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#include "../../../ASCIICrabs/ROut.h"
#endif
namespace CardsWorldTest {
using namespace ::_;
using namespace iGeek; 

// Fail counter so the unit can make the process exit non-zero on a bad
// assertion. The ASCIICrabs A_ASSERT/A_AVOW macros print-and-continue (they do
// not set the exit code), so we wrap them: on a failed condition we bump the
// counter, and Core() returns an error string at the end if it is non-zero.
inline ISC& CWFails() {
  static ISC count = 0;
  return count;
}

#define CORE_CHECK(cond)                                       \
  do {                                                         \
    if (!::_::Test(cond)) {                                    \
      ::_::RefOut() << "\nFAILURE Core at line:" << __LINE__   \
                    << " in \"" << __FILE__ << "\"\n";         \
      ++CWFails();                                             \
    }                                                          \
  } while (0)

#define CORE_EQ(a, b)                                          \
  do {                                                         \
    if (!::_::TestEq(a, b)) {                                  \
      ::_::RefOut() << "\nFAILURE Core (expecting " << (a)     \
                    << " found " << (b) << ") at line:"        \
                    << __LINE__ << " in \"" << __FILE__ << "\"\n"; \
      ++CWFails();                                             \
    }                                                          \
  } while (0)

#define CORE_PTR(p)                                            \
  do {                                                         \
    if (IsError(p)) {                                          \
      ::_::RefOut() << "\nFAILURE Core (nil ptr) at line:"     \
                    << __LINE__ << " in \"" << __FILE__ << "\"\n"; \
      ++CWFails();                                             \
    }                                                          \
  } while (0)

// Build a Card from a pip (1-13) with correct face/point values.
CardsWorld::Card MakeCard(ISC pip, ISC suit = 1) {
  ISC face_value = pip;
  ISC point_value = (pip >= 11) ? 10 : pip;  // face cards are 10.
  return CardsWorld::Card(pip, suit, face_value, point_value,
                          CardsWorld::SuitCulture::SCFrench);
}

// A small helper: value of a hand built from the given pips (suits default to
// club, culture French). pips 1-13; 1=ace, 11=jack, 12=queen, 13=king.
ISC HandValueFromPips(const ISC* pips, ISC count) {
  CardsWorld::Card cards[16];
  for (ISC i = 0; i < count; ++i) {
    cards[i] = MakeCard(pips[i]);
  }
  CardsWorld::Hand hand(1, 54);
  for (ISC i = 0; i < count; ++i) hand.AddCard(&cards[i]);
  CardsWorld::Player player("Valuer", 0);
  CardsWorld::Blackjack bj(&player);
  return bj.HandValue(hand);
}

// The unit tests for the card world core. Returns _NilP on success, or an
// error string naming the first failed assertion.
inline const CHA* Core(const CHA* args) {
  _ATestBegin;

  // Card: point values
  {
    _DROut("Testing Card point values\n");
    CardsWorld::Card ace(1, 1, 0, 0, CardsWorld::SuitCulture::SCFrench);
    CardsWorld::Card ten(10, 2, 0, 0, CardsWorld::SuitCulture::SCFrench);
    CardsWorld::Card king(13, 4, 0, 0, CardsWorld::SuitCulture::SCFrench);
    CORE_CHECK(ace.PipValue() == 1);
    CORE_CHECK(ace.SuitValue() == 1);
    CORE_CHECK(ten.PipValue() == 10);
    CORE_CHECK(king.PipValue() == 13);
    CORE_CHECK(king.SuitValue() == 4);
  }

  // CardStack: add / peek / take / count
  {
    _DROut("Testing CardStack add/peek/take/count\n");
    CardsWorld::Deck deck;
    CardsWorld::CardStack stock = deck.Stock();
    CORE_EQ(stock.CardCount(), 52);  // a fresh 52-card deck.

    // Peek does not remove.
    CardsWorld::Card* top = stock.PeekCard(0);
    CORE_PTR(top);
    CORE_EQ(stock.CardCount(), 52);

    // Take removes from the top.
    CardsWorld::Card* taken = stock.TakeNextCard();
    CORE_PTR(taken);
    CORE_EQ(stock.CardCount(), 51);

    // Draw 51 more, then the stack is empty and TakeNextCard returns nil.
    for (ISC i = 0; i < 51; ++i) stock.TakeNextCard();
    CORE_EQ(stock.CardCount(), 0);
    CORE_CHECK(stock.IsEmpty());
    CORE_CHECK(stock.TakeNextCard() == _NilP);  // empty -> nil, no crash.
  }

  // CardStack: shuffle integrity (regression for count_ corruption)
  {
    _DROut("Testing CardStack shuffle integrity\n");
    CardsWorld::Deck deck;
    for (ISC iter = 0; iter < 50; ++iter) {
      CardsWorld::CardStack stock = deck.Stock();
      stock.Shuffle();
      CORE_EQ(stock.CardCount(), 52);  // shuffle must not corrupt the count.
    }
  }

  // Deck: 52 unique cards
  {
    _DROut("Testing Deck builds 52 unique cards\n");
    CardsWorld::Deck deck;
    CardsWorld::CardStack stock = deck.Stock();
    // Count distinct (suit, pip) pairs.
    bool seen[5][14] = {};
    ISC distinct = 0;
    for (ISC i = 0; i < stock.CardCount(); ++i) {
      CardsWorld::Card* c = stock.PeekCard(i);
      CORE_PTR(c);
      ISC suit = c->SuitValue();
      ISC pip = c->PipValue();
      if (suit >= 1 && suit <= 4 && pip >= 1 && pip <= 13 && !seen[suit][pip]) {
        seen[suit][pip] = true;
        ++distinct;
      }
    }
    CORE_EQ(distinct, 52);  // exactly 52 unique (suit, pip) combos.
  }

  // HandValue: ace demotion
  {
    _DROut("Testing HandValue ace demotion\n");
    // A + 10 = 21 (ace counts as 11).
    { ISC pips[2] = {1, 10}; CORE_EQ(HandValueFromPips(pips, 2), 21); }
    // A + A + 9 = 21 (two aces 11 + 9 = 31, demote one ace -> 21).
    { ISC pips[3] = {1, 1, 9}; CORE_EQ(HandValueFromPips(pips, 3), 21); }
    // A + A + A + 9 = 12 (two aces demoted).
    { ISC pips[4] = {1, 1, 1, 9}; CORE_EQ(HandValueFromPips(pips, 4), 12); }
    // 10 + 10 + 10 = 30 (bust, no aces to demote).
    { ISC pips[3] = {10, 10, 10}; CORE_EQ(HandValueFromPips(pips, 3), 30); }
    // K + 5 = 15 (face cards are 10).
    { ISC pips[2] = {13, 5}; CORE_EQ(HandValueFromPips(pips, 2), 15); }
  }

  // IsBlackjack: natural 21
  {
    _DROut("Testing IsBlackjack\n");
    {
      ISC pips[2] = {1, 10};
      CardsWorld::Card cards[2] = {MakeCard(pips[0], 1), MakeCard(pips[1], 2)};
      CardsWorld::Hand hand(1, 54);
      hand.AddCard(&cards[0]);
      hand.AddCard(&cards[1]);
      CardsWorld::Player player("BJ", 0);
      CardsWorld::Blackjack bj(&player);
      CORE_CHECK(bj.IsBlackjack(hand));  // A+10 in two cards is a natural.
    }
    {
      // 10 + 10 + A = 21 but in three cards -> NOT a natural.
      ISC pips[3] = {10, 10, 1};
      CardsWorld::Card cards[3] = {MakeCard(pips[0], 1), MakeCard(pips[1], 2),
                                   MakeCard(pips[2], 3)};
      CardsWorld::Hand hand(1, 54);
      for (ISC i = 0; i < 3; ++i) hand.AddCard(&cards[i]);
      CardsWorld::Player player("BJ2", 0);
      CardsWorld::Blackjack bj(&player);
      CORE_CHECK(!bj.IsBlackjack(hand));
    }
  }

  // Round outcome: player bust
  {
    _DROut("Testing round outcome (player bust)\n");
    CardsWorld::Player player("Buster", 100);
    CardsWorld::Blackjack bj(&player);
    bj.NewRound();
    // Hit until the player busts or stands; a hand over 21 must settle as bust.
    // (We can't force specific cards, so verify the invariant: if the player's
    // hand exceeds 21, the outcome is a player bust.)
    ISC guard = 0;
    while (!bj.RoundOver() && guard < 20) {
      bj.Hit();
      ++guard;
    }
    if (bj.HandValue(bj.PlayerHand()) > 21) {
      CORE_EQ(ISC(bj.GetOutcome()), ISC(CardsWorld::Outcome::kPlayerBust));
    }
    CORE_CHECK(bj.RoundOver());  // the round must have settled.
  }

  if (CWFails() != 0) {
    _DROut("\n" << CWFails() << " Core assertion(s) FAILED\n");
    return "cards_world_core_test_failure";
  }
  return _NilP;
}

}  //< namespace CardsWorldTest
