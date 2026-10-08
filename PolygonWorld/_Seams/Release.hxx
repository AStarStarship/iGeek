// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>

#if SEAM == ASTARTUP_COOKBOOK_RELEASE
#include "_Debug.hxx"
#else
#include "_Release.hxx"
#endif

using namespace _;

namespace astartup { namespace cookbook { 
inline const CHA* Release (CHA* seam_log, CHA* seam_end, const CHA* args) {
#if SEAM >= ASTARTUP_COOKBOOK_RELEASE
  TEST_BEGIN;

  PRINT_HEADING ("Testing Foo fun.");

#endif
  return 0;
}
}  //< namespace cookbook
}  //< namespace astartup
