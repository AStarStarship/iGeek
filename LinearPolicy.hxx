// Copyright AStarship <https://astarship.net>.
// LinearPolicy.hxx — a small linear policy (obs -> logits + value) that
// implements TPolicy. Used by the M5 PPO loop to prove the training loop
// works end-to-end on a toy env with a known optimum.
//
// Why a linear policy here: the PPO loop needs dL/dW (weight gradients), and
// for a linear head dL/dW = obs^T @ grad_logits (H = obs, no hidden state).
// This makes the weight update exact + verifiable, isolating the PPO loop
// (GAE + clip + entropy + value-clip) from the transformer's full backprop
// (which is the follow-up: wire TTransformer's block gradients into PPO).
// The gradient interface (Backward -> grad_logits/grad_value) is the SAME one
// the TTransformer uses and the one M4 gradchecked.
#pragma once
#include "Policy.h"
#include "PolicyNet.hxx"  // for TPolicyLoss/TValueLoss + the Backward math shape
#include <math.h>

namespace _ {

class TLinearPolicy : public TPolicy {
 public:
  TLinearPolicy(ISC obs_len, ISC action_count, IUD seed)
      : obs_len_(obs_len), action_count_(action_count), seed_(seed),
        W_(NILP), b_(NILP), W_v_(NILP), b_v_(NILP), built_(false), lr_(0.1f) {}
  ~TLinearPolicy() {
    TTensorFree(W_); TTensorFree(b_);
    TTensorFree(W_v_); TTensorFree(b_v_);
  }

  ISC ActionCount() override { return action_count_; }
  ISC ObsLength() override { return obs_len_; }

  void Build(IUD seed) {
    if (built_) {
      return;
    }
    seed_ = seed;
    TLcgInit();  // (no-op placeholder; we use a local rng below)
    W_ = TTensorAlloc(obs_len_, action_count_);
    b_ = TTensorAlloc(action_count_, 1);
    W_v_ = TTensorAlloc(obs_len_, 1);
    b_v_ = TTensorAlloc(1, 1);
    // Small random init.
    IUD s = seed_ ? seed_ : 1;
    for (ISC i = 0; i < obs_len_; ++i) {
      for (ISC j = 0; j < action_count_; ++j) {
        s = s * 6364136223846793005ULL + 1442695040888963407ULL;
        FPC u = (FPC)(((s >> 11) & 0xFFFFFFFFu) / (double)0xFFFFFFFFu);
        W_.At(i, j) = (u * 2.0f - 1.0f) * 0.5f;
      }
      s = s * 6364136223846793005ULL + 1442695040888963407ULL;
      FPC u2 = (FPC)(((s >> 11) & 0xFFFFFFFFu) / (double)0xFFFFFFFFu);
      W_v_.At(i, 0) = (u2 * 2.0f - 1.0f) * 0.5f;
    }
    built_ = true;
  }

  void Forward(const TTensor& obs, TTensor& logits, TTensor& values) override {
    if (!built_) Build(12345);
    TTensorMatMul(obs, W_, logits); TTensorAddBiasInPlace(logits, b_);
    TTensorMatMul(obs, W_v_, values); TTensorAddBiasInPlace(values, b_v_);
  }

  void SampleActions(const TTensor& logits, ISC* actions,
                     FPC* logp) override {
    ISC B = logits.rows_;
    // Allocate the LSE output for ALL rows once (TTensorRowLogSumExp writes
    // out.At(i, 0) for every row i in [0, B)); sizing it [1,1] overflows.
    TTensor lse = TTensorAlloc(B, 1);
    TTensorRowLogSumExp(logits, lse);
    for (ISC b = 0; b < B; ++b) {
      // Sampled-stochastic (the sample-from-CDF pattern): a seeded LCG
      // draws u in [0,1) and the action is the first index whose CDF exceeds
      // u. Pure argmax here is a degenerate policy (zero exploration -> PPO
      //'s gradient carries no signal and the value head can't bootstrap);
      // the old argmax was a debugging leftover, not the design.
      seed_ = seed_ * 6364136223846793005ULL + 1442695040888963407ULL;
      FPC u = (FPC)(((seed_ >> 11) & 0xFFFFFFFFu) / (double)0xFFFFFFFFu);
      FPC cumsum = 0.0f;
      ISC sampled = logits.cols_ - 1;
      for (ISC a = 0; a < logits.cols_; ++a) {
        cumsum += (FPC)exp(logits.AtC(b, a) - lse.At(b, 0));
        if (u < cumsum) { sampled = a; break; }
      }
      actions[b] = sampled;
      logp[b] = logits.AtC(b, sampled) - lse.At(b, 0);
    }
    TTensorFree(lse);
  }

