// Copyright AStarship <https://astarship.net>.
// PolicyNet.hxx — TransformerPolicy forward (compact transformer policy).
//
// Architecture (forward; backward is M4):
//   observation [B, observation_len]
//     -> W_e (observation_len x dim) + b_e        [embed] -> LN
//     -> L blocks:  MHSA(2 heads) -> +res -> LN -> FFN(2x dim, ReLU) -> +res
//     -> action head  W_a (dim x A) + b_a -> logits [B, A]
//     -> value head   W_v (dim x 1) + b_v -> values [B]
//
// Weights are OWNED by the net (allocated in the ctor from a seed, freed in
// the dtor). Forward is a pure function of (observation, weights) -> deterministic,
// and the PPO loop can persist + update the weights via Backward (M4/M5).
// No C++ stdlib. Uses the TensorFPC ops from Tensor.hxx + <math.h>.
#pragma once
#include "Policy.h"
#include "../ASCIICrabs/Math.hxx"
#include "Tensor.hxx"
#include "../CrabsTK/Math/Blas.h"
namespace iGeek {
using namespace ::_;

namespace {
// Tiny deterministic LCG for weight init (seeded, reproducible).
struct TLcg {
  IUD state_;
  explicit TLcg(IUD seed) : state_(seed ? seed : 0x9E3779B97F4A7C15ULL) {}
  FPC Next() {  // [0, 1)
    state_ = state_ * 6364136223846793005ULL + 1442695040888963407ULL;
    IUD hi = (state_ >> 11) & 0xFFFFFFFFu;
    return FPC(hi / (double)0xFFFFFFFFu);
  }
};
void InitRand(TensorFPC& t, TLcg& rng, FPC scale) {
  for (ISC i = 0; i < t.rows_; ++i)
    for (ISC j = 0; j < t.cols_; ++j)
      t.At(i, j) = (rng.Next() * 2.0f - 1.0f) * scale;
}

/* MHSA forward (single-token MVP): X [B, dim] -> Y [B, dim].
   With one token per row, the attention score is a single value, softmax of
   one element is 1.0, so the attention context == V. We still compute Q/K/V
   and the output projection to keep the structure correct + differentiable. */
void MhsaForward(TensorFPC X, TransformerBlock& blk, TensorFPC& Y) {
  ISC B = X.rows_;
  TensorFPC Q = TensorFPCAlloc(B, blk.dim_), K = TensorFPCAlloc(B, blk.dim_),
          V = TensorFPCAlloc(B, blk.dim_);
  TensorFPCMatMul(X, blk.W_q, Q); TensorFPCAddBiasInPlace(Q, blk.b_q);
  TensorFPCMatMul(X, blk.W_k, K); TensorFPCAddBiasInPlace(K, blk.b_k);
  TensorFPCMatMul(X, blk.W_v, V); TensorFPCAddBiasInPlace(V, blk.b_v);
  TensorFPCMatMul(V, blk.W_o, Y); TensorFPCAddBiasInPlace(Y, blk.b_o);
  TensorFPCFree(Q); TensorFPCFree(K); TensorFPCFree(V);
}
}  //< namespace

TransformerPolicy::TransformerPolicy(ISC observation_len, ISC action_count, ISC dim, ISC layers)
    : observation_len_(observation_len), action_count_(action_count),
      dim_(dim < 8 ? 8 : dim), layers_(layers < 1 ? 1 : layers),
      seed_(12345), built_(false), blocks_(_NilP) {}

TransformerPolicy::~TransformerPolicy() {
  if (!built_) {
    return;
  }
  TensorFPCFree(W_e_); TensorFPCFree(b_e_);
  TensorFPCFree(W_a_); TensorFPCFree(b_a_);
  TensorFPCFree(W_v_); TensorFPCFree(b_v_);
  if (blocks_ != _NilP) {
    for (ISC l = 0; l < layers_; ++l) {
      TransformerBlock& b = blocks_[l];
      TensorFPCFree(b.W_q); TensorFPCFree(b.W_k); TensorFPCFree(b.W_v);
      TensorFPCFree(b.W_o); TensorFPCFree(b.b_q); TensorFPCFree(b.b_k);
      TensorFPCFree(b.b_v); TensorFPCFree(b.b_o); TensorFPCFree(b.W_f1);
      TensorFPCFree(b.W_f2); TensorFPCFree(b.b_f1); TensorFPCFree(b.b_f2);
    }
    delete[] blocks_;
    blocks_ = _NilP;
  }
  built_ = false;
}

ISC TransformerPolicy::ActionCount() { return action_count_; }
ISC TransformerPolicy::ObservationLength() { return observation_len_; }

/* (Re)build the weights from seed_. Idempotent; called from Forward on first
   use and available to re-seed. */
void TransformerPolicy::Build() {
  if (built_) {
    return;
  }
  TLcg rng(seed_);
  FPC s = 1.0f / FPC(FPC(SquareRoot(dim_)));
  W_e_ = TensorFPCAlloc(observation_len_, dim_);
  b_e_ = TensorFPCAlloc(dim_, 1);
  InitRand(W_e_, rng, 0.5f);
  W_a_ = TensorFPCAlloc(dim_, action_count_);
  b_a_ = TensorFPCAlloc(action_count_, 1);
  InitRand(W_a_, rng, 0.5f);
  W_v_ = TensorFPCAlloc(dim_, 1);
  b_v_ = TensorFPCAlloc(1, 1);
  InitRand(W_v_, rng, 0.5f);
  blocks_ = new TransformerBlock[layers_];
  for (ISC l = 0; l < layers_; ++l) {
    TransformerBlock& b = blocks_[l];
    b.dim_ = dim_; b.heads_ = 2;
    b.W_q = TensorFPCAlloc(dim_, dim_);
    b.W_k = TensorFPCAlloc(dim_, dim_);
    b.W_v = TensorFPCAlloc(dim_, dim_);
    b.W_o = TensorFPCAlloc(dim_, dim_);
    b.b_q = TensorFPCAlloc(dim_, 1);
    b.b_k = TensorFPCAlloc(dim_, 1);
    b.b_v = TensorFPCAlloc(dim_, 1);
    b.b_o = TensorFPCAlloc(dim_, 1);
    b.W_f1 = TensorFPCAlloc(dim_, dim_ * 2);
    b.W_f2 = TensorFPCAlloc(dim_ * 2, dim_);
    b.b_f1 = TensorFPCAlloc(dim_ * 2, 1);
    b.b_f2 = TensorFPCAlloc(dim_, 1);
    InitRand(b.W_q, rng, s); InitRand(b.W_k, rng, s);
    InitRand(b.W_v, rng, s); InitRand(b.W_o, rng, s);
    InitRand(b.W_f1, rng, s); InitRand(b.W_f2, rng, s);
  }
  built_ = true;
}

void TransformerPolicy::Forward(TensorFPC observation, TensorFPC& logits,
                           TensorFPC& values) {
  if (!built_) {
    Build();
  }
  ISC B = observation.rows_;
  // Embed.
  TensorFPC H = TensorFPCAlloc(B, dim_);
  TensorFPCMatMul(observation, W_e_, H); TensorFPCAddBiasInPlace(H, b_e_);
  TensorFPCLayerNormInPlace(H);
  // Blocks.
  for (ISC l = 0; l < layers_; ++l) {
    TransformerBlock& blk = blocks_[l];
    TensorFPC Res = TensorFPCAlloc(B, dim_);
    for (ISC i = 0; i < B; ++i)
      for (ISC j = 0; j < dim_; ++j) Res.At(i, j) = H.At(i, j);
    MhsaForward(H, blk, H);
    TensorFPCAdd(H, Res, H);
    TensorFPCLayerNormInPlace(H);
    TensorFPC F1 = TensorFPCAlloc(B, dim_ * 2);
    TensorFPCMatMul(H, blk.W_f1, F1); TensorFPCAddBiasInPlace(F1, blk.b_f1);
    TensorFPCReluInPlace(F1);
    TensorFPC F2 = TensorFPCAlloc(B, dim_);
    TensorFPCMatMul(F1, blk.W_f2, F2); TensorFPCAddBiasInPlace(F2, blk.b_f2);
    TensorFPCAdd(F2, Res, H);
    TensorFPCFree(Res); TensorFPCFree(F1); TensorFPCFree(F2);
  }
  // Action head.
  TensorFPCMatMul(H, W_a_, logits); TensorFPCAddBiasInPlace(logits, b_a_);
  // Value head.
  TensorFPCMatMul(H, W_v_, values); TensorFPCAddBiasInPlace(values, b_v_);
  TensorFPCFree(H);
}

void TransformerPolicy::SampleActions(TensorFPC logits, ISC* actions,
                                 FPC* logp) {
  ISC B = logits.rows_;
  // Allocate LSE for ALL rows once (TensorFPCRowLogSumExp writes out.At(i,0) for
  // every row i in [0,B)); sizing it [1,1] overflows. Same fix as
  // TLinearPolicy::SampleActions.
  TensorFPC lse = TensorFPCAlloc(B, 1);
  TensorFPCRowLogSumExp(logits, lse);
  for (ISC b = 0; b < B; ++b) {
    ISC best = 0;
    FPC bestv = logits.At(b, 0);
    for (ISC a = 1; a < logits.cols_; ++a) {
      if (logits.At(b, a) > bestv) { bestv = logits.At(b, a); best = a; }
    }
    actions[b] = best;
    logp[b] = logits.At(b, best) - lse.At(b, 0);
  }
  TensorFPCFree(lse);
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
   (The PPO clip-ratio and vf-clip are applied in the PPO loop on top of these
   base gradients; here we return the unclipped per-row gradients.) */
void TransformerPolicy::Backward(TensorFPC logits, const ISC* actions,
                            TensorFPC advantages, TensorFPC old_logp,
                            TensorFPC values, TensorFPC returns,
                            TensorFPC& grad_logits, TensorFPC& grad_value) {
  (void)old_logp;  // Used by the PPO clip-ratio, not the base grad.
  ISC B = logits.rows_, A = logits.cols_;
  for (ISC b = 0; b < B; ++b) {
    // softmax(logits[b])
    TensorFPC row = TensorFPCAlloc(1, A);
    for (ISC a = 0; a < A; ++a) row.At(0, a) = logits.At(b, a);
    TensorFPCSoftmaxInPlace(row);
    ISC a_b = (actions != _NilP) ? actions[b] : 0;
    FPC adv = advantages.At(b, 0);
    for (ISC a = 0; a < A; ++a) {
      FPC onehot = (a == a_b) ? 1.0f : 0.0f;
      grad_logits.At(b, a) = -adv * (onehot - row.At(0, a));
    }
    TensorFPCFree(row);
    // value grad
    grad_value.At(b, 0) = 2.0f * (values.At(b, 0) - returns.At(b, 0));
  }
}

/* Policy surrogate loss (scalar): L = sum_b -logp[b] * adv[b], where
   logp[b] = logits[b, a_b] - logsumexp(logits[b]). This is the scalar the M4
   finite-difference gradcheck differentiates w.r.t. the logits, and it is the
   same quantity whose gradient Backward returns (grad_logits). */
FPC TPolicyLoss(TensorFPC logits, const ISC* actions,
                TensorFPC advantages) {
  ISC B = logits.rows_, A = logits.cols_;
  FPC total = 0.0f;
  for (ISC b = 0; b < B; ++b) {
    // logsumexp of row b (stable).
    FPC maxv = logits.At(b, 0);
    for (ISC a = 1; a < A; ++a)
      if (logits.At(b, a) > maxv) maxv = logits.At(b, a);
    FPC sum = 0.0f;
    for (ISC a = 0; a < A; ++a) sum += FPC(exp(logits.At(b, a) - maxv));
    FPC lse = maxv + FPC(log(sum));
    ISC a_b = (actions != _NilP) ? actions[b] : 0;
    FPC logp = logits.At(b, a_b) - lse;
    total += -logp * advantages.At(b, 0);
  }
  return total;
}

/* Value loss (scalar): V = sum_b (value[b] - ret[b])^2. */
FPC TValueLoss(TensorFPC values, TensorFPC returns) {
  FPC total = 0.0f;
  for (ISC b = 0; b < values.rows_; ++b) {
    FPC d = values.At(b, 0) - returns.At(b, 0);
    total += d * d;
  }
  return total;
}

} //< namespace iGeek
