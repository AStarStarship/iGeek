// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>
#ifndef TYPECRAFT_PLAYER_DECL
#define TYPECRAFT_PLAYER_DECL
#include "Inventory.h"
namespace Typecraft {

class Entity {
 public:
  Entity(const CHA* name = "Steve");

  /* Gets the X coordinate. */
  inline ISC GetX();

  /* Sets the X Value. */
  inline void SetX(ISC value);

  /* Gets the Y coordinate. */
  inline ISC GetY();

  /* Sets the X Value. */
  inline void SetY(ISC value);

  /* Gets the Y coordinate. */
  inline ISC GetZ();

  /* Sets the X Value. */
  inline void SetZ(ISC value);

  /* Gets the max number of blocks an Entity can reach to manipulate. */
  ISC GetReach();

  /* Sets the max number of blocks an Entity can reach to manipulate.
  @return Returns zero upon success and the input value if it is < 0 . */
  ISC SetReach(ISC value);

  /* Gets the Entity's health. */
  ISC GetHealth();

  /* Sets the Entity's health.
  @return Returns zero upon success and the input value if it is < 0 . */
  ISC SetHealth(ISC value);

  /* Gets the Entity's max health. */
  ISC GetMaxHealth();

  /* Sets the max number of blocks an Entity can reach to manipulate.
  @return Returns zero upon success and the input value if it is < 0 . */
  ISC SetMaxHealth(ISC value);

  /* Gets the Entity's armor rating. */
  ISC GetArmor();

  /* Sets the Entity's armor rating.
  @return Returns zero upon success and the input value if it is < 0 . */
  ISC SetArmor(ISC value);

  /* Gets the Entity's height measured in half blocks. */
  ISC Height();

  /* Sets the Entity's height in half blocks.
  @return Returns zero upon success and the input value if it is < 0 . */
  ISC SetHeight(ISC value);

  /* Gets the Entity's hunger rating. */
  ISC GetHunger();

  /* Sets the Entity's hunger rating.
  @return Returns zero upon success and the input value if it is < 0 . */
  ISC SetHunger(ISC value);

  /* Print's the Entity to the console. */
  void Print();

  /* Gets a reverence to the Inventory. */
  Inventory& GetInventory();

  /* Prints the Entity to the console. */
  virtual void Print();

 private:
  ISC x_,                //< The Entity's x position.
      y_,                //< The Entity's y position.
      z_,                //< The Entity's z position.
      reach_,            //< How far the Entity can reach to Mine/Attack/etc.
      health_,           //< The Entity's health.
      max_health,        //< The Entity's max health.
      attack_,           //< The Entity's attack rating.
      armor_,            //< The Entity's armor rating.
      hunger_,           //< The Entity's hunger rating.
      height_;           //< The height of the Entity stored in half blocks.
  Inventory inventory_;  //< An Entity's Inventory.
};

}  //< namespace Typecraft
#endif
