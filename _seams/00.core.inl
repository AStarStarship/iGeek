/* Interactive Gym Environment and Educational Kit (iGeek) @version 0.x
@link    https://github.com/KabukiStarship/iGeek.git
@file    /_Seams/00.Core.inl
@author  Cale McCollough <https://cookingwithcale.org>
@license Copyright (C) 2015-21 Kabuki Starship (TM) <kabukistarship.com>;
This Source Code Form is subject to the terms of the Mozilla Public License,
v. 2.0. If a copy of the MPL was not distributed with this file, You can obtain
one at <https://mozilla.org/MPL/2.0/>. */
#include <_Config.h>
#if SEAM == IGEEK_TEST
#include "_Debug.inl"
#else
#include "_Release.inl"
#endif
using namespace _;
namespace _ {
inline const CHA* Core(const CHA* args) {
#if SEAM >= IGEEK_CORE
  TEST_BEGIN;

#endif
  return 0;
}
}  //< namespace _
