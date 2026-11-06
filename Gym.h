// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekGymH
#define _iGeekGymH
#include "Environment.h"
#include "EnvironmentGoal.h"
#include "EnvironmentManifest.h"
namespace iGeek {

/* Max parallel envs in one Gym batch. Fixed-size (no std vector).
   8 is a sane console default; raise for larger vectorization. */
enum { GymEnvMax = 8 };

/* A container for one or more Envs. vectorized-batch: this is the analog of
   the fused-kernel vectorized `Env*`/`Agent*` — a Gym batches N parallel envs so
   the policy can step them all in one call (the `Agent{observations,
   actions, rewards, terminals, action_mask}` arrays).
Migrated 2026-10-07:
  - STANDALONE (not `public TRoom`) — see Environment.h for the verified reason
    (TRoom instantiation is broken upstream; logged to ASCIICrabs/AGENT_PLAN.md).
  - `Gym(const CHA* name)` (was `const STA*` dead type).
  - Added the vectorized batch accessors the PPO loop needs (see
    iGeek AGENT_PLAN.md, batch section).

Buffer ownership, shape, mutability, lifetime:
  - All five batch arrays (observations_, actions_, rewards_,
    terminals_, action_mask_) are allocated in the subclass constructor
    (either heap `new[]` or stack storage) and freed in the subclass
    destructor. The base Gym never allocates or frees them.
  - Shape: flat 1-D, indexed [env * feature + i].
    observations_ has env_count * obs_len_ floats.
    actions_, rewards_, terminals_ have env_count elements.
    action_mask_ has env_count * action_count_ floats.
  - Mutability: the policy WRITES actions_, the Gym WRITES
    observations_/rewards_/terminals_/action_mask_ after StepBatch.
    The PPO loop reads rewards_/terminals_/observations_ after
    StepBatch, writes actions_ before StepBatch.
  - Lifetime: valid from constructor return through StepBatch/
    ResetBatch calls. Invalid after the destructor runs.
  - LoadDLL is a stub: it does nothing and returns void. Calling it
    has no effect — DLL loading is not implemented.
  - AddEnv stores the pointer in envs_[] but does not take ownership;
    the caller must keep the Env alive for the Gym's lifetime.
*/
class Gym {
 public:

  Gym(const CHA* gym_name = "gym")
      : gym_name_(gym_name), env_count_(0), observations_(_NilP),
        actions_(_NilP), rewards_(_NilP), terminals_(_NilP), action_mask_(_NilP),
        obs_len_(0), action_count_(0) {
    EnvironmentManifestInit(&manifest_);
  }
  virtual ~Gym() {}

  const CHA* Name() { return gym_name_; }
  EnvironmentManifest& Manifest() { return manifest_; }
  void SetManifest(EnvironmentManifest& m) { manifest_ = m; }

  void AddEnv(Environment* env);

  BOL LoadDLL(const CHA* library);

  /* --- Vectorized batch interface (vectorized-batch) -------------------
  The PPO loop never calls a single env; it steps the whole batch. These
  return pointers to the flat per-batch arrays (owned by the Gym,
  preallocated at init — the the reference PPO kernel "preallocate at init, free at close"
  pattern). The arrays are indexed [env_count][feature]. */

  /* Number of envs in the batch (= EnvironmentCount of the added envs). */
  ISC EnvironmentCount() { return env_count_; }

  /* Flat observation buffer: [env_count][obs_len]. */
  FPC* Observations() { return observations_; }
  /* Flat action buffer (policy writes, Gym reads): [env_count]. */
  ISC* Actions() { return actions_; }
  /* Flat reward buffer (Gym writes, PPO reads): [env_count]. */
  FPC* Rewards() { return rewards_; }
  /* Flat terminal flag (1.0 done / 0.0 running): [env_count]. */
  FPC* Terminals() { return terminals_; }
  /* Flat action mask (1.0 legal / 0.0 illegal): [env_count][action_count]. */
  FPC* ActionMask() { return action_mask_; }

  /* Step the whole batch by the actions in Actions(); fills Rewards/
  Terminals/ActionMask and advances observations. One call = one batch
  step (one batch step). Virtual so a toy gym can override it for tests. */
  virtual void StepBatch();

  /* Reset every env in the batch to a fresh episode (full-batch reset). */
  virtual void ResetBatch();

 protected:

  const CHA* gym_name_;
  EnvironmentManifest manifest_;
  ISC env_count_;
  FPC* observations_;
  ISC* actions_;
  FPC* rewards_;
  FPC* terminals_;
  FPC* action_mask_;
  ISC obs_len_;
  ISC action_count_;
  Environment* envs_[GymEnvMax];
};

} //< namespace iGeek
#endif
