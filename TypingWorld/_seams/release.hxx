// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_config.h>

#if SEAM == ASTARTUP_COOKBOOK_RELEASE
#include "_debug.hxx"
#else
#include "_release.hxx"
#endif

using namespace _;

namespace astartup { namespace cookbook { 
inline const CHA* Release (const CHA* args) {
#if SEAM >= ASTARTUP_COOKBOOK_RELEASE
  TEST_BEGIN;

  PRINT_HEADING ("Testing Foo fun.");

#endif
  return 0;
}
}  //< namespace cookbook
}  //< namespace astartup
