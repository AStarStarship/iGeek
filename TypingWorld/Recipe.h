// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>
#ifndef TYPECRAFT_RECIPE_H
#define TYPECRAFT_RECIPE_H
#include "Item.h"
namespace Typecraft {

/* A Crafting Table recipe.
Recipes do not take up the entire crafting table. For this reason, the width and 
height of the recipe are stored. */
class Recipe : public Item {
 public:
  /* Default constructs a empty Recipe. */
  Recipe();

  /* Prints the Recipe to the Console. */
  void Print();

 private:
  CHA* name_,                   //< The name of the Recipe.
     * description_;            //< A description of the Recipe.
  ISC width_,                   //< The width of the recipe in ingredients.
      height_;                  //< The height of the recipe in ingredients.
  ItemType ingredients_[3][3];  //< The ingredients.
};

}  //< namespace Typecraft
#endif
