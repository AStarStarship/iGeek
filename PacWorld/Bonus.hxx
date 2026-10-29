// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_BONUS_HXX
#define IGEEK_BONUS_HXX
#include "Bonus.h"

using namespace PacWorld;

Bonus::Bonus() : fruit_(Banana), texture_rect_(32, 0, 30, 30) {
  SetFruit(Banana);
}

void Bonus::SetFruit(Bonus::Fruit fruit) {
  fruit_ = static_cast<ISC>(fruit);
  if (fruit == Bonus::Banana) {
    texture_rect_ = ISC4(32, 0, 30, 30);
  } else if (fruit == Bonus::Apple) {
    texture_rect_ = ISC4(23 + 30, 0, 30, 30);
  } else if (fruit == Bonus::Cherry) {
    texture_rect_ = ISC4(32 + 60, 0, 30, 30);
  }
}

#endif  // IGEEK_BONUS_HXX
