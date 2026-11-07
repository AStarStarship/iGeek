// Copyright AStarship <https://astarship.net>.
// LinearPolicy.hxx — a small linear policy (observation -> logits + value) that
// implements Policy. Used by the M5 PPO loop to prove the training loop
// works end-to-end on a toy env with a known optimum.
//
// Why a linear policy here: the PPO loop needs dL/dW (weight gradients), and
// for a linear head dL/dW = observation^T @ grad_logits (H = observation, no hidden state).
// This makes the weight update exact + verifiable, isolating the PPO loop
// (GAE + clip + entropy + value-clip) from the transformer's full backprop
// (which is the follow-up: wire TransformerPolicy's block gradients into PPO).
// The gradient interface (Backward -> grad_logits/grad_value) is the SAME one
// the TransformerPolicy uses and the one M4 gradchecked.
#pragma once
#include "Policy.h"
#include "../CrabsTK/Math/Blas.h"
namespace iGeek {
using namespace ::_;

class TLinearPolicy : public Policy {
 public:
  TLinearPolicy(ISC observation_len, ISC action_count, IUD seed)
      : observation_len_(observation_len), action_count_(action_count), seed_(seed),
        W_(_NilP), b_(_NilP), W_v_(_NilP), b_v_(_NilP), built_(false), lr_(0.1f) {}
  ~TLinearPolicy() {
    TensorFPCFree(W_); TensorFPCFree(b_);
    TensorFPCFree(W_v_); TensorFPCFree(b_v_);
  }

  ISC ActionCount() override { return action_count_; }
  ISC ObservationLength() override { return observation_len_; }

  void Build(IUD seed) {
    if (built_) {
      return;
    }
    seed_ = seed;
    TLcgInit();  // (no-op placeholder; we use a local rng below)
    W_ = TensorFPCAlloc(observation_len_, action_count_);
    b_ = TensorFPCAlloc(action_count_, 1);
    W_v_ = TensorFPCAlloc(observation_len_, 1);
    b_v_ = TensorFPCAlloc(1, 1);
    // Small random init.
    IUD s = seed_ ? seed_ : 1;
    for (ISC i = 0; i < observation_len_; ++i) {
      for (ISC j = 0; j < action_count_; ++j) {
        s = s * 6364136223846793005ULL + 1442695040888963407ULL;
        FPC u = FPC(((s >> 11) & 0xFFFFFFFFu) / (double)0xFFFFFFFFu);
        W_.At(i, j) = (u * 2.0f - 1.0f) * 0.5f;
      }
      s = s * 6364136223846793005ULL + 1442695040888963407ULL;
      FPC u2 = FPC(((s >> 11) & 0xFFFFFFFFu) / (double)0xFFFFFFFFu);
      W_v_.At(i, 0) = (u2 * 2.0f - 1.0f) * 0.5f;
    }
    built_ = true;
  }

  void Forward(TensorFPC observation, TensorFPC& logits, TensorFPC& values) override {
    if (!built_) Build(12345);
    TensorFPCMatMul(observation, W_, logits); TensorFPCAddBiasInPlace(logits, b_);
    TensorFPCMatMul(observation, W_v_, values); TensorFPCAddBiasInPlace(values, b_v_);
  }

  void SampleActions(TensorFPC logits, ISC* actions,
                     FPC* logp) override {
    ISC B = logits.rows_;
    // Allocate the LSE output for ALL rows once (TensorFPCRowLogSumExp writes
    // out.At(i, 0) for every row i in [0, B)); sizing it [1,1] overflows.
    TensorFPC lse = TensorFPCAlloc(B, 1);
    TensorFPCRowLogSumExp(logits, lse);
    for (ISC b = 0; b < B; ++b) {
      // Sampled-stochastic (the sample-from-CDF pattern): a seeded LCG
      // draws u in [0,1) and the action is the first index whose CDF exceeds
      // u. Pure argmax here is a degenerate policy (zero exploration -> PPO
      //'s gradient carries no signal and the value head can't bootstrap);
      // the old argmax was a debugging leftover, not the design.
      seed_ = seed_ * 6364136223846793005ULL + 1442695040888963407ULL;
      FPC u = FPC(((seed_ >> 11) & 0xFFFFFFFFu) / (double)0xFFFFFFFFu);
      FPC cumsum = 0.0f;
      ISC sampled = logits.cols_ - 1;
      for (ISC a = 0; a < logits.cols_; ++a) {
        cumsum += FPC(exp(logits.At(b, a) - lse.At(b, 0)));
        if (u < cumsum) { sampled = a; break; }
      }
      actions[b] = sampled;
      logp[b] = logits.At(b, sampled) - lse.At(b, 0);
    }
    TensorFPCFree(lse);
  }

