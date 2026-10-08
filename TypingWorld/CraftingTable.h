// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>
#ifndef TYPECRAFT_CRAFTINGTABLE_DECL
#define TYPECRAFT_CRAFTINGTABLE_DECL
#include "Item.h"
namespace Typecraft {
/* Crafting Table. */
class CraftingTable : public Item {
 public:
  /* Default constructs a empty CraftingTable. */
  CraftingTable();

  /* Sets the Item at the given x and y coordinate. */
  Item* GetItem(ISC x, ISC y);

  /* Sets the Item at the given x and y coordinate. */
  Item* SetItem(ISC x, ISC y, Item* block);

  /* Prints the CraftingTable to the console. */
  void Print();

 private:
  Item* blocks_[3][3];
};

}  //< namespace Typecraft
#endif
