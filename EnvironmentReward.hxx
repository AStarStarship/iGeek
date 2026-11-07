// Copyright AStarship <https://astarship.net>.
#pragma once
#include "EnvironmentReward.h"
namespace iGeek {
using namespace ::_;

EnvironmentReward::EnvironmentReward() : reward_(0.0f), crabs_(_NilP) {}
EnvironmentReward::~EnvironmentReward() {}

const Op* EnvironmentReward::Star(CHC index, Crabs* crabs) {
return _NilP;
}

/* Set / get the scalar reward for this step. */
void EnvironmentReward::SetReward(FPC r) { reward_ = r; }
FPC EnvironmentReward::Reward() { return reward_; }

/* Optional datum container to stream the reward into (may be _NilP). */
void EnvironmentReward::SetCrabs(Crabs* c) { crabs_ = c; }
Crabs* EnvironmentReward::GetCrabs() { return crabs_; }

} //< namespace iGeek
