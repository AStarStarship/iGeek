// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>

#if SEAM == ASTARTUP_COOKBOOK_TEST
#include "_Debug.hxx"
#else
#include "_Release.hxx"
#endif

using namespace _;

namespace astartup {
namespace cookbook {
inline const CHA* _0_Foo(CHA* seam_log, CHA* seam_end, const CHA* args) {
#if SEAM >= ASTARTUP_COOKBOOK_TEST
  TEST_BEGIN;

  PRINT_HEADING("Testing Foo Fun.");

#endif
  return 0;
}
}  //< namespace cookbook
}  //< namespace astartup
