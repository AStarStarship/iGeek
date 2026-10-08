// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>
#ifndef TYPECRAFT_ITEMGROUP_DECL
#define TYPECRAFT_ITEMGROUP_DECL
//
#include "Item.h"
namespace Typecraft {
class ItemGroup {
 public:
  /* Constructs an ItemGroup with the given max_size. */
  ItemGroup(ISC max_size);

  /* Destructor */
  ~ItemGroup();

  /* Gets the inventory count. */
  ISC GetCount();

  /* Gets the max size of the Inventory. */
  ISC GetSize();

  /* Gets the Item at the given index. */
  Item* GetItem(ISC index);

  /* Adds the Item at the given index.
  @param  item Th Item to add.
  @return Returns -1 if the Inventory is full. */
  ISC AddItem(Item* item);

  /* Removes the Item at the given index.
  @param  The index of the Item to remove.
  @return Returns nullptr if the Index is out of bounds. */
  Item* RemoveItem(ISC index);

  /* Prints the ItemGroup to the console. */
  void Print();

 private:
  _::ArX<Item*> items;  //< An array of Inventory Items.
};

}  //< namespace Typecraft
#endif  //< TYPECRAFT_ITEMGROUP_H
