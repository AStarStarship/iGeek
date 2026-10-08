// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>
#ifndef TYPECRAFT_SPAWN_DECL
#define TYPECRAFT_SPAWN_DECL
#include "Item.h"
namespace Typecraft {

class Spawn : public Item {
 public:
  /* Constructs a Spawn of the given key. */
  Spawn(const CHA* name);

  /* Gets the type. */
  ISC GetType();

  /* Sets the type. */
  const CHA* Setype(ISC type);

  /* Mines a block. */
  void Mine();

 private:
  ISC type_;  //< What type of Spawn it is.
};

}  //< namespace Typecraft
#endif
