// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekEnvironmentRewardH
#define _iGeekEnvironmentRewardH
#include "../ASCIICrabs/Operand.h"
namespace iGeek {
using ::_::Operand;
using ::_::Op;
using ::_::Crabs;

/* Wraps the scalar reward for one env step as an ASCIICrabs Operand data node.
Migrated 2026-10-07:
  - Fixed the copy-pasted include guard (was _iGeekEnvironmentGoalH).
  - `Star(CHN, ...)` -> `Star(CHC, ...)`.
  - Dropped the `TRoom<CHD, CHD>` member (instantiating it fails — verified
    2026-10-07, see ASCIICrabs/AGENT_PLAN.md). The scalar reward is the FPC
    member; the optional datum container is a `Crabs*` pointer.
*/
class EnvironmentReward : public Operand {
 public:

  EnvironmentReward() : reward_(0.0f), crabs_(_NilP) {}
  ~EnvironmentReward() {}

  /* ASCIICrabs Operand operation. */
  const Op* Star(CHC index, Crabs* crabs) override;

  /* Set / get the scalar reward for this step. */
  void SetReward(FPC r) { reward_ = r; }
  FPC Reward() { return reward_; }

  /* Optional datum container to stream the reward into (may be _NilP). */
  void SetCrabs(Crabs* c) { crabs_ = c; }
  Crabs* GetCrabs() { return crabs_; }

 private:

  FPC reward_;
  Crabs* crabs_;
};

template <typename Printer>
Printer& PrintTo(Printer& p, EnvironmentReward& env_reward) {
  return p << "\nEnvironmentReward";
}

} //< namespace iGeek
#endif
