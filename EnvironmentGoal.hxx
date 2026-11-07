// Copyright AStarship <https://astarship.net>.
#pragma once
#include "EnvironmentGoal.h"
namespace iGeek {
using namespace ::_;

EnvironmentGoal::EnvironmentGoal()
    : observation_(_NilP), desired_goal_(_NilP), achieved_goal_(_NilP) {}
EnvironmentGoal::~EnvironmentGoal() {}

const Op* EnvironmentGoal::Star(CHC index, Crabs* crabs) { return _NilP; }

  /* Accessors (vectorized-batch: the policy reads observation, the reward
  compares achieved vs desired). */
Crabs* EnvironmentGoal::Observation() { return observation_; }
Crabs* EnvironmentGoal::DesiredGoal() { return desired_goal_; }
Crabs* EnvironmentGoal::AchievedGoal() { return achieved_goal_; }
const Crabs* EnvironmentGoal::ObservationC() { return observation_; }

  /* Bind a datum container to a slot (no std; the world owns the Crabs). */
void EnvironmentGoal::SetObservation(Crabs* c) { observation_ = c; }
void EnvironmentGoal::SetDesiredGoal(Crabs* c) { desired_goal_ = c; }
void EnvironmentGoal::SetAchievedGoal(Crabs* c) { achieved_goal_ = c; }
} //< namespace iGeek
