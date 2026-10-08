// Copyright AStarship <https://astarship.net>.
// Probe include for the headless PacWorld env + the iGeek RL core.
// Defines IGEEK_PACWORLD seam, pulls in config + RL + PacWorld headers.
#pragma once
#include "../../ASCIICrabs/_ConfigHeader.h"
#include "../../ASCIICrabs/_ConfigDefault.h"
#include "../../ASCIICrabs/_ConfigFooter.h"

// Seam levels for the PacWorld world (local to this probe; the real world
// _Seams.h would define these).
#ifndef IGEEK_PACWORLD
#define IGEEK_PACWORLD 900
#endif

#include "../Tensor.h"
#include "../Policy.h"
#include "../Env.h"
#include "../Gym.h"
#include "../PPO.h"

#include "PacWorld.h"
#include "../Tensor.hxx"
#include "../PolicyNet.hxx"
#include "../LinearPolicy.hxx"
#include "../PPO.hxx"
#include "../Gym.hxx"
#include "PacWorld.hxx"
