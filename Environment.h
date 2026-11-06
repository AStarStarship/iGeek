// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekEnvironmentH
#define _iGeekEnvironmentH
#include "EnvironmentGoal.h"
namespace iGeek {
using ::_::Crabs;

/* Base class for an iGeek world environment.

Migrated 2026-10-07 to the latest ASCIICrabs API:
  - STANDALONE (not `public TRoom`). Verified 2026-10-07: instantiating
    `TRoom<CHD, CHD>` against the latest ASCIICrabs FAILS to compile —
    `TRoom::Main()`/`Star()` carry pre-existing upstream bugs
    (nil `this_->ExecAll()` -> `TDoor` has no `ExecAll`; `OpFirst/OpLast`
    const-ISC* vs const-DTB* mismatch; `OpPush` undeclared) at
    ../ASCIICrabs/Room.hpp:297,310,332,341,348. The same blocker forced the
    card world's `BlackjackEnv` to be standalone. So the iGeek env/gym base
    is standalone too; the ASCIICrabs Operand "data node" role (storing percepts/actions
    as Crabs datums) is opt-in per world, NOT the base class. Logged to
    ../ASCIICrabs/AGENT_PLAN.md.
  - `STA*` (dead type; in memory `const CHA*`) replaced with `const CHA*`.
  - `FPC` (float32) is the per-step scalar reward — matches the fused-kernel
    `Agent.rewards` and the Qualia plan's "single scalar reward" decision.
*/
class Environment {
 public:

  Environment(const CHA* env_name = "env", ISC state_count = 2)
      : environment_name_(env_name), state_count_(state_count) {}
  virtual ~Environment() {}

  const CHA* Name() { return environment_name_; }
  ISC StateCount() { return state_count_; }

  /* Per-step reward. Achieved vs desired EnvironmentGoal -> scalar reward.
  vectorized-batch: reward is a single scalar per env per step (FPC); multi-
  metric episode data streams separately, NOT through EnvironmentReward.
  @param crabs        The Crabs to read/write the reward to. May be _NilP.
  @param achieved     What the agent actually did this step.
  @param desired      What we want the agent to do.
  @param info         Human-readable info string. May be _NilP.
  @return             The scalar reward for this step (FPC). */
  virtual FPC ComputeReward(Crabs* crabs, EnvironmentGoal achieved_goal,
                            EnvironmentGoal desired_goal, const CHA* info) = 0;

  /* Number of parallel envs this Gym manages (vectorization width).
  Default 1 (single-agent). A vectorized Gym overrides this. */
  virtual ISC EnvironmentCount() { return 1; }

  /* Reset the environment to a fresh episode. Deterministic per seed. */
  virtual void Reset() {}

  /* True when the current episode is over for this env. */
  virtual BOL Done() { return false; }

 protected:

  const CHA* environment_name_;
  ISC state_count_;
};

template <typename Printer>
Printer& PrintTo(Printer& p, Environment& env) {
  return p << "\nEnv";
}

} //< namespace iGeek
#endif
