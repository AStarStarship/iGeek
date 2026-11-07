// Copyright AStarship <https://astarship.net>.
// iGeek core rollup — the SINGLE place the RL/agent layer implementation is
// included. Seam units (root 01.Core.hxx, world 01.Core.hxx) rely on this being
// pulled once by the _Seams/_Tests.hxx rollup rather than re-including it.
#include <_Config.h>
//
// Order matters (each .hxx includes its own .h; deps come first):
//   1. ASCIICrabs runtime (ROut/RefOut, Test, Stringf, Autoject, ...)
//   2. data nodes: EnvironmentGoal / EnvironmentReward / Env
//   3. math: Tensor
//   4. learning: Policy(Net) / Gym / PPO
//   5. agent tiers + metrics
//
#pragma once
#ifndef _iGeekPackageHXX
#define _iGeekPackageHXX

// 1. ASCIICrabs runtime (pulled ONCE; unguarded rollup — do not re-include).
//    ASCIICrabs is a sibling of the iGeek root, hence ../ASCIICrabs/.
#include "../ASCIICrabs/_Package.hxx"

// 2. Data nodes.
#include "EnvironmentGoal.hxx"
#include "EnvironmentReward.hxx"
#include "Environment.hxx"

// 3. Math.
#include "Tensor.hxx"

// 4. Learning.
#include "PolicyNet.hxx"
#include "Gym.hxx"
#include "PPO.hxx"

// 5. Agent tiers + metrics.
#include "AgentState.hxx"
#include "Metrics.hxx"
#include "Agent.hxx"

#endif  //< _iGeekPackageHXX
