// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>
#include "_Release.hxx"
using namespace _;
namespace iGeek {
namespace autopilot {
inline const CHA* Release (const CHA* args) {
#if SEAM >= IGEEK_RELEASE
  TEST_BEGIN;
#endif
  return 0;
}
} //< namespace autopilot
} //< namespace iGeek
