/* Interactive Gym Environment and Educaitonal Kit @version 0.x
@link    https://github.com/KabukiStarship/iGeek.git
@file    /School.inl
@author  Cale McCollough <https://cookingwithcale.org>
@license Copyright (C) 2021 Kabuki Starship (TM) <kabukistarship.com>;
This Source Code Form is subject to the terms of the Mozilla Public License,
v. 2.0. If a copy of the MPL was not distributed with this file, You can obtain
one at <https://mozilla.org/MPL/2.0/>. */
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
