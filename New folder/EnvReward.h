/* Interactive Gym Environment and Educaitonal Kit @version 0.x
@link    https://github.com/KabukiStarship/iGeek.git
@file    /EnvGoal.h
@author  Cale McCollough <https://cookingwithcale.org>
@license Copyright (C) 2021 Kabuki Starship (TM) <kabukistarship.com>;
This Source Code Form is subject to the terms of the Mozilla Public License,
v. 2.0. If a copy of the MPL was not distributed with this file, You can obtain
one at <https://mozilla.org/MPL/2.0/>. */
#pragma once
#ifndef IGEEK_ENVGOAL_DECL
#define IGEEK_ENVGOAL_DECL
#include "../Script2/Operand.h"
namespace _ {

class EnvReward : public Operand {

  public:

  EnvReward();

  // Scrtip2 opertaions.
  virtual const Op* Star(CHN index, Crabs* crabs);

  private:

  _::Room reward_;
};

template <typename Printer>
Printer& PrintTo(Printer& p, const EnvReward& env_reward) {
  return p << "\nEnvGoal";
}

}  // namespace _
#endif
