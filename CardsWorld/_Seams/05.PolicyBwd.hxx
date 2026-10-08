// Copyright AStarship <https://astarship.net>.

// The card world's POLICY-BACKWARD seam unit: the M4 finite-difference
// gradcheck — the load-bearing correctness gate for the PPO loop. Verifies
// TTransformer::Backward's grad_logits/grad_value match the numeric
// (f(x+eps)-f(x-eps))/2eps gradient of TPolicyLoss/TValueLoss.
// Gated on CARDSWORLD_CORE.

#if SEAM >= CARDSWORLD_CORE

using namespace ::_;
namespace CWTest {

namespace {
inline ISC& PBFAILS() {
  static ISC count = 0;
  return count;
}
#define PB_CHECK(cond)                                       \
  do {                                                       \
    if (!::_::Test(cond)) {                                  \
      ::_::StdOut() << "\nFAILURE PolicyBwd at line:" << __LINE__ \
                    << " in \"" << __FILE__ << "\"\n";       \
      ++PBFAILS();                                           \
    }                                                        \
  } while (0)
// Check |a - b| < tol (relative-ish).
#define PB_CLOSE(a, b, tol)                                  \
  do {                                                       \
    { FPC _d = (a) - (b); if (_d < 0) _d = -_d;             \
      if (!::_::Test(_d < (tol))) {                         \
        ::_::StdOut() << "\nFAILURE PolicyBwd (close " << (ISC)((a)*1e6f) \
                      << " vs " << (ISC)((b)*1e6f) << ") at line:" \
                      << __LINE__ << " in \"" << __FILE__ << "\"\n"; \
        ++PBFAILS();                                         \
      } } } while (0)
}  // namespace

inline const CHA* PolicyBwd(const CHA* args) {
  A_TEST_BEGIN;
  const ISC B = 3, A = 3;
  const FPC eps = 1e-3f;

  // Fixed inputs (small, non-degenerate so gradients are non-trivial).
  TTensor logits = TTensorAlloc(B, A);
  // Distinct logits per row so softmax is not uniform.
  logits.At(0,0)=0.3f; logits.At(0,1)=1.1f; logits.At(0,2)=-0.2f;
  logits.At(1,0)=2.0f; logits.At(1,1)=-1.5f; logits.At(1,2)=0.7f;
  logits.At(2,0)=-0.8f; logits.At(2,1)=0.4f; logits.At(2,2)=1.9f;
  ISC actions[B] = {1, 0, 2};
  TTensor adv = TTensorAlloc(B, 1);
  adv.At(0,0)=0.9f; adv.At(1,0)=-1.3f; adv.At(2,0)=0.5f;
  TTensor values = TTensorAlloc(B, 1);
  values.At(0,0)=0.4f; values.At(1,0)=-0.6f; values.At(2,0)=1.1f;
  TTensor returns = TTensorAlloc(B, 1);
  returns.At(0,0)=1.0f; returns.At(1,0)=-0.2f; returns.At(2,0)=0.8f;
  TTensor old_logp = TTensorAlloc(B, 1);  // unused by base grad.
  old_logp.At(0,0)=0; old_logp.At(1,0)=0; old_logp.At(2,0)=0;

  D_COUT("PolicyBwd: gradcheck grad_logits (analytic vs finite-diff)\n");
  // Analytic grad from Backward.
  TTransformer net(4, A, 8, 1);  // net shape irrelevant to the base grad.
  TTensor grad_logits = TTensorAlloc(B, A);
  TTensor grad_value = TTensorAlloc(B, 1);
  net.Backward(logits, actions, adv, old_logp, values, returns,
               grad_logits, grad_value);
  // Finite-difference each logits[b,a].
  ISC max_err_idx = -1; FPC max_err = 0.0f;
  for (ISC b = 0; b < B; ++b) {
    for (ISC a = 0; a < A; ++a) {
      FPC lo = logits.At(b, a);
      // f(x+eps)
      logits.At(b, a) = lo + eps;
      FPC f_hi = TPolicyLoss(logits, actions, adv);
      // f(x-eps)
      logits.At(b, a) = lo - eps;
      FPC f_lo = TPolicyLoss(logits, actions, adv);
      // restore
      logits.At(b, a) = lo;
      FPC num_grad = (f_hi - f_lo) / (2.0f * eps);
      FPC ana = grad_logits.At(b, a);
      FPC d = ana - num_grad; if (d < 0) d = -d;
      if (d > max_err) { max_err = d; max_err_idx = b * A + a; }
      // Tolerance: 1e-2 abs is generous for FPC + central diff at eps=1e-3.
      PB_CHECK(d < 1e-2f);
    }
  }
  (void)max_err_idx;

  D_COUT("PolicyBwd: gradcheck grad_value (analytic vs finite-diff)\n");
  for (ISC b = 0; b < B; ++b) {
    FPC lo = values.At(b, 0);
    values.At(b, 0) = lo + eps;
    FPC f_hi = TValueLoss(values, returns);
    values.At(b, 0) = lo - eps;
    FPC f_lo = TValueLoss(values, returns);
    values.At(b, 0) = lo;
    FPC num_grad = (f_hi - f_lo) / (2.0f * eps);
    FPC ana = grad_value.At(b, 0);
    FPC d = ana - num_grad; if (d < 0) d = -d;
    PB_CHECK(d < 1e-2f);
    // Also hand-verify: dV/dvalue = 2*(value - return).
    FPC expect = 2.0f * (lo - returns.At(b, 0));
    FPC de = ana - expect; if (de < 0) de = -de;
    PB_CHECK(de < 1e-4f);
  }

  D_COUT("PolicyBwd: grad_logits row-sums to ~ -adv (identity check)\n");
  // For the policy surrogate, sum_a dL/dlogits[b,a] = -adv[b] * (1 - sum_a p)
  // = -adv[b] * (1 - 1) = 0. So each row's grad_logits should sum to ~0.
  for (ISC b = 0; b < B; ++b) {
    FPC rowsum = 0.0f;
    for (ISC a = 0; a < A; ++a) rowsum += grad_logits.At(b, a);
    FPC dr = rowsum; if (dr < 0) dr = -dr;
    PB_CHECK(dr < 1e-3f);
  }

  TTensorFree(logits); TTensorFree(adv); TTensorFree(values);
  TTensorFree(returns); TTensorFree(old_logp); TTensorFree(grad_logits);
  TTensorFree(grad_value);

  if (PBFAILS() != 0) {
    D_COUT("\n" << PBFAILS() << " PolicyBwd assertion(s) FAILED\n");
    return "cards_world_policy_bwd_test_failure";
  }
  D_COUT("PolicyBwd: gradcheck PASSED (max err " << (ISC)(max_err*1e6f) << "e-6)\n");
  return NILP;
}

}  // namespace CWTest
#endif
