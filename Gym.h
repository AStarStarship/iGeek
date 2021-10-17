/* Interactive Gym Environment and Educaitonal Kit @version 0.x
@link    https://github.com/KabukiStarship/iGeek.git
@file    /Gym.h
@author  Cale McCollough <https://cookingwithcale.org>
@license Copyright (C) 2021 Kabuki Starship (TM) <kabukistarship.com>;
This Source Code Form is subject to the terms of the Mozilla Public License,
v. 2.0. If a copy of the MPL was not distributed with this file, You can obtain
one at <https://mozilla.org/MPL/2.0/>. */
#pragma once
#ifndef IGEEK_GYM_DECL
#define IGEEK_GYM_DECL
#include "Env.h"
#include "EnvGoal.h"
#include "../Script2/Room.h"
namespace _ {

class Gym : public Room {
  public:
  
  Gym(const STA* name);

  void AddEnv(Env* env);
};
}  //< namespace _
#endif
