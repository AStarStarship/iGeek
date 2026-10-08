// Copyright AStarship <https://astarship.net>.
#include <_Config.h>
#if SEAM == IGEEK_TEST
#include "_Debug.hxx"
#else
#include "_Release.hxx"
#endif
using namespace _;
namespace _ {
inline const CHA* Core(const CHA* args) {
#if SEAM >= IGEEK_CORE
  TEST_BEGIN;

#endif
  return 0;
}
}  //< namespace _
