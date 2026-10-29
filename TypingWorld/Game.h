// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>
#ifndef TYPINGWORLD_GAME_DECL
#define TYPINGWORLD_GAME_DECL

#include "Entity.h"
#include "World.h"

namespace TypingWorld {

class Game {
 public:
  /* Creates a new games with the default settings. */
  Game();

 private:
  ISC x_,  //< The screen x coordinate.
      y_,  //< The screen y coordinate.
      z_;  //< The screen z coordinate.

  Entity* player_;  //< The player under control.
  World* world_;    //< The current world.
};

}  //< namespace TypingWorld
#endif
