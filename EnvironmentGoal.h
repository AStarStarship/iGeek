// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekEnvironmentGoalH
#define _iGeekEnvironmentGoalH
#include "../ASCIICrabs/Operand.h"
namespace iGeek {
using ::_::Operand;
using ::_::Op;
using ::_::Crabs;

/* An ASCIICrabs Operand data node holding the three goals relevant to one env step:
   the raw observation, the achieved goal, and the desired goal.
Migrated 2026-10-07:
  - `Star(CHN, ...)` -> `Star(CHC, ...)`: the Operand virtual is
    `const Op* Star(CHC index, Crabs* crabs)` in the latest ASCIICrabs
    (CHC = char32_t; School.h already used this form).
  - `_::Room`/`TRoom` members -> `Crabs*` pointers. A `TRoom<CHD,CHD>`
    member instantiates the buggy `TRoom::Main`/`Star` (verified 2026-10-07,
    see ASCIICrabs/AGENT_PLAN.md) and fails to compile. The data-node role
    (store percepts/actions as Crabs datums) is served by a `Crabs*`
    container pointer instead — no template instantiation.
*/
class EnvironmentGoal : public Operand {
 public:

  EnvironmentGoal() : observation_(_NilP), desired_goal_(_NilP), achieved_goal_(_NilP) {}
  ~EnvironmentGoal() {}

  /* ASCIICrabs Operand operation. */
  const Op* Star(CHC index, Crabs* crabs) override;

  /* Accessors (vectorized-batch: the policy reads observation, the reward
  compares achieved vs desired). */
  Crabs* Observation() { return observation_; }
  Crabs* DesiredGoal() { return desired_goal_; }
  Crabs* AchievedGoal() { return achieved_goal_; }
  const Crabs* ObservationC() { return observation_; }

  /* Bind a datum container to a slot (no std; the world owns the Crabs). */
  void SetObservation(Crabs* c) { observation_ = c; }
  void SetDesiredGoal(Crabs* c) { desired_goal_ = c; }
  void SetAchievedGoal(Crabs* c) { achieved_goal_ = c; }

 private:

  Crabs* observation_;
  Crabs* desired_goal_;
  Crabs* achieved_goal_;
};

template <typename Printer>
Printer& PrintTo(Printer& p, EnvironmentGoal& env_goal) {
  return p << "\nEnvironmentGoal";
}

} //< namespace iGeek
#endif
