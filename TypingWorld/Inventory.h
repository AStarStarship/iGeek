// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>
#ifndef TYPECRAFT_INVENTORY_H
#define TYPECRAFT_INVENTORY_H
//
#include "ItemGroup.h"
namespace Typecraft {
class Inventory {
 public:
  /* Constructs an Inventory with the given max size. */
  Inventory(ISC max_size);

  /* Gets inventory count. */
  ISC GetCount();

  /* Adds an Item. */
  ISC AddItem(Item* item);

  /* Removes the Item with the given index.
  @param  The index of the Item to remove.
  @return Returns nullptr if the index is out of bounds and a pointer to the 
  Item it removed upon success. */
  Item* RemoveItem(ISC index);

  /* Deletes the entire Inventory. */
  void DeleteAll();

  /* Prints the Inventory to the console. */
  void Print(_::Log& log);

 private:
  ArX<ItemGroup*> items_;  //< An array of Inventory Items.
};

}  //< namespace Typecraft
#endif
