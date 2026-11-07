// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekSpaceEnvH
#define _iGeekSpaceEnvH
#include <_Config.h>
#if _Seam >= _iGeekSpaceWorldCore
#include "Environment.h"
#include "SpaceConfig.h"
#include "SpaceMath.h"
#include "SpacePhysics.h"
#include "Stringf.hpp"

namespace iGeek {

/* The spacecraft AS an Environment: a Room/thread an agent can join. The task
   is attitude hold — null the body angular velocity and hold orientation near
   level while staying inside a velocity envelope. The action is a discrete
   torque/force pulse on one body axis, which is what an RCS jet pair produces.
   The env IS the Environment, so it owns the Room and the manifest a joining
   agent reads. */
class SpaceEnv : public Environment
{
 public:
  enum {
    ObservationLength = 13,
    ActionCount = 13,   //< 0 hold, 1..12 = +/- torque and force on x, y, z
    MaxSteps = 600,
  };
  SpaceEnv(IUD seed = 7);
  void Reset();
  FPC Step(ISC action);
  void Observe();
  const FPC* Observation() { return observation_; }
  BOL Done() { return done_ || steps_ >= ISC(MaxSteps); }
  ISC Steps() { return steps_; }
  TSpaceState& State() { return state_; }

 private:
  TSpaceState state_;
  TSpaceParams params_;
  TSpaceControl control_;
  FPC observation_[ObservationLength] = {};
  ISC steps_;
  BOL done_;
  IUD seed_;
};

}  //< namespace iGeek
#endif  //< _Seam >= _iGeekSpaceWorldCore
#endif  //< _iGeekSpaceEnvH
