// Copyright AStarship <https://astarship.net>.
#pragma once
#include <_Config.h>

#ifndef TILEWORLD_INVENTORY
#define TILEWORLD_INVENTORY

#include "Item.h"

class Inventory : public Operand {
  Item** items_;
  unsigned item_count_;
  unsigned item_count_max_;

  void Initialize();
  void Expand();
  void Nullify(const unsigned from = 0);
  void FreeMemory();

 public:
  Inventory();
  virtual ~Inventory();
};
#endif
