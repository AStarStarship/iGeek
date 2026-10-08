// Copyright AStarship <https://astarship.net>.

#pragma once
#include <module_config.h>

#if SEAM == IGEEK_WIKIWORLD_0
#include "_debug.hxx"
#else
#include "_release.hxx"
#endif

using namespace _;

namespace IGeek { namespace WikiWorld { 
inline const CHA* Core (CHA* seam_log, CHA* seam_end, const CHA* args) {
#if SEAM >= IGEEK_WIKIWORLD_0
  TEST_BEGIN;

  PRINT_HEADING ("Testing Foo Fun.");

#endif
  return 0;
}
}  //< namespace WikiWorld
}  //< namespace IGeek
