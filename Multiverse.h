// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekMultiverseH
#define _iGeekMultiverseH
#include "EnvironmentGoal.h"
#include "Gym.h"
namespace iGeek {

/* A container environment that holds many Envs (or many Gyms).
Migrated 2026-10-07:
  - Fixed the copy-pasted guard (was _iGeekEnvH).
  - STANDALONE (not `public _::Room`) — see Environment.h for the verified reason
    (TRoom instantiation is broken upstream; logged to ASCIICrabs/AGENT_PLAN.md).
  - `ComputeReward(..., STA* info)` -> `(..., const CHA* info)`.
*/
class Multiverse {
 public:

  Multiverse(const CHA* name = "multiverse") : name_(name) {}
  virtual ~Multiverse() {}

  const CHA* Name() { return name_; }

  /* Reward for the whole multiverse step: aggregate of each env's
  achieved vs desired EnvironmentGoal. */
  virtual FPC ComputeReward(Crabs* crabs, EnvironmentGoal achieved_goal,
                            EnvironmentGoal desired_goal, const CHA* info) {
    return 0.0f;
  }

 protected:

  const CHA* name_;
};

} //< namespace iGeek
#endif
