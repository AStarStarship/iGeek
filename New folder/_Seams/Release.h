/* iGeek @version 0.x
@link    https://github.com/KabukiStarship/iGeek.git
@file    /_Seams/release.inl
@author  Cale McCollough <https://cookingwithcale.org>
@license Copyright (C) 2015-21 Kabuki Starship (TM) <kabukistarship.com>;
This Source Code Form is subject to the terms of the Mozilla Public License,
v. 2.0. If a copy of the MPL was not distributed with this file, You can obtain
one at <https://mozilla.org/MPL/2.0/>. */
#pragma once
#include <_Config.h>
#if SEAM == IGEEK_RELEASE
#include "_Debug.inl"
#else
#include "_Release.inl"
#endif
namespace _ {

inline const CHA* Release (const CHA* args) {
#if SEAM >= IGEEK_RELEASE
  TEST_BEGIN;
#endif
  return 0;
}
}  //< namespace _
