// Copyright AStarship <https://astarship.net>.
#include "SchoolCourse.h
#if SEAM >= IGEEK_CORE
namespace _ {

SchoolCourse::Course() {

}

ISC SchoolCourse::ClassCount() { return class_count_; }

void SchoolCourse::CreateRepo() {
  //InitializeRepo
  for (int i = 0; i < class_count_; ++i) {
  }
}

const Op* SchoolCourse::Star(CHC index, Crabs* crabs) {
  static const Op cThis = {
    "Door",
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
#endif
