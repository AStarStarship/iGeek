// Copyright AStarship <https://astarship.net>.
#include "EnemySystem.h"

TileWorld::EnemySystem::EnemySystem(_::ISC capacity)
    : capacity_(capacity < 1 ? 1 : capacity), count_(0) {
  enemies_ = new TileWorld::Enemy*[capacity_](NILP);
}

TileWorld::EnemySystem::~EnemySystem() {
  for (_::ISC i = 0; i < capacity_; i++) {
    if (enemies_[i] != NILP) delete enemies_[i];
  }
  delete[] enemies_;
}

// PORT-NOTE: the original switched on EnemyTypes::RAT and pushed a new Rat into
// a shared array; the headless version allocates into its own fixed array.
// The "unknown type" branch printed an error via std::cout; here it returns -1.
_::ISC TileWorld::EnemySystem::CreateEnemy(const _::ISM type, const _::FPC xPos,
                                           const _::FPC yPos) {
  switch (type) {
    case EnemyTypes::RAT:
      if (count_ >= capacity_) return -1;
      enemies_[count_] = new TileWorld::Enemy(xPos, yPos);
      return count_++;
    default:
      return -1;
  }
}

// PORT-NOTE: the original EnemySystem::Update was empty (each enemy was updated
// by the game state); kept as-is.
void TileWorld::EnemySystem::Update(const _::FPC& dt) {
  (void)dt;
}
