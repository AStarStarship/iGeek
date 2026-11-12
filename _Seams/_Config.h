// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekSeamsConfig
#define _iGeekSeamsConfig
#include "../../ASCIICrabs/_Config.h"
#undef _Seam
/* Every seam unit re-includes this header, so the seam is re-asserted here too
   or the first include after the config chain resets it. */
#define _Seam _SeamN
#endif  //< _iGeekSeamsConfig
