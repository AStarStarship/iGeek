// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>
#ifndef TYPECRAFT_CHUNK_DECL
#define TYPECRAFT_CHUNK_DECL
#include "Block.h"
#include "Entity.h"
namespace Typecraft {

class Chunk {
 public:
  enum {
    cSize = 16,     //< The width and height of a Chunk in Block(s).
    cHeight = 512,  //< The height of a Chunk in Block(s).
  };

  /* A chunk of Clocks. */
  Chunk();

  /* Gets the X coordinate. */
  inline ISC GetX();

  /* Gets the Y coordinate. */
  inline ISC GetY();

  /* Gets the Y coordinate. */
  inline ISC GetZ();

  /* Gets inventory count. */
  inline Block* GetBlock(ISC x, ISC y, ISC z);

  /* Adds set's the block at the given index. */
  inline BOL SetBlock(Block* block, ISC const x, ISC y, ISC z);

 private:
  ISC x_,                    //< The farthest x position to the left.
      y_,                    //< The farthest y position to the fight.
      z_;                    //< The lowest z value.
  AArray<Item*> items_;      //< The Item(s) in the Chunk.
  AArray<Entity*> entities;  //< The Entity(s) in the Chunk.
  Block* blocks_[cSize][cSize][cHeight];
  //< The Block(s) in the Chunk.
};

}  //< namespace Typecraft
#endif
