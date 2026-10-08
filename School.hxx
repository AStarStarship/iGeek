// Copyright AStarship <https://astarship.net>.
#include "School.h"
#if SEAM >= IGEEK_CORE
namespace _ {

School::School() {}

ISC School::ClassCount() { return class_count_; }

void School::CreateRepo() {
  // InitializeRepo
}

const Op* School::Star(CHC index, Crabs* crabs) {
  static const Op cThis = {"Door",
                           OpFirst('A'),
                           OpFirst('A' + 0),
                           "A course in a program.",
                           '}',
                           ';',
                           ' ',
                           false,
                           nullptr,
                           nullptr};
  if (index == '?') {
    return CrabsQuery(crabs, cThis);
  }
  index -= ' ';
  if (((ISC)index) >= slots_->count) {
    return DoorResult(this, Door::c_ErrorInvalidOp);
  }
  return nullptr;
}
}  // namespace _
#endif
