// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekPPOH
#define _iGeekPPOH
#include "Gym.h"
#include "Policy.h"
#include "Tensor.h"
namespace iGeek {
using namespace ::_;

/* PPO training loop over a vectorized Gym + a Policy.
vectorized-batch: mirrors the structure of the local reference Proximal Poicy 
Optimzation implementation's src/algo.cu fused PPO kernel — clipped policy loss,
value-clip (vf-clip), entropy bonus, GAE advantages — but in CPU FPC (no CUDA) 
on the ASCIICrabs no-stdlib type system.

The rollout buffer holds one full batch-step of experience:
actions, rewards, terminals, old logits, old logp, values, observation,
action_mask.  Preallocated at init, freed at close (preallocate-at-init 
pattern). */
struct PPOStats {
  FPC policy_loss_;
  FPC value_loss_;
  FPC entropy_;
  FPC approx_kl_;
  FPC explained_var_;
};

class PPO {
 public:

  /* 
  @param gym     The vectorized environment (batch width = EnvironmentCount).
  @param policy  The trainable policy to update.
  @param horizon Steps per rollout (trajectory length).
  @param epochs  PPO update epochs per rollout. */
  PPO(Gym& gym, Policy& policy, ISC horizon, ISC epochs);

  ~PPO();

  /* Run one rollout (collect horizon batch-steps) then update the policy
  for `epochs` epochs. Returns the stats of the last epoch. */
  PPOStats TrainStep();

  /* GAE hyperparameters (standard defaults). */
  void SetGamma(FPC g);
  void SetLambda(FPC l);
  void SetClipEps(FPC e);
  void SetVfClipEps(FPC e);
  void SetEntropyCoef(FPC c);
  void SetValueCoef(FPC c);

 private:

  /* Collect one rollout into the buffer. */
  void CollectRollout();
  
  /* Compute GAE advantages + returns from the buffer. */
  void ComputeAdvantages();

  /* One epoch of clipped PPO+value+entropy over minibatches. */
  PPOStats UpdatePolicy();

  Gym& gym_;
  Policy& policy_;
  ISC horizon_;
  ISC epochs_;

  // Rollout buffer (batched: [horizon * env_count] rows).
  TensorFPC observation_; //< [H*B, observation_len]
  TensorFPC old_logits_;  //< [H*B, A]
  TensorFPC values_;      //< [H*B]
  ISC* actions_;          //< [H*B]
  FPC* logp_;             //< [H*B]
  FPC* rewards_;          //< [H*B]
  FPC* terminals_;        //< [H*B]
  FPC* advantages_;       //< [H*B]
  FPC* returns_;          //< [H*B]

  // Hyperparameters.
  FPC gamma_;
  FPC lam_;
  FPC clip_eps_;
  FPC vf_clip_eps_;
  FPC ent_coef_;
  FPC vf_coef_;
};

}        //< namespace iGeek
#endif   //< _iGeekPPOH
