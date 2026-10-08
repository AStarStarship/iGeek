// Copyright AStarship <https://astarship.net>.

#include "Player.h"
#if SEAM >= IGEEK_PLAYER
namespace _ {
namespace CardsWorld {

/* Copies up to (dest_size-1) chars from src to dest, nil-terminating. */
static void NameCopy(CHA* dest, ISC dest_size, const CHA* src) {
  if (dest == NILP || dest_size <= 0) return;
  dest[0] = 0;
  if (src == NILP) return;
  ISC i = 0;
  while (src[i] != 0 && i < dest_size - 1) {
    dest[i] = src[i];
    ++i;
  }
  dest[i] = 0;
}

Player::Player(const CHA* name, ISC points)
    : win_count_(0),
      point_count_(points < 1 ? 1 : points),
      hand_() {
  NameCopy(name_, NameLengthMax, name);
}

void Player::SetName(const CHA* name) {
  NameCopy(name_, NameLengthMax, name);
}

ISC Player::PointsAdd(ISC num_points) {
  point_count_ += num_points;
  return point_count_;
}

}  //< namespace CardsWorld
}  //< namespace _
#endif
