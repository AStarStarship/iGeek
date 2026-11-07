// Copyright AStarship <https://astarship.net>.
#include "Environment.h"
#pragma once
#ifndef _iGeekEnvironmentHXX
#define _iGeekEnvironmentHXX
namespace iGeek {
using namespace ::_;

Environment::Environment(const CHA* env_name, ISC state_count)
    : environment_name_(env_name), state_count_(state_count) {
  EnvironmentManifestInit(&manifest_);
}
    
Environment::~Environment() {}

const CHA* Environment::Name() { return environment_name_; }
ISC Environment::StateCount() { return state_count_; }

/* Per-step reward. Achieved vs desired EnvironmentGoal -> scalar reward.
vectorized-batch: reward is a single scalar per env per step (FPC); multi-
metric episode data streams separately, NOT through EnvironmentReward.
@param crabs        The Crabs to read/write the reward to. May be _NilP.
@param achieved     What the agent actually did this step.
@param desired      What we want the agent to do.
@param info         Human-readable info string. May be _NilP.
@return             The scalar reward for this step (FPC). */
FPC Environment::ComputeReward(Crabs* crabs, EnvironmentGoal achieved_goal,
                               EnvironmentGoal desired_goal, const CHA* info) {
  return 0;
}

/* Number of parallel envs this Gym manages (vectorization width).
Default 1 (single-agent). A vectorized Gym overrides this. */
ISC Environment::EnvironmentCount() { return 1; }

/* Reset the environment to a fresh episode. Deterministic per seed. */
void Environment::Reset() {}

/* True when the current episode is over for this env. */
BOL Environment::Done() { return false; }
}       //< namespace iGeek
#endif  //< _iGeekEnvironmentHXX
