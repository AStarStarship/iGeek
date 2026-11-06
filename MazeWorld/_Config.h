// Copyright AStarship <https://astarship.net>.
// MazeWorld config: defines the local _iGeekMazeWorld seam ID and sets _Seam to
// it, so MazeWorld.hxx's `#if _Seam >= _iGeekMazeWorld` gate admits the body.
// Mirrors the SpaceWorld local-seam pattern (SpaceConfig.h).
#pragma once
#ifndef _iGeekMazeWorldConfigH
#define _iGeekMazeWorldConfigH
#define _iGeekMazeWorld 24
#ifndef _Seam
#define _Seam _iGeekMazeWorld
#endif
#include "../../ASCIICrabs/_ConfigHeader.h"
#include "../../ASCIICrabs/_ConfigDefault.h"
#include "../../ASCIICrabs/_ConfigFooter.h"
#endif
