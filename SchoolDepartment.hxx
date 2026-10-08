// Copyright AStarship <https://astarship.net>.
#include "SchoolDepartment.h"
#if SEAM >= IGEEK_CORE
namespace _ {

SchoolDepartment::SchoolDepartment() {

}

ISC SchoolDepartment::ClassCount() { return class_count_; }

void SchoolDepartment::CreateRepo() {
  // InitializeRepo
  for (int i = 0; i < class_count_; ++i) {

  }
}

const Op* SchoolDepartment::Star(CHC index, Crabs* crabs) {
  static const Op cThis = { "Door",
    OpFirst('A'),
    OpFirst('A' + 0),
    "A course in a program.",
    '}',
    ';',
    ' ',
    false,
    nullptr,
    nullptr
  };
  if (index == '?') {
    return CrabsQuery(crabs, cThis);
  }
  index -= ' ';
  if (((ISC)index) >= slots_->count) {
    return DoorResult(this, Door::ErrorInvalidOp);
  }
  return nullptr;
}

}  // namespace _
#endif
