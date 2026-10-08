// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>
#if SEAM == IGEEK_RELEASE
#include "_Debug.hxx"
#else
#include "_Release.hxx"
#endif
namespace _ {

inline const CHA* Release (const CHA* args) {
#if SEAM >= IGEEK_RELEASE
  TEST_BEGIN;
#endif
  return 0;
}
}  //< namespace _
