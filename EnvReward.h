// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_ENVREWARD_DECL
#define IGEEK_ENVREWARD_DECL
#include "../ASCIICrabs/Operand.h"
namespace _ {

/* Wraps the scalar reward for one env step as a Script2 data node.
Migrated 2026-10-07:
  - Fixed the copy-pasted include guard (was IGEEK_ENVGOAL_DECL).
  - `Star(CHN, ...)` -> `Star(CHC, ...)`.
  - Dropped the `TRoom<CHD, CHD>` member (instantiating it fails — verified
    2026-10-07, see ASCIICrabs/AGENT_PLAN.md). The scalar reward is the FPC
    member; the optional datum container is a `Crabs*` pointer.
*/
class EnvReward : public Operand {
 public:

  EnvReward() : reward_(0.0f), crabs_(NILP) {}
  ~EnvReward() {}

  /* Script2 operation. */
  const Op* Star(CHC index, Crabs* crabs) override;

  /* Set / get the scalar reward for this step. */
  void SetReward(FPC r) { reward_ = r; }
  FPC Reward() const { return reward_; }

  /* Optional datum container to stream the reward into (may be NILP). */
  void SetCrabs(Crabs* c) { crabs_ = c; }
  Crabs* GetCrabs() const { return crabs_; }

 private:

  FPC reward_;
  Crabs* crabs_;
};

template <typename Printer>
Printer& PrintTo(Printer& p, const EnvReward& env_reward) {
  return p << "\nEnvReward";
}

}  // namespace _
#endif
