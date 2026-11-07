// Copyright AStarship <https://astarship.net>.
// MazeWorld config: this world's seam ID is _iGeekMazeWorldCore, defined once in
// the global seam table (_Seams/_Seams.cpp) alongside every other module. Do NOT
// define a local seam number here: the previous local value (24) collided with
// _CrabsRelease in that table, and assigning _Seam locally silently excluded every
// gate above it.
#pragma once
#ifndef _iGeekMazeWorldConfigH
#define _iGeekMazeWorldConfigH
#include "../../../ASCIICrabs/_Config.h"
#endif
