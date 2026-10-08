// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>
#ifndef TYPECRAFT_ENCHANTMENT_DECL
#define TYPECRAFT_ENCHANTMENT_DECL
#include "ItemGroup.h"
namespace Typecraft {

/* # Summary of enchantments
| Name                  | Summary                             | JEID | PEID | ML  | W  |
|:---------------------:|:-----------------------------------:|:----:|:----:|:---:|:--:|
| Aqua Affinity         | Increases underwater mining rate    | 6    | 8    | I   | 2  |
| Bane of Arthropods    | Increases damage to arthropods      | 18   | 11   | V   | 5  |
| Blast Protection      | Reduces explosion damage            | 3    | 3    | IV  | 2  |
| Curse of Binding **   | Prevents removal of item            | 10   | —    | I   | 1  |
| Curse of Vanishing ** | Item destroyed on death             | 71   | —    | I   | 1  |
| Depth Strider         | Increases underwater movement speed | 8    | 7    | III | 2  |
| Efficiency            | Increases mining speed              | 32   | 15   | V   | 10 |
| Feather Falling       | Reduces fall damage                 | 2    | 2    | IV  | 5  |
| Fire Aspect           | Sets target on fire                 | 20   | 13   | II  | 2  |
| Fire Protection       | Reduces fire damage                 | 1    | 1    | IV  | 5  |
| Flame                 | Arrows set target on fire           | 50   | 21   | I   | 2  |
| Fortune               | Increases block drops               | 35   | 18   | III | 2  |
| Frost Walker          | Walk on water                       | 9    | 25   | II  | 2  |
| Infinity              | Shooting consumes no arrows         | 51   | 22   | I   | 1  |
| Knockback             | Increases knockback                 | 19   | 12   | II  | 5  |
| Looting               | Increases mob loot                  | 21   | 14   | III | 2  |
| Luck of the Sea       | Increases fishing luck              | 61   | 23   | III | 2  |
| Lure                  | Increases fishing rate              | 62   | 24   | III | 2  |
| Mending               | Repair with experience              | 70   | 26   | I   | 2  |
| Power                 | Increases arrow damage              | 48   | 19   | V   | 10 |
| Projectile Protection | Reduces projectile damage           | 4    | 4    | IV  | 5  |
| Protection            | Reduces most damage                 | 0    | 0    | IV  | 10 |
| Punch                 | Increases arrow knockback           | 49   | 20   | II  | 2  |
| Respiration           | Extends underwater breathing time   | 5    | 6    | III | 2  |
| Sharpness             | Increases damage                    | 16   | 9    | V   | 10 |
| Silk Touch            | Mined blocks drop themselves        | 33   | 16   | I   | 1  |
| Smite                 | Increases damage to undead          | 17   | 10   | V   | 5  |
| Sweeping Edge *       | Increases sweeping attack damage    | 22   | —    | III | 2  |
| Thorns                | Damages attackers                   | 7    | 5    | III | 1  |
| Unbreaking            | Increases effective durability      | 34   | 17   | III | 5  |

* Key: JEID = Java Edition ID. PEID = Pocket Edition ID; ML = Max Level; W = Weight; 
* = Java Edition Only; ** = Java and Legacy Console editions only] */

struct Enchantment {
   ISN JEID,     //< Java Edition ID.
       PEID,     //< Pocket Edition ID.
       MaxLevel, //< The max
       Weight;   //< The weight of the Enchantment.
  
  /* Constructs an Enchantment. */
   Enchantment ();

  /* Virtual destructor for deallocating dynamic memory in subclasses. */
  virtual ~Enchantment ();

  /* Sets the enchantment weight. */
  ISC WeightSet(ISN weight);

  /* Removes the Item with the given index.
  @param  The index of the Item to remove.
  @return Returns nullptr if the index is out of bounds and a pointer to
  the Item it removed upon success. */
  Item* RemoveItem(ISC index);

  /* Deletes the entire Inventory. */
  void DeleteAll();

  /* Prints the Inventory to the console. */
  void Print();

  private:
  _::AArray<ItemGroup*> items_;  //< An array of Inventory Items.
};

}  //< namespace Typecraft
#endif
