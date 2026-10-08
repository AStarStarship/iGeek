// Copyright AStarship <https://astarship.net>.
#include "Chunk.h"
namespace Typecraft {

Chunk::Chunk() {}

ISC Chunk::GetX() { return x_; }

ISC Chunk::GetY() { return y_; }

ISC Chunk::GetZ() { return z_; }

Block* Chunk::GetBlock(ISC x, ISC y, ISC z) {
  if (x < 0) return nullptr;
  if (y < 0) return nullptr;
  if (z < 0) return nullptr;
  if (x >= kSize) return nullptr;
  if (y >= kSize) return nullptr;
  if (z >= kHeight) return nullptr;
  return blocks_[x][y][z];
}

void Chunk::SetBlock(Block* block, ISC const x, ISC y, ISC z) {
  if (x < 0) return;
  if (y < 0) return;
  if (z < 0) return;
  if (x >= kSize) return;
  if (y >= kSize) return;
  if (z >= kHeight) return;
  blocks_[x][y][z]->Change(block);
}

}  // namespace Typecraft
