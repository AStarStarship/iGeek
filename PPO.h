// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_PPO_DECL
#define IGEEK_PPO_DECL
#include "Gym.h"
#include "Policy.h"
#include "Tensor.h"
namespace _ {

/* PPO training loop over a vectorized Gym + a TPolicy.
   vectorized-batch: mirrors the structure of the local reference PPO implementation's
   src/algo.cu fused PPO kernel — clipped policy loss, value-clip (vf-clip),
   entropy bonus, GAE advantages — but in CPU FPC (no CUDA) on the
   ASCIICrabs no-stdlib type system.

   The rollout buffer holds one full batch-step of experience:
     actions, rewards, terminals, old logits, old logp, values, obs,
     action_mask.  Preallocated at init, freed at close (preallocate-at-init pattern).
*/
struct TPPOStats {
  FPC policy_loss_;
  FPC value_loss_;
  FPC entropy_;
  FPC approx_kl_;
  FPC explained_var_;
};

class TPPO {
 public:

  /* @param gym       The vectorized environment (batch width = EnvCount).
     @param policy    The trainable policy to update.
     @param horizon   Steps per rollout (trajectory length).
     @param epochs    PPO update epochs per rollout. */
  TPPO(Gym& gym, TPolicy& policy, ISC horizon, ISC epochs);

  ~TPPO();

  /* Run one rollout (collect horizon batch-steps) then update the policy
     for `epochs` epochs. Returns the stats of the last epoch. */
  TPPOStats TrainStep();

  /* GAE hyperparameters (standard defaults). */
  void SetGamma(FPC g) { gamma_ = g; }
  void SetLambda(FPC l) { lam_ = l; }
  void SetClipEps(FPC e) { clip_eps_ = e; }
  void SetVfClipEps(FPC e) { vf_clip_eps_ = e; }
  void SetEntropyCoef(FPC c) { ent_coef_ = c; }
  void SetValueCoef(FPC c) { vf_coef_ = c; }

 private:

  /* Collect one rollout into the buffer. */
  void CollectRollout();
  /* Compute GAE advantages + returns from the buffer. */
  void ComputeAdvantages();
  /* One epoch of clipped PPO+value+entropy over minibatches. */
  TPPOStats UpdatePolicy();

  Gym& gym_;
  TPolicy& policy_;
  ISC horizon_;
  ISC epochs_;

  /* Rollout buffer (batched: [horizon * env_count] rows). */
  TTensor obs_;        //< [H*B, obs_len]
  TTensor old_logits_; //< [H*B, A]
  TTensor values_;     //< [H*B]
  ISC* actions_;       //< [H*B]
  FPC* logp_;          //< [H*B]
  FPC* rewards_;       //< [H*B]
  FPC* terminals_;     //< [H*B]
  FPC* advantages_;    //< [H*B]
  FPC* returns_;       //< [H*B]

  /* Hyperparameters. */
  FPC gamma_;
  FPC lam_;
  FPC clip_eps_;
  FPC vf_clip_eps_;
  FPC ent_coef_;
  FPC vf_coef_;
};

}  // namespace _
#endif
