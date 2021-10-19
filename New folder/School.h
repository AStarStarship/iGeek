/* Interactive Gym Environment and Educaitonal Kit @version 0.x
@link    https://github.com/KabukiStarship/iGeek.git
@file    /School.h
@author  Cale McCollough <https://cookingwithcale.org>
@license Copyright (C) 2021 Kabuki Starship (TM) <kabukistarship.com>;
This Source Code Form is subject to the terms of the Mozilla Public License,
v. 2.0. If a copy of the MPL was not distributed with this file, You can obtain
one at <https://mozilla.org/MPL/2.0/>. */
#pragma once
#include <_Config.h>
#if SEAM >= IGEEK_CORE
#ifndef IGEEK_COURSE_CODE
#define IGEEK_COURSE_CODE
#include "../Script2/Operand.h"
namespace _ {

class School : public Operand {

  School();

  ISC ClassCount();

  void CreateRepo();

  /* Script2 operations. */
  virtual const Op* Star(CHC index, Crabs* crabs);

  private:

  ISC class_count_;   //< The number of 
      
};
}  // namespace _
#endif
#endif
