// Copyright AStarship <https://astarship.net>.

#pragma once
#include <module_config.h>

#if SEAM == THEATER_0
#include "_debug.hxx"
#else
#include "_release.hxx"
#endif

using namespace _;

namespace project { namespace theater { 
inline const CHA* _1_Bar (CHA* seam_log, CHA* seam_end, const CHA* args) {
#if SEAM >= THEATER_0
  TEST_BEGIN;

  PRINT_HEADING ("Testing Foo fun.");

#endif
  return 0;
}
}  //< namespace theater
}  //< namespace project
