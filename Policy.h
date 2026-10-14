// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_POLICY_DECL
#define IGEEK_POLICY_DECL
#include "Tensor.h"
namespace _ {

/* A transformer "block": MHSA + FFN, each with residual + layernorm.
   POD of TTensor weights (no logic; the forward logic is in PolicyNet.hxx). */
struct TBlock {
  ISC dim_;
  ISC heads_;
  // MHSA projections.
  TTensor W_q, W_k, W_v, W_o;  // dim x dim each.
  TTensor b_q, b_k, b_v, b_o;  // dim x 1 each.
  // FFN.
  TTensor W_f1, W_f2;  // dim x (2dim), (2dim) x dim.
  TTensor b_f1, b_f2;
};

/* A trainable policy: obs -> (action logits, value).
   vectorized-batch: this is the neural head that the fused-kernel fused PPO
   kernel calls — given a batch of obs it produces `logits` (action
   distribution) and `values_pred` (value baseline), and the PPO update
   writes gradients back into `grad_logits`/`grad_value`.

   Kept as an INTERFACE (pure virtuals) so:
     - TTransformer (the MuSE-style net, milestone M3/M4) implements it.
     - A hand-written policy (e.g. the card world's hit-to-17) can implement
       it for baselines, no network needed.
     - The PPO loop depends only on this, not on any concrete net.

   All tensors are batched [B, ...] where B = the Gym's EnvCount.
*/
class TPolicy {
 public:

  virtual ~TPolicy() {}

  /* Action space size (A). For blackjack: 2 (hit/stand). */
  virtual ISC ActionCount() = 0;
  /* Observation feature length (obs_len). */
  virtual ISC ObsLength() = 0;

  /* Forward: obs [B, obs_len] -> logits [B, A] + values [B].
     Deterministic for a given obs (same weights, no sampling here). */
  virtual void Forward(const TTensor& obs, TTensor& logits,
                       TTensor& values) = 0;

  /* Sample an action per row from the (categorical) logits. Writes chosen
     action index into actions_[b] and log-prob into logp_[b]. Uses the
     world RNG (seeded + clamped) for the sampling noise. */
  virtual void SampleActions(const TTensor& logits, ISC* actions,
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
     @param old_logp  Per-row log-prob of the taken action under the old
                      policy [B] (for the PPO ratio; M4 uses it for the
                      surrogate gradient).
     @param values   Current value-head output [B].
     @param returns  Per-row target return (adv + value) [B].
     @param grad_logits  OUT: dL/dlogits [B, A].
     @param grad_value   OUT: dV/dvalue [B]. */
  virtual void Backward(const TTensor& logits, const ISC* actions,
                        const TTensor& advantages, const TTensor& old_logp,
                        const TTensor& values, const TTensor& returns,
                        TTensor& grad_logits, TTensor& grad_value) = 0;

  /* Learn rate (Adam/lr). vectorized-batch: the PPO kernel exposes lr. */
  virtual void SetLearnRate(FPC lr) { lr_ = lr; }
  FPC LearnRate() const { return lr_; }

 protected:

  FPC lr_;
};

/* Concrete MuSE-style transformer policy (milestone M3/M4). Declared here
   so the PPO loop can name it; implemented in PolicyNet.hxx once TTensor
   ops land. Kept out of this header's required surface so the interface
   compiles before the net does. */
class TTransformer : public TPolicy {
 public:

  TTransformer(ISC obs_len, ISC action_count, ISC dim, ISC layers);
  ~TTransformer() override;

  ISC ActionCount() override;
  ISC ObsLength() override;
  void Forward(const TTensor& obs, TTensor& logits, TTensor& values) override;
  void SampleActions(const TTensor& logits, ISC* actions, FPC* logp) override;
  void Backward(const TTensor& logits, const ISC* actions,
                const TTensor& advantages, const TTensor& old_logp,
                const TTensor& values, const TTensor& returns,
                TTensor& grad_logits, TTensor& grad_value) override;

 private:

  ISC obs_len_;
  ISC action_count_;
  ISC dim_;
  ISC layers_;
  /* Weights, owned + persistent (allocated in the ctor from a seed, freed in
     the dtor). Forward uses these; the PPO loop updates them via Backward. */
  TTensor W_e_, b_e_;                 //< Embedding (obs_len x dim, dim x 1).
  TTensor W_a_, b_a_;                 //< Action head (dim x A, A x 1).
  TTensor W_v_, b_v_;                 //< Value head (dim x 1, 1 x 1).
  TBlock* blocks_;                    //< [layers_] transformer blocks.
  IUD seed_;                          //< Weight-init seed (for reproducibility).
  BOL built_;                         //< Weights allocated yet.
  void Build();                        //< Allocate + seed the weights (idempotent).
};

}  // namespace _
#endif
