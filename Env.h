/* Interactive Gym Environment and Educaitonal Kit @version 0.x
@link    https://github.com/KabukiStarship/iGeek.git
@file    /Env.h
@author  Cale McCollough <https://cookingwithcale.org>
@license Copyright (C) 2021 Kabuki Starship (TM) <kabukistarship.com>;
This Source Code Form is subject to the terms of the Mozilla Public License,
v. 2.0. If a copy of the MPL was not distributed with this file, You can obtain
one at <https://mozilla.org/MPL/2.0/>. */
#pragma once
#ifndef IGEEK_ENV_DECL
#define IGEEK_ENV_DECL
#include "EnvGoal.h"
#include "../Script2/Room.h"
namespace _ {

class Env : public Room {
  public:
  
  Env();

  virtual void ComputeReward(Crabs* crabs, EnvGoal achieved_goal,
                             EnvGoal desired_goal, STA* info) = 0;
};

}  //< namespace _
#endif
