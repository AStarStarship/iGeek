// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekTensorH
#define _iGeekTensorH
#include <_Config.h>
namespace iGeek {
using namespace ::_;

/* Minimal row-major float tensor for RL policy math.
NO standard library. This is the primitive the vectorized-batch policy
(TransformerPolicy) and PPO loop (PPO) are built on.

vectorized-batch: the whole RL data model is tensor-valued
(observation_t*, float* actions/rewards, logits, grad_logits, grad_values_pred,
action_mask). We need a flat float buffer with shape + the handful of
elementwise/linear ops a policy forward/backward uses.

IMPORTANT GAP: ASCIICrabs has no tensor/linear-algebra primitive. The
heavy kernels (MatMul, Softmax, LayerNorm, backprop) are logged to
../ASCIICrabs/AGENT_PLAN.md — until then, these ops are implemented
here in iGeek as plain FPC* loops over a flat buffer.

Chimera+: T prefix = POD struct, CamelCase type, lower_snake_case
mutable members, free functions T-prefixed. */
struct TensorFPC {
  FPC* data_;     //< Flat row-major float storage (rows_ * cols_).
  ISC rows_;      //< Number of rows (batch dim).
  ISC cols_;      //< Number of columns (feature dim).
  ISC dtype_;     //< 0 = FPC (default). Reserved for bf16/ISC later.

  /* Element at (r, c). No bounds check (call sites know the shape). */
  FPC& At(ISC r, ISC c) { return data_[r * cols_ + c]; }

  /* Total element count. */
  ISC Count() { return rows_ * cols_; }

  /* True if the tensor owns no storage (nil). */
  BOL IsNil() { return data_ == _NilP; }
};

// Allocation (preallocate-at-init pattern: preallocate at init, free at close)

/* Allocate a rows x columns FPC tensor, zero-initialized. Returns a TensorFPC
whose data_ is heap-owned (free with TensorFPCFree). */
TensorFPC TensorFPCAlloc(ISC rows, ISC columns);

/* Free the tensor storage. Sets *t.data_ = _NilP so a double-free is safe. */
void TensorFPCFree(TensorFPC& t);

// Elementwise / linear ops (CPU FPC, no CUDA)

/* C[i,j] = sum_k A[i,k] * B[k,j].   A: r x k, B: k x c, C: r x c. */
void TensorFPCMatMul(TensorFPC A, TensorFPC B, TensorFPC& C);

/* In-place row softmax: each row of X becomes exp(x)/sum(exp(x)). */
void TensorFPCSoftmaxInPlace(TensorFPC& X);

/* Row-wise layer norm (mean/std per row), in place. */
void TensorFPCLayerNormInPlace(TensorFPC& X);

/* In-place ReLU: x = max(0, x). */
void TensorFPCReluInPlace(TensorFPC& X);

/* C = A + B (elementwise). A and B MUST have the same shape [r, c].
For a bias column [K, 1] added to a [B, K] tensor, use TensorFPCAddBias. */
void TensorFPCAdd(TensorFPC A, TensorFPC B, TensorFPC& C);

/* In-place bias broadcast: X is [B, K], bias is [K, 1]. Adds bias[j] to
every row's column j: X[i, j] += bias[j, 0]. This is the correct way to
add a learned bias (a [K, 1] column) to a batched [B, K] activation —
TensorFPCAdd would overflow because it assumes matching shapes. */
void TensorFPCAddBiasInPlace(TensorFPC& X, TensorFPC bias);

/* C = A * s (scalar broadcast). */
void TensorFPCScale(TensorFPC A, FPC s, TensorFPC& C);

/* Row-wise log-sum-exp (stable), into out (rows x 1). */
void TensorFPCRowLogSumExp(TensorFPC A, TensorFPC& out);

} //< namespace iGeek
#endif
