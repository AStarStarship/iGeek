// Copyright AStarship <https://astarship.net>.

#include "Hand.h"
#if SEAM >= IGEEK_HAND
namespace _ {
namespace CardsWorld {

Hand::Hand()
    : CardStack(0, 54, true) {}

Hand::Hand(ISC min_cards, ISC max_cards)
    : CardStack(min_cards, max_cards, true) {}

ISC Hand::DealTwoCards(CardStack& stock) {
  if (stock.CardCount() < 2) return -1;
  for (ISC i = 0; i < 2; ++i) AddCard(stock.TakeNextCard());
  return 0;
}

const CHA* Hand::Describe(CHA* buffer, ISC buffer_size) const {
  if (buffer == NILP || buffer_size <= 0) return "";
  buffer[0] = 0;
  // Build a short description like "A c 2 d" (ASCII suit chars: c/d/h/s).
  CHA* cursor = buffer;
  CHA* stop = buffer + buffer_size - 1;
  for (ISC i = 0; i < CardCount() && cursor < stop; ++i) {
    const Card* c = PeekCard(i);
    if (c == NILP) break;
    CHA pip_char = '0';
    switch (c->PipValue()) {
      case 1: pip_char = 'A'; break;
      case 11: pip_char = 'J'; break;
      case 12: pip_char = 'Q'; break;
      case 13: pip_char = 'K'; break;
      default: pip_char = static_cast<CHA>('0' + c->PipValue()); break;
    }
    CHA suit_char = '?';
    switch (c->SuitValue()) {
      case 1: suit_char = 'c'; break;
      case 2: suit_char = 'd'; break;
      case 3: suit_char = 'h'; break;
      case 4: suit_char = 's'; break;
    }
    if (cursor < stop - 1) {
      *cursor++ = pip_char;
      *cursor++ = suit_char;
    }
    if (cursor < stop && i < CardCount() - 1) *cursor++ = ' ';
  }
  *cursor = 0;
  return buffer;
}

}  //< namespace CardsWorld
}  //< namespace _
#endif
