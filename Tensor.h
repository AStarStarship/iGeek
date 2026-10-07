// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_TENSOR_DECL
#define IGEEK_TENSOR_DECL
// NOTE: do NOT include _Package.hxx here — that pulls in the whole ASCIICrabs
// implementation and double-includes it in the single-TU world build. The POD
// types (FPC/ISC/BOL/NILP) come from the config headers already on the build.
namespace _ {

/* Minimal row-major float tensor for RL policy math.
   NO standard library. This is the primitive the Puffer-informed policy
   (TTransformer) and PPO loop (TPPO) are built on.

   Puffer-informed: PufferLib's whole data model is tensor-valued
   (obs_t*, float* actions/rewards, logits, grad_logits, grad_values_pred,
   action_mask). We need a flat float buffer with shape + the handful of
   elementwise/linear ops a policy forward/backward uses.

   IMPORTANT GAP: ASCIICrabs has no tensor/linear-algebra primitive. The
   heavy kernels (MatMul, Softmax, LayerNorm, backprop) are logged to
   ../ASCIICrabs/AGENT_PLAN.md — until then, these ops are implemented
   here in iGeek as plain FPC* loops over a flat buffer.

   Chimera+: T prefix = POD struct, CamelCase type, lower_snake_case
   mutable members, free functions T-prefixed.
*/
struct TTensor {
  FPC* data_;     //< Flat row-major float storage (rows_ * cols_).
  ISC rows_;      //< Number of rows (batch dim).
  ISC cols_;      //< Number of cols (feature dim).
  ISC dtype_;     //< 0 = FPC (default). Reserved for bf16/ISC later.

  /* Element at (r, c). No bounds check (call sites know the shape). */
  FPC& At(ISC r, ISC c) { return data_[r * cols_ + c]; }
  const FPC& AtC(ISC r, ISC c) const { return data_[r * cols_ + c]; }

  /* Total element count. */
  ISC Count() const { return rows_ * cols_; }

  /* True if the tensor owns no storage (nil). */
  BOL IsNil() const { return data_ == NILP; }
};

/* --- Allocation (Puffer pattern: preallocate at init, free at close) --- */

/* Allocate a rows x cols FPC tensor, zero-initialized. Returns a TTensor
   whose data_ is heap-owned (free with TTensorFree). */
TTensor TTensorAlloc(ISC rows, ISC cols);

/* Free the tensor storage. Sets *t.data_ = NILP so a double-free is safe. */
void TTensorFree(TTensor& t);

/* --- Elementwise / linear ops (CPU FPC, no CUDA) ---------------------- */

/* C[i,j] = sum_k A[i,k] * B[k,j].   A: r x k, B: k x c, C: r x c. */
void TTensorMatMul(const TTensor& A, const TTensor& B, TTensor& C);

/* In-place row softmax: each row of X becomes exp(x)/sum(exp(x)). */
void TTensorSoftmaxInPlace(TTensor& X);

/* Row-wise layer norm (mean/std per row), in place. */
void TTensorLayerNormInPlace(TTensor& X);

/* In-place ReLU: x = max(0, x). */
void TTensorReluInPlace(TTensor& X);

/* C = A + B (elementwise). */
void TTensorAdd(const TTensor& A, const TTensor& B, TTensor& C);

/* C = A * s (scalar broadcast). */
void TTensorScale(const TTensor& A, FPC s, TTensor& C);

/* Row-wise log-sum-exp (stable), into out (rows x 1). */
void TTensorRowLogSumExp(const TTensor& A, TTensor& out);

}  // namespace _
#endif