  void Backward(TensorFPC logits, const ISC* actions,
                TensorFPC advantages, TensorFPC old_logp,
                TensorFPC values, TensorFPC returns,
                TensorFPC& grad_logits, TensorFPC& grad_value) override {
    // Same base-gradient math as TransformerPolicy::Backward (verified by M4).
    (void)old_logp;
    ISC B = logits.rows_, A = logits.cols_;
    for (ISC b = 0; b < B; ++b) {
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
      grad_value.At(b, 0) = 2.0f * (values.At(b, 0) - returns.At(b, 0));
    }
  }

  /* SGD step: W -= lr * dL/dW, where dL/dW = observation^T @ grad_logits (the linear
     head's hidden state is the observation). Also steps the value head. This is the
     weight update the PPO loop calls after Backward. */
  void SgdStep(TensorFPC observation, TensorFPC grad_logits,
               TensorFPC grad_value, FPC lr) {
    ISC B = observation.rows_;
    // dW[a-feature][a] = sum_b observation[b, f] * grad_logits[b, a].
    // W_ is [observation_len, action_count]; grad_logits is [B, action_count].
    // dW_ = observation^T (action_count x observation_len ... wait, observation is B x observation_len).
    // dW_[f, a] = sum_b observation[b, f] * grad_logits[b, a].
    TensorFPC dW = TensorFPCAlloc(observation_len_, action_count_);
    TensorFPC db = TensorFPCAlloc(action_count_, 1);
    TensorFPC dWv = TensorFPCAlloc(observation_len_, 1);
    TensorFPC dbv = TensorFPCAlloc(1, 1);
    for (ISC b = 0; b < B; ++b) {
      for (ISC f = 0; f < observation_len_; ++f) {
        FPC ob = observation.At(b, f);
        for (ISC a = 0; a < action_count_; ++a)
          dW.At(f, a) += ob * grad_logits.At(b, a);
        dWv.At(f, 0) += ob * grad_value.At(b, 0);
      }
      for (ISC a = 0; a < action_count_; ++a)
        db.At(a, 0) += grad_logits.At(b, a);
      dbv.At(0, 0) += grad_value.At(b, 0);
    }
    // Average over the batch.
    FPC invB = 1.0f / FPC(B);
    for (ISC f = 0; f < observation_len_; ++f)
      for (ISC a = 0; a < action_count_; ++a)
        W_.At(f, a) -= lr * invB * dW.At(f, a);
    for (ISC a = 0; a < action_count_; ++a) b_.At(a, 0) -= lr * invB * db.At(a, 0);
    for (ISC f = 0; f < observation_len_; ++f) W_v_.At(f, 0) -= lr * invB * dWv.At(f, 0);
    b_v_.At(0, 0) -= lr * invB * dbv.At(0, 0);
    TensorFPCFree(dW); TensorFPCFree(db); TensorFPCFree(dWv); TensorFPCFree(dbv);
  }

  /* Expose weights for diagnostics/tests. */
  TensorFPC& W() { return W_; }
  TensorFPC& Wv() { return W_v_; }

 private:
  ISC observation_len_;
  ISC action_count_;
  IUD seed_;
  TensorFPC W_, b_;
  TensorFPC W_v_, b_v_;
  BOL built_;
  FPC lr_;
  void TLcgInit() {}
};

} //< namespace iGeek
