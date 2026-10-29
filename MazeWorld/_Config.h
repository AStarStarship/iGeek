// Copyright AStarship <https://astarship.net>.
// MazeWorld config: defines the local IGEEK_MAZEWORLD seam ID and sets SEAM to
// it, so MazeWorld.hxx's `#if SEAM >= IGEEK_MAZEWORLD` gate admits the body.
// Mirrors the SpaceWorld local-seam pattern (SpaceConfig.h).
#pragma once
#ifndef CRABS_MAZEWORLD_CONFIG_H
#define CRABS_MAZEWORLD_CONFIG_H
#define IGEEK_MAZEWORLD 24
#ifndef SEAM
#define SEAM IGEEK_MAZEWORLD
#endif
#include "../../ASCIICrabs/_ConfigHeader.h"
#include "../../ASCIICrabs/_ConfigDefault.h"
#include "../../ASCIICrabs/_ConfigFooter.h"
#endif
