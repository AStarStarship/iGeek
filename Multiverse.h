// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_MULTIVERSE_DECL
#define IGEEK_MULTIVERSE_DECL
#include "EnvGoal.h"
#include "Gym.h"
namespace _ {

/* A container environment that holds many Envs (or many Gyms).
Migrated 2026-10-07:
  - Fixed the copy-pasted guard (was IGEEK_ENV_DECL).
  - STANDALONE (not `public _::Room`) — see Env.h for the verified reason
    (TRoom instantiation is broken upstream; logged to ASCIICrabs/AGENT_PLAN.md).
  - `ComputeReward(..., STA* info)` -> `(..., const CHA* info)`.
*/
class Multiverse {
 public:

  Multiverse(const CHA* name = "multiverse") : name_(name) {}
  virtual ~Multiverse() {}

  const CHA* Name() const { return name_; }

  /* Reward for the whole multiverse step: aggregate of each env's
  achieved vs desired EnvGoal. */
  virtual FPC ComputeReward(Crabs* crabs, EnvGoal achieved_goal,
                            EnvGoal desired_goal, const CHA* info) {
    return 0.0f;
  }

 protected:

  const CHA* name_;
};

}  // namespace _
#endif
