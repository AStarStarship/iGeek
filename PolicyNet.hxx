// Copyright AStarship <https://astarship.net>.
// PolicyNet.hxx — TTransformer forward (small MuSE-style transformer policy).
//
// Architecture (forward; backward is M4):
//   obs [B, obs_len]
//     -> W_e (obs_len x dim) + b_e        [embed] -> LN
//     -> L blocks:  MHSA(2 heads) -> +res -> LN -> FFN(2x dim, ReLU) -> +res
//     -> action head  W_a (dim x A) + b_a -> logits [B, A]
//     -> value head   W_v (dim x 1) + b_v -> values [B]
//
// Weights are OWNED by the net (allocated in the ctor from a seed, freed in
// the dtor). Forward is a pure function of (obs, weights) -> deterministic,
// and the PPO loop can persist + update the weights via Backward (M4/M5).
// No C++ stdlib. Uses the TTensor ops from Tensor.hxx + <math.h>.
#pragma once
#include "Policy.h"
#include "Tensor.hxx"
#include <math.h>

namespace _ {

namespace {
// Tiny deterministic LCG for weight init (seeded, reproducible).
struct TLcg {
  IUD state_;
  explicit TLcg(IUD seed) : state_(seed ? seed : 0x9E3779B97F4A7C15ULL) {}
  FPC Next() {  // [0, 1)
    state_ = state_ * 6364136223846793005ULL + 1442695040888963407ULL;
    IUD hi = (state_ >> 11) & 0xFFFFFFFFu;
    return (FPC)(hi / (double)0xFFFFFFFFu);
  }
};
void InitRand(TTensor& t, TLcg& rng, FPC scale) {
  for (ISC i = 0; i < t.rows_; ++i)
    for (ISC j = 0; j < t.cols_; ++j)
      t.At(i, j) = (rng.Next() * 2.0f - 1.0f) * scale;
}

/* MHSA forward (single-token MVP): X [B, dim] -> Y [B, dim].
   With one token per row, the attention score is a single value, softmax of
   one element is 1.0, so the attention context == V. We still compute Q/K/V
   and the output projection to keep the structure correct + differentiable. */
void MhsaForward(const TTensor& X, const TBlock& blk, TTensor& Y) {
  ISC B = X.rows_;
  TTensor Q = TTensorAlloc(B, blk.dim_), K = TTensorAlloc(B, blk.dim_),
          V = TTensorAlloc(B, blk.dim_);
  TTensorMatMul(X, blk.W_q, Q); TTensorAddBiasInPlace(Q, blk.b_q);
  TTensorMatMul(X, blk.W_k, K); TTensorAddBiasInPlace(K, blk.b_k);
  TTensorMatMul(X, blk.W_v, V); TTensorAddBiasInPlace(V, blk.b_v);
  TTensorMatMul(V, blk.W_o, Y); TTensorAddBiasInPlace(Y, blk.b_o);
  TTensorFree(Q); TTensorFree(K); TTensorFree(V);
}
}  // namespace

TTransformer::TTransformer(ISC obs_len, ISC action_count, ISC dim, ISC layers)
    : obs_len_(obs_len), action_count_(action_count),
      dim_(dim < 8 ? 8 : dim), layers_(layers < 1 ? 1 : layers),
      seed_(12345), built_(false), blocks_(NILP) {}

TTransformer::~TTransformer() {
  if (!built_) {
    return;
  }
  TTensorFree(W_e_); TTensorFree(b_e_);
  TTensorFree(W_a_); TTensorFree(b_a_);
  TTensorFree(W_v_); TTensorFree(b_v_);
  if (blocks_ != NILP) {
    for (ISC l = 0; l < layers_; ++l) {
      TBlock& b = blocks_[l];
      TTensorFree(b.W_q); TTensorFree(b.W_k); TTensorFree(b.W_v);
      TTensorFree(b.W_o); TTensorFree(b.b_q); TTensorFree(b.b_k);
      TTensorFree(b.b_v); TTensorFree(b.b_o); TTensorFree(b.W_f1);
      TTensorFree(b.W_f2); TTensorFree(b.b_f1); TTensorFree(b.b_f2);
    }
    delete[] blocks_;
    blocks_ = NILP;
  }
  built_ = false;
}

ISC TTransformer::ActionCount() { return action_count_; }
ISC TTransformer::ObsLength() { return obs_len_; }

/* (Re)build the weights from seed_. Idempotent; called from Forward on first
   use and available to re-seed. */
void TTransformer::Build() {
  if (built_) {
    return;
  }
  TLcg rng(seed_);
  FPC s = 1.0f / (FPC)sqrt((FPC)dim_);
  W_e_ = TTensorAlloc(obs_len_, dim_);
  b_e_ = TTensorAlloc(dim_, 1);
  InitRand(W_e_, rng, 0.5f);
  W_a_ = TTensorAlloc(dim_, action_count_);
  b_a_ = TTensorAlloc(action_count_, 1);
  InitRand(W_a_, rng, 0.5f);
  W_v_ = TTensorAlloc(dim_, 1);
  b_v_ = TTensorAlloc(1, 1);
  InitRand(W_v_, rng, 0.5f);
  blocks_ = new TBlock[layers_];
  for (ISC l = 0; l < layers_; ++l) {
    TBlock& b = blocks_[l];
    b.dim_ = dim_; b.heads_ = 2;
    b.W_q = TTensorAlloc(dim_, dim_);
    b.W_k = TTensorAlloc(dim_, dim_);
    b.W_v = TTensorAlloc(dim_, dim_);
    b.W_o = TTensorAlloc(dim_, dim_);
    b.b_q = TTensorAlloc(dim_, 1);
    b.b_k = TTensorAlloc(dim_, 1);
    b.b_v = TTensorAlloc(dim_, 1);
    b.b_o = TTensorAlloc(dim_, 1);
    b.W_f1 = TTensorAlloc(dim_, dim_ * 2);
    b.W_f2 = TTensorAlloc(dim_ * 2, dim_);
    b.b_f1 = TTensorAlloc(dim_ * 2, 1);
    b.b_f2 = TTensorAlloc(dim_, 1);
    InitRand(b.W_q, rng, s); InitRand(b.W_k, rng, s);
    InitRand(b.W_v, rng, s); InitRand(b.W_o, rng, s);
    InitRand(b.W_f1, rng, s); InitRand(b.W_f2, rng, s);
  }
  built_ = true;
}

void TTransformer::Forward(const TTensor& obs, TTensor& logits,
                           TTensor& values) {
  if (!built_) {
    Build();
  }
  ISC B = obs.rows_;
  // Embed.
  TTensor H = TTensorAlloc(B, dim_);
  TTensorMatMul(obs, W_e_, H); TTensorAddBiasInPlace(H, b_e_);
  TTensorLayerNormInPlace(H);
  // Blocks.
  for (ISC l = 0; l < layers_; ++l) {
    const TBlock& blk = blocks_[l];
    TTensor Res = TTensorAlloc(B, dim_);
    for (ISC i = 0; i < B; ++i)
      for (ISC j = 0; j < dim_; ++j) Res.At(i, j) = H.At(i, j);
    MhsaForward(H, blk, H);
    TTensorAdd(H, Res, H);
    TTensorLayerNormInPlace(H);
    TTensor F1 = TTensorAlloc(B, dim_ * 2);
    TTensorMatMul(H, blk.W_f1, F1); TTensorAddBiasInPlace(F1, blk.b_f1);
    TTensorReluInPlace(F1);
    TTensor F2 = TTensorAlloc(B, dim_);
    TTensorMatMul(F1, blk.W_f2, F2); TTensorAddBiasInPlace(F2, blk.b_f2);
    TTensorAdd(F2, Res, H);
    TTensorFree(Res); TTensorFree(F1); TTensorFree(F2);
  }
  // Action head.
  TTensorMatMul(H, W_a_, logits); TTensorAddBiasInPlace(logits, b_a_);
  // Value head.
  TTensorMatMul(H, W_v_, values); TTensorAddBiasInPlace(values, b_v_);
  TTensorFree(H);
}

void TTransformer::SampleActions(const TTensor& logits, ISC* actions,
                                 FPC* logp) {
  ISC B = logits.rows_;
  // Allocate LSE for ALL rows once (TTensorRowLogSumExp writes out.At(i,0) for
  // every row i in [0,B)); sizing it [1,1] overflows. Same fix as
  // TLinearPolicy::SampleActions.
  TTensor lse = TTensorAlloc(B, 1);
  TTensorRowLogSumExp(logits, lse);
  for (ISC b = 0; b < B; ++b) {
    ISC best = 0;
    FPC bestv = logits.AtC(b, 0);
    for (ISC a = 1; a < logits.cols_; ++a) {
      if (logits.AtC(b, a) > bestv) { bestv = logits.AtC(b, a); best = a; }
    }
    actions[b] = best;
    logp[b] = logits.AtC(b, best) - lse.At(b, 0);
  }
  TTensorFree(lse);
}

/* Backward (M4): compute the PPO policy-surrogate + value-loss gradients
   w.r.t. the action logits and the value head. These are the exact quantities
   the fused PPO kernel emits (grad_logits, grad_value).

   Per row b:
     p[b,a]   = softmax(logits[b])[a]
     logp[b]  = logits[b, a_b] - logsumexp(logits[b])
     L_b      = -logp[b] * adv[b]           (policy surrogate)
     dL/dlogits[b,a] = -adv[b] * (1(a == a_b) - p[b,a])
     V_b      = (value[b] - ret[b])^2       (value loss)
     dV/dvalue[b]    = 2 * (value[b] - ret[b])
   (The PPO clip-ratio and vf-clip are applied in the TPPO loop on top of these
   base gradients; here we return the unclipped per-row gradients.) */
void TTransformer::Backward(const TTensor& logits, const ISC* actions,
                            const TTensor& advantages, const TTensor& old_logp,
                            const TTensor& values, const TTensor& returns,
                            TTensor& grad_logits, TTensor& grad_value) {
  (void)old_logp;  // Used by the TPPO clip-ratio, not the base grad.
  ISC B = logits.rows_, A = logits.cols_;
  for (ISC b = 0; b < B; ++b) {
    // softmax(logits[b])
    TTensor row = TTensorAlloc(1, A);
    for (ISC a = 0; a < A; ++a) row.At(0, a) = logits.AtC(b, a);
    TTensorSoftmaxInPlace(row);
    ISC a_b = (actions != NILP) ? actions[b] : 0;
    FPC adv = advantages.AtC(b, 0);
    for (ISC a = 0; a < A; ++a) {
      FPC onehot = (a == a_b) ? 1.0f : 0.0f;
      grad_logits.At(b, a) = -adv * (onehot - row.At(0, a));
    }
    TTensorFree(row);
    // value grad
    grad_value.At(b, 0) = 2.0f * (values.AtC(b, 0) - returns.AtC(b, 0));
  }
}

/* Policy surrogate loss (scalar): L = sum_b -logp[b] * adv[b], where
   logp[b] = logits[b, a_b] - logsumexp(logits[b]). This is the scalar the M4
   finite-difference gradcheck differentiates w.r.t. the logits, and it is the
   same quantity whose gradient Backward returns (grad_logits). */
FPC TPolicyLoss(const TTensor& logits, const ISC* actions,
                const TTensor& advantages) {
  ISC B = logits.rows_, A = logits.cols_;
  FPC total = 0.0f;
  for (ISC b = 0; b < B; ++b) {
    // logsumexp of row b (stable).
    FPC maxv = logits.AtC(b, 0);
    for (ISC a = 1; a < A; ++a)
      if (logits.AtC(b, a) > maxv) maxv = logits.AtC(b, a);
    FPC sum = 0.0f;
    for (ISC a = 0; a < A; ++a) sum += (FPC)exp(logits.AtC(b, a) - maxv);
    FPC lse = maxv + (FPC)log(sum);
    ISC a_b = (actions != NILP) ? actions[b] : 0;
    FPC logp = logits.AtC(b, a_b) - lse;
    total += -logp * advantages.AtC(b, 0);
  }
  return total;
}

/* Value loss (scalar): V = sum_b (value[b] - ret[b])^2. */
FPC TValueLoss(const TTensor& values, const TTensor& returns) {
  FPC total = 0.0f;
  for (ISC b = 0; b < values.rows_; ++b) {
    FPC d = values.AtC(b, 0) - returns.AtC(b, 0);
    total += d * d;
  }
  return total;
}

}  // namespace _
