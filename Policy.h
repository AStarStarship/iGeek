// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekPolicyH
#define _iGeekPolicyH
#include "Tensor.h"
namespace iGeek {

/* A transformer "block": MHSA + FFN, each with residual + layernorm.
POD of TensorFPC weights (no logic; the forward logic is in PolicyNet.hxx). */
struct TransformerBlock {
  ISC dim_;
  ISC heads_;
  // MHSA projections.
  TensorFPC W_q, W_k, W_v, W_o;  // dim x dim each.
  TensorFPC b_q, b_k, b_v, b_o;  // dim x 1 each.
  // FFN.
  TensorFPC W_f1, W_f2;  // dim x (2dim), (2dim) x dim.
  TensorFPC b_f1, b_f2;
};

/* A trainable policy: observation -> (action logits, value).
vectorized-batch: this is the neural head that the fused-kernel fused PPO
kernel calls — given a batch of observation it produces `logits` (action
distribution) and `values_pred` (value baseline), and the PPO update
writes gradients back into `grad_logits`/`grad_value`.

Kept as an INTERFACE (pure virtuals) so:
- TransformerPolicy (the compact transformer net, milestone M3/M4) implements it.
- A hand-written policy (e.g. the card world's hit-to-17) can implement
   it for baselines, no network needed.
- The PPO loop depends only on this, not on any concrete net.

All tensors are batched [B, ...] where B = the Gym's EnvironmentCount. */
class Policy {
 public:

  virtual ~Policy() {}

  /* Action space size (A). For blackjack: 2 (hit/stand). */
  virtual ISC ActionCount() = 0;
  /* Observation feature length (observation_len). */
  virtual ISC ObservationLength() = 0;

  /* Forward: observation [B, observation_len] -> logits [B, A] + values [B].
  Deterministic for a given observation (same weights, no sampling here). */
  virtual void Forward(TensorFPC observation, TensorFPC& logits,
                       TensorFPC& values) = 0;

  /* Sample an action per row from the (categorical) logits. 
  Writes chosen action index into actions_[b] and log-prob into logp_[b]. Uses 
  the world RNG (seeded + clamped) for the sampling noise. */
  virtual void SampleActions(TensorFPC logits, ISC* actions,
                             FPC* logp) = 0;

  /* Backward: given the current logits, the taken actions, the advantages,
  and the old log-probs, compute gradients w.r.t. the action logits and
  the value head. These are the EXACT quantities the fused PPO kernel
  emits (grad_logits, grad_value). A concrete net then backprops through
  its own weights.

  Policy surrogate (per row b): L_b = -logp[b] * adv[b], where
  logp[b] = logits[b, a_b] - logsumexp(logits[b]).
  Value loss (per row b): V_b = (value[b] - return[b])^2  (vf-clip later).

  @param logits   Current policy logits [B, A].
  @param actions  The taken action per row (length B).
  @param advantages  Per-row advantage [B].
  @param old_logp  Per-row log-prob of the taken action under the old policy [B]
      (for the PPO ratio; M4 uses it for the surrogate gradient).
  @param values   Current value-head output [B].
  @param returns  Per-row target return (adv + value) [B].
  @param grad_logits  OUT: dL/dlogits [B, A].
  @param grad_value   OUT: dV/dvalue [B]. */
  virtual void Backward(TensorFPC logits, const ISC* actions,
      TensorFPC advantages, TensorFPC old_logp, TensorFPC values, 
      TensorFPC returns, TensorFPC& grad_logits, TensorFPC& grad_value) = 0;

  /* Learn rate (Adam/lr). vectorized-batch: the PPO kernel exposes lr. */
  virtual void SetLearnRate(FPC lr) { lr_ = lr; }
  FPC LearnRate() { return lr_; }

 protected:

  FPC lr_;
};

/* Concrete compact transformer policy (milestone M3/M4). Declared here
so the PPO loop can name it; implemented in PolicyNet.hxx once TensorFPC
ops land. Kept out of this header's required surface so the interface
compiles before the net does. */
class TransformerPolicy : public Policy {
 public:

  TransformerPolicy(ISC observation_len, ISC action_count, ISC dim, ISC layers);
  ~TransformerPolicy() override;

  ISC ActionCount() override;
  ISC ObservationLength() override;
  void Forward(TensorFPC observation, TensorFPC& logits, TensorFPC& values) override;
  void SampleActions(TensorFPC logits, ISC* actions, FPC* logp) override;
  void Backward(TensorFPC logits, const ISC* actions,
                TensorFPC advantages, TensorFPC old_logp,
                TensorFPC values, TensorFPC returns,
                TensorFPC& grad_logits, TensorFPC& grad_value) override;

 private:

  ISC observation_len_;
  ISC action_count_;
  ISC dim_;
  ISC layers_;
  /* Weights, owned + persistent (allocated in the ctor from a seed, freed in
     the dtor). Forward uses these; the PPO loop updates them via Backward. */
  TensorFPC W_e_, b_e_;                 //< Embedding (observation_len x dim, dim x 1).
  TensorFPC W_a_, b_a_;                 //< Action head (dim x A, A x 1).
  TensorFPC W_v_, b_v_;                 //< Value head (dim x 1, 1 x 1).
  TransformerBlock* blocks_;                    //< [layers_] transformer blocks.
  IUD seed_;                          //< Weight-init seed (for reproducibility).
  BOL built_;                         //< Weights allocated yet.
  void Build();                        //< Allocate + seed the weights (idempotent).
};

}        //< namespace iGeek
#endif   //< _iGeekPolicyH
