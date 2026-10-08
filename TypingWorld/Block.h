// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>
#ifndef TYPECRAFT_BLOCK_DECL
#define TYPECRAFT_BLOCK_DECL
#include "Item.h"
namespace Typecraft {

/* A MineCraft block that may contain items when it breaks.
When you break a Block in MineCraft, it may give you zero or more Item(s)
all of the same type. */
class Block : public Item {
 public:
  /* Constructor creates a Block with the given count of the same Item. */
  Block(ISC type, ISC variant, ISC count = 0);

  /* Gets the type of Block. */
  ISC GetNumItems();

  /* Gets the type. */
  Item::Type Getype();

  /* Mines a block.
  @return Returns 0 if not mined and */
  ISC Mine(Item* tool);

 private:
  ISC strenth_,          //< Block strength.
      regeneration_;     //< How fast the block regenerates.
  Array<Item*>* items_;  //< Items contained in this block.
};

}  //< namespace Typecraft
#endif
