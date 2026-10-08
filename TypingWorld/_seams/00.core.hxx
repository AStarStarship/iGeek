// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_config.h>

#if SEAM == ASTARTUP_COOKBOOK_TEST
#include "_debug.hxx"
#else
#include "_release.hxx"
#endif

using namespace _;

namespace astartup {
namespace cookbook {
inline const CHA* _0_Foo(const CHA* args) {
#if SEAM >= ASTARTUP_COOKBOOK_TEST
  TEST_BEGIN;

  PRINT_HEADING("Testing Foo Fun.");

#endif
  return 0;
}
}  // namespace cookbook
}  // namespace astartup
