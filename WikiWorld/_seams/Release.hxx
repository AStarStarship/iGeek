// Copyright AStarship <https://astarship.net>.

#pragma once
#include <module_config.h>

#if SEAM == IGEEKWIKIWORLD_0
#include "_Debug.hxx"
#else
#include "_Release.hxx"
#endif

using namespace _;

namespace IGeek { namespace WikiWorld { 
inline const CHA* Release (CHA* seam_log, CHA* seam_end, const CHA* args) {
#if SEAM >= IGEEKWIKIWORLD_0
  TEST_BEGIN;

  PRINT_HEADING ("Testing Foo fun.");

#endif
  return 0;
}
}  //< namespace WikiWorld
}  //< namespace IGeek
