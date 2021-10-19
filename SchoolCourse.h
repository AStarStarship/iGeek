/* Interactive Gym Environment and Educaitonal Kit @version 0.x
@link    https://github.com/KabukiStarship/iGeek.git
@file    /SchoolCourse.h
@author  Cale McCollough <https://cookingwithcale.org>
@license Copyright (C) 2021 Kabuki Starship (TM) <kabukistarship.com>;
This Source Code Form is subject to the terms of the Mozilla Public License,
v. 2.0. If a copy of the MPL was not distributed with this file, You can obtain
one at <https://mozilla.org/MPL/2.0/>. */
#pragma once
#ifndef IGEEK_SCHOOLCOURSE
#define IGEEK_SCHOOLCOURSE
#include "SchoolCourse.h"
#if SEAM >= IGEEK_CORE
namespace _ {

class SchoolCourse: public _::Operand {

  SchoolCourse();

  ISC SectionCount();

  void CreateRepo();

  /* Script2 operations. */
  virtual const Op* Star(CHC index, Crabs* crabs);
  
  private:
  
  ISC section_count_;   //< The number of different sections of this class.
};
}
#endif
#endif
