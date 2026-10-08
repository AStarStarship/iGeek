// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>

#include "00_foo.h"

#if SEAM >= PROJECT_IGEEK_1
#include "01_bar.h"
#endif

namespace _ {

static const CHA* Test(const CHA* args) {
  return TTestTree<Foo
#if SEAM >= PROJECT_IGEEK_1
                   ,
                   Release
#endif
                   >(seam_log, seam_end, args);
}
}  //< namespace _