  void Backward(const TTensor& logits, const ISC* actions,
                const TTensor& advantages, const TTensor& old_logp,
                const TTensor& values, const TTensor& returns,
                TTensor& grad_logits, TTensor& grad_value) override {
    // Same base-gradient math as TTransformer::Backward (verified by M4).
    (void)old_logp;
    ISC B = logits.rows_, A = logits.cols_;
    for (ISC b = 0; b < B; ++b) {
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
      grad_value.At(b, 0) = 2.0f * (values.AtC(b, 0) - returns.AtC(b, 0));
    }
  }

  /* SGD step: W -= lr * dL/dW, where dL/dW = obs^T @ grad_logits (the linear
     head's hidden state is the obs). Also steps the value head. This is the
     weight update the TPPO loop calls after Backward. */
  void SgdStep(const TTensor& obs, const TTensor& grad_logits,
               const TTensor& grad_value, FPC lr) {
    ISC B = obs.rows_;
    // dW[a-feature][a] = sum_b obs[b, f] * grad_logits[b, a].
    // W_ is [obs_len, action_count]; grad_logits is [B, action_count].
    // dW_ = obs^T (action_count x obs_len ... wait, obs is B x obs_len).
    // dW_[f, a] = sum_b obs[b, f] * grad_logits[b, a].
    TTensor dW = TTensorAlloc(obs_len_, action_count_);
    TTensor db = TTensorAlloc(action_count_, 1);
    TTensor dWv = TTensorAlloc(obs_len_, 1);
    TTensor dbv = TTensorAlloc(1, 1);
    for (ISC b = 0; b < B; ++b) {
      for (ISC f = 0; f < obs_len_; ++f) {
        FPC ob = obs.AtC(b, f);
        for (ISC a = 0; a < action_count_; ++a)
          dW.At(f, a) += ob * grad_logits.AtC(b, a);
        dWv.At(f, 0) += ob * grad_value.AtC(b, 0);
      }
      for (ISC a = 0; a < action_count_; ++a)
        db.At(a, 0) += grad_logits.AtC(b, a);
      dbv.At(0, 0) += grad_value.AtC(b, 0);
    }
    // Average over the batch.
    FPC invB = 1.0f / (FPC)B;
    for (ISC f = 0; f < obs_len_; ++f)
      for (ISC a = 0; a < action_count_; ++a)
        W_.At(f, a) -= lr * invB * dW.At(f, a);
    for (ISC a = 0; a < action_count_; ++a) b_.At(a, 0) -= lr * invB * db.At(a, 0);
    for (ISC f = 0; f < obs_len_; ++f) W_v_.At(f, 0) -= lr * invB * dWv.At(f, 0);
    b_v_.At(0, 0) -= lr * invB * dbv.At(0, 0);
    TTensorFree(dW); TTensorFree(db); TTensorFree(dWv); TTensorFree(dbv);
  }

  /* Expose weights for diagnostics/tests. */
  TTensor& W() { return W_; }
  TTensor& Wv() { return W_v_; }

 private:
  ISC obs_len_;
  ISC action_count_;
  IUD seed_;
  TTensor W_, b_;
  TTensor W_v_, b_v_;
  BOL built_;
  FPC lr_;
  void TLcgInit() {}
};

}  // namespace _
