// Copyright AStarship <https://astarship.net>.

#include "CardStack.h"
#if SEAM >= IGEEK_CARDSTACK
namespace _ {
namespace CardsWorld {

CardStack::CardStack()
    : card_count_min_(0),
      card_count_max_(54),
      is_visible_(true),
      count_(0) {
  for (ISC i = 0; i < CardStackMax; ++i) cards_[i] = NILP;
}

CardStack::CardStack(ISC min_cards, ISC max_cards, BOL is_visible)
    : card_count_min_(min_cards < 0 ? 0 : min_cards),
      card_count_max_(max_cards < 1 ? 1 : max_cards),
      is_visible_(is_visible),
      count_(0) {
  for (ISC i = 0; i < CardStackMax; ++i) cards_[i] = NILP;
}

CardStack::CardStack(const CardStack& other)
    : card_count_min_(other.card_count_min_),
      card_count_max_(other.card_count_max_),
      is_visible_(other.is_visible_),
      count_(other.count_) {
  for (ISC i = 0; i < CardStackMax; ++i)
    cards_[i] = (i < other.count_) ? other.cards_[i] : NILP;
}

ISC CardStack::Compare(const CardStack& other) const {
  ISC point_value = PointValue(), other_point_value = other.PointValue();
  if (point_value > other_point_value) return 1;
  if (point_value < other_point_value) return -1;
  return 0;
}

ISC CardStack::PointValue() const {
  ISC total = 0;
  for (ISC i = 0; i < count_; ++i) total += cards_[i]->PointValue();
  return total;
}

void CardStack::Shuffle() {
  // Fisher-Yates: walk from the top down, swap with a random card below.
  // NOTE: _::Random(min,max) in the current ASCIICrabs core (high-SEAM
  // TRandom) is not strictly uniform and can return values > max, so clamp
  // j to [0, i] to keep the swap in-bounds. (Upstream TRandom bug: it draws
  // from a half-normal instead of a uniform distribution.)
  for (ISC i = count_ - 1; i > 0; --i) {
    ISC j = Random(0, i);
    if (j < 0) j = 0;
    if (j > i) j = i;  // Defensive clamp: Random() may exceed max.
    Card* tmp = cards_[i];
    cards_[i] = cards_[j];
    cards_[j] = tmp;
  }
}

ISC CardStack::AddCard(Card* new_card) {
  if (new_card == NILP) return -1;
  if (count_ + 1 > card_count_max_) return 2;
  cards_[count_++] = new_card;
  return 0;
}

ISC CardStack::InsertCard(Card* new_card, ISC index) {
  if (new_card == NILP) return -1;
  if (index < 0 || index > count_) return 1;
  if (count_ + 1 > card_count_max_) return 2;
  for (ISC i = count_; i > index; --i) cards_[i] = cards_[i - 1];
  cards_[index] = new_card;
  ++count_;
  return 0;
}

ISC CardStack::AddCardStack(const CardStack& cards) {
  if (count_ + cards.count_ > card_count_max_) return 1;
  for (ISC i = 0; i < cards.count_; ++i) AddCard(cards.cards_[i]);
  return 0;
}

ISC CardStack::DrawCards(CardStack& source, ISC cards_to_take) {
  if (cards_to_take < 0) return -1;
  if (cards_to_take > source.CardCount()) return 1;
  if (count_ + cards_to_take > card_count_max_) return 2;
  for (ISC i = 0; i < cards_to_take; ++i) AddCard(source.TakeNextCard());
  return 0;
}

BOL CardStack::RemoveCard(Card* card) {
  for (ISC i = 0; i < count_; ++i) {
    if (cards_[i] == card) {
      for (ISC j = i; j < count_ - 1; ++j) cards_[j] = cards_[j + 1];
      --count_;
      return true;
    }
  }
  return false;
}

Card* CardStack::PeekCard(ISC index) const {
  if (index < 0 || index >= count_) return NILP;
  return cards_[index];
}

Card* CardStack::TakeCard(ISC index) {
  if (index < 0 || index >= count_) return NILP;
  Card* result = cards_[index];
  for (ISC j = index; j < count_ - 1; ++j) cards_[j] = cards_[j + 1];
  --count_;
  return result;
}

Card* CardStack::TakeNextCard() {
  if (count_ == 0) return NILP;
  Card* next_card = cards_[count_ - 1];
  --count_;
  return next_card;
}

Card* CardStack::TakeRandomCard() {
  if (count_ == 0) return NILP;
  ISC random_index = Random(0, count_ - 1);
  if (random_index < 0) random_index = 0;
  if (random_index >= count_) random_index = count_ - 1;  // Clamp (Random may exceed max).
  return TakeCard(random_index);
}

}  //< namespace CardsWorld
}  //< namespace _
#endif
