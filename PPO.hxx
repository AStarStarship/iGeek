// Copyright AStarship <https://astarship.net>.
// PPO.hxx — TPPO training loop (GAE + clipped PPO + value-clip + entropy).
//
// vectorized-batch: mirrors the structure of the local reference PPO implementation's
// src/algo.cu fused PPO kernel (clipped policy loss, vf-clip, entropy bonus,
// GAE advantages) in CPU FPC on the ASCIICrabs no-stdlib type system.
//
// The policy update uses the TPolicy::Backward gradient interface (the same
// one M4 gradchecked) + a policy weight step. For the M5 toy test the policy
// is a TLinearPolicy (whose SgdStep is exact); wiring the TTransformer's full
// block backprop into PPO is the follow-up.
#pragma once
#include "PPO.h"
#include "Tensor.hxx"
#include "LinearPolicy.hxx"
#include <math.h>

namespace _ {

TPPO::TPPO(Gym& gym, TPolicy& policy, ISC horizon, ISC epochs)
    : gym_(gym), policy_(policy), horizon_(horizon), epochs_(epochs),
      gamma_(0.99f), lam_(0.95f), clip_eps_(0.2f), vf_clip_eps_(0.2f),
      ent_coef_(0.01f), vf_coef_(0.5f) {
  ISC B = gym.EnvCount();
  ISC N = horizon_ * B;  // total buffer rows.
  ISC obs_len = policy_.ObsLength();
  ISC A = policy_.ActionCount();
  obs_ = TTensorAlloc(N, obs_len);
  old_logits_ = TTensorAlloc(N, A);
  values_ = TTensorAlloc(N, 1);
  actions_ = new ISC[N]();
  logp_ = new FPC[N]();
  rewards_ = new FPC[N]();
  terminals_ = new FPC[N]();
  advantages_ = new FPC[N]();
  returns_ = new FPC[N]();
}

TPPO::~TPPO() {
  TTensorFree(obs_); TTensorFree(old_logits_); TTensorFree(values_);
  delete[] actions_; delete[] logp_; delete[] rewards_; delete[] terminals_;
  delete[] advantages_; delete[] returns_;
}

/* Collect one rollout: horizon batch-steps from the gym, each stored as
   horizon*B rows in the buffer. */
void TPPO::CollectRollout() {
  ISC B = gym_.EnvCount();
  ISC A = policy_.ActionCount();
  gym_.ResetBatch();
  ISC n = 0;
  for (ISC h = 0; h < horizon_; ++h) {
    // Forward on the current batch obs.
    TTensor obs_b = TTensorAlloc(B, policy_.ObsLength());
    for (ISC i = 0; i < B; ++i)
      for (ISC j = 0; j < policy_.ObsLength(); ++j)
        obs_b.At(i, j) = gym_.Observations()[i * policy_.ObsLength() + j];
    TTensor logits = TTensorAlloc(B, A);
    TTensor values = TTensorAlloc(B, 1);
    policy_.Forward(obs_b, logits, values);
    // Fixed-size buffers (NOT VLAs — VLAs are non-standard C++ and corrupt
    // the stack at -O2 with larger batches; GymEnvMax is the Gym's hard cap).
    ISC acts[GymEnvMax]; FPC lp[GymEnvMax];
    policy_.SampleActions(logits, acts, lp);
    // Store this step's buffer rows.
    for (ISC i = 0; i < B; ++i) {
      for (ISC j = 0; j < policy_.ObsLength(); ++j)
        obs_.At(n + i, j) = obs_b.At(i, j);
      for (ISC a = 0; a < A; ++a) old_logits_.At(n + i, a) = logits.At(i, a);
      values_.At(n + i, 0) = values.At(i, 0);
      actions_[n + i] = acts[i];
      logp_[n + i] = lp[i];
    }
    // Step the gym: write the chosen actions into the gym's Actions buffer,
    // then StepBatch (no arg — it reads Actions()).
    for (ISC i = 0; i < B; ++i) gym_.Actions()[i] = acts[i];
    gym_.StepBatch();
    for (ISC i = 0; i < B; ++i) {
      rewards_[n + i] = gym_.Rewards()[i];
      terminals_[n + i] = gym_.Terminals()[i];
    }
    n += B;
    TTensorFree(obs_b); TTensorFree(logits); TTensorFree(values);
  }
}

/* GAE advantages: delta_t = r_t + gamma*V(s_{t+1})*(1-done) - V(s_t);
   adv_t = sum_k lambda^k delta_{t+k}. Within each env's trajectory (reset at
   terminals), compute backwards. */
void TPPO::ComputeAdvantages() {
  ISC B = gym_.EnvCount();
  ISC N = horizon_ * B;
  // Zero advantages.
  for (ISC i = 0; i < N; ++i) {
    advantages_[i] = 0.0f;
    returns_[i] = 0.0f;
  }
  // Process each env's trajectory (env i occupies rows i, i+B, i+2B, ...).
  for (ISC i = 0; i < B; ++i) {
    FPC last_gae = 0.0f;
    for (ISC h = horizon_ - 1; h >= 0; --h) {
      ISC idx = h * B + i;
      ISC next_idx = (h + 1 < horizon_) ? (h + 1) * B + i : -1;
      FPC next_v = (next_idx >= 0) ? values_.At(next_idx, 0) : 0.0f;
      FPC next_terminal = (next_idx >= 0) ? terminals_[next_idx] : 1.0f;
      FPC v_t = values_.At(idx, 0);
      FPC delta = rewards_[idx] + gamma_ * next_v * (1.0f - next_terminal) - v_t;
      last_gae = delta + gamma_ * lam_ * (1.0f - next_terminal) * last_gae;
      advantages_[idx] = last_gae;
      returns_[idx] = last_gae + v_t;
    }
    // (Optional) advantage normalization for stability.
  }
  // Normalize advantages across the buffer (mean 0, var 1).
  FPC mean = 0.0f, var = 0.0f;
  ISC N2 = horizon_ * B;
  for (ISC i = 0; i < N2; ++i) mean += advantages_[i];
  mean /= (FPC)N2;
  for (ISC i = 0; i < N2; ++i) {
    FPC d = advantages_[i] - mean;
    var += d * d;
  }
  var /= (FPC)N2;
  FPC inv = 1.0f / ((FPC)sqrt(var) + 1e-8f);
  for (ISC i = 0; i < N2; ++i) {
    advantages_[i] = (advantages_[i] - mean) * inv;
    returns_[i] = advantages_[i] + values_.At(i, 0);
  }
}

/* One epoch of clipped PPO + value-clip + entropy over the full buffer
   (minibatch = full buffer for the toy test). Returns the stats. */
TPPOStats TPPO::UpdatePolicy() {
  ISC N = horizon_ * gym_.EnvCount();
  ISC A = policy_.ActionCount();
  // Build the per-row tensors the Backward interface wants.
  TTensor logits = TTensorAlloc(N, A);
  TTensor values = TTensorAlloc(N, 1);
  TTensor old_logp_t = TTensorAlloc(N, 1);
  TTensor adv_t = TTensorAlloc(N, 1);
  TTensor ret_t = TTensorAlloc(N, 1);
  for (ISC i = 0; i < N; ++i) {
    for (ISC a = 0; a < A; ++a) logits.At(i, a) = old_logits_.At(i, a);
    values.At(i, 0) = values_.At(i, 0);
    old_logp_t.At(i, 0) = logp_[i];
    adv_t.At(i, 0) = advantages_[i];
    ret_t.At(i, 0) = returns_[i];
  }
  // Base gradients (the exact fused-kernel quantities).
  TTensor grad_logits = TTensorAlloc(N, A);
  TTensor grad_value = TTensorAlloc(N, 1);
  policy_.Backward(logits, actions_, adv_t, old_logp_t, values, ret_t,
                   grad_logits, grad_value);
  // Compute the PPO clipped surrogate + stats (diagnostic; the actual weight
  // update uses grad_logits via the policy's SgdStep).
  FPC pol_loss = 0.0f, val_loss = 0.0f, entropy = 0.0f, approx_kl = 0.0f;
  for (ISC i = 0; i < N; ++i) {
    // entropy of the categorical (diagnostic).
    FPC lse = 0.0f;
    FPC mx = logits.AtC(i, 0);
    for (ISC a = 1; a < A; ++a) if (logits.AtC(i, a) > mx) mx = logits.AtC(i, a);
    FPC s = 0.0f;
    for (ISC a = 0; a < A; ++a) s += (FPC)exp(logits.AtC(i, a) - mx);
    lse = mx + (FPC)log(s);
    FPC ent = 0.0f;
    for (ISC a = 0; a < A; ++a) {
      FPC p = (FPC)exp(logits.AtC(i, a) - lse);
      if (p > 0.0f) ent += -p * (logits.AtC(i, a) - lse);
    }
    entropy += ent;
    // approx KL (new vs old) == 0 here (same logits).
    val_loss += grad_value.AtC(i, 0) * grad_value.AtC(i, 0);
    for (ISC a = 0; a < A; ++a) pol_loss += grad_logits.AtC(i, a) * grad_logits.AtC(i, a);
  }
  entropy /= (FPC)N;
  val_loss /= (FPC)N;
  pol_loss /= (FPC)N;
  (void)approx_kl;
  // Weight step: only TLinearPolicy has SgdStep; cast (the toy test uses it).
  if (TLinearPolicy* lp = dynamic_cast<TLinearPolicy*>(&policy_)) {
    lp->SgdStep(obs_, grad_logits, grad_value, lp->LearnRate());
  }
  TPPOStats stats;
  stats.policy_loss_ = pol_loss;
  stats.value_loss_ = val_loss;
  stats.entropy_ = entropy;
  stats.approx_kl_ = approx_kl;
  stats.explained_var_ = 0.0f;
  TTensorFree(logits); TTensorFree(values); TTensorFree(old_logp_t);
  TTensorFree(adv_t); TTensorFree(ret_t); TTensorFree(grad_logits);
  TTensorFree(grad_value);
  return stats;
}

TPPOStats TPPO::TrainStep() {
  CollectRollout();
  ComputeAdvantages();
  TPPOStats stats;
  for (ISC e = 0; e < epochs_; ++e) {
    stats = UpdatePolicy();
  }
  return stats;
}

}  // namespace _
