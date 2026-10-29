// Copyright AStarship <https://astarship.net>.
// iGeek core rollup — the SINGLE place the RL/agent layer implementation is
// included. Seam units (root 01.Core.hxx, world 01.Core.hxx) rely on this being
// pulled once by the _Seams/_Tests.hxx rollup rather than re-including it.
//
// Order matters (each .hxx includes its own .h; deps come first):
//   1. ASCIICrabs runtime (COut/StdOut, Test, Stringf, Autoject, ...)
//   2. data nodes: EnvGoal / EnvReward / Env
//   3. math: Tensor
//   4. learning: Policy(Net) / Gym / PPO
//   5. agent tiers + metrics
//
#pragma once
#ifndef IGEEK_PACKAGE
#define IGEEK_PACKAGE 1

// 1. ASCIICrabs runtime (pulled ONCE; unguarded rollup — do not re-include).
//    ASCIICrabs is a sibling of the iGeek root, hence ../ASCIICrabs/.
#include "../ASCIICrabs/_Package.hxx"

// 2. Data nodes.
#include "EnvGoal.h"
#include "EnvGoal.hxx"
#include "EnvReward.h"
#include "EnvReward.hxx"
#include "Env.h"
#include "Env.hxx"

// 3. Math.
#include "Tensor.h"
#include "Tensor.hxx"

// 4. Learning.
#include "Policy.h"
#include "PolicyNet.hxx"
#include "Gym.h"
#include "Gym.hxx"
#include "PPO.h"
#include "PPO.hxx"

// 5. Agent tiers + metrics.
#include "AgentState.h"
#include "AgentState.hxx"
#include "Metrics.h"
#include "Metrics.hxx"
#include "Agent.h"
#include "Agent.hxx"

#endif  // IGEEK_PACKAGE
