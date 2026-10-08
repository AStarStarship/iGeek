// Copyright AStarship <https://astarship.net>.
// Gym.hxx — base Gym batch-method stubs. The base Gym is a container; the
// concrete vectorized behavior lives in subclasses (BlackjackGym is
// standalone, TToyGym overrides both). These base stubs satisfy the vtable
// (StepBatch/ResetBatch are virtual) and do nothing — a bare Gym has no envs.
#include "Gym.h"

namespace _ {

void Gym::StepBatch() {
  // Base does nothing; subclasses override.
}

void Gym::ResetBatch() {
  // Base does nothing; subclasses override.
}

void Gym::AddEnv(Env* env) {
  if (env == NILP || env_count_ >= GymEnvMax) {
    return;
  }
  envs_[env_count_++] = env;
}

void Gym::LoadDLL(const CHA* library) {
  (void)library;  // DLL loading is a later milestone.
}

}  // namespace _
