// Copyright AStarship <https://astarship.net>.

#ifndef TYPINGWORLD_RECIPE_DECL
#define TYPINGWORLD_RECIPE_DECL

#include "Item.h"

namespace TypingWorld {

/* A Crafting Table recipe.
Recipes do not take up the entire crafting table. For this reason, the
width and height of the recipe are stored. */
class Recipe: public Item {
 public:
  /* Default constructs a empty CraftingTable. */
  Recipe();

  /* Prints the Recipe to the Console. */
  void Print();

 private:
  CHA *name_,                   //< The name of the Recipe.
      *description_;            //< A description of the Recipe.
  ISC width_,                   //< The width of the recipe in ingredients.
      height_;                  //< The height of the recipe in ingredients.
  ItemType ingredients_[3][3];  //< The ingredients.
};

}  //< namespace TypingWorld
#endif
