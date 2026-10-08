// Copyright AStarship <https://astarship.net>.

// The card world's POLICY seam unit: unit tests for the TTransformer forward
// (M3). Gated on CARDSWORLD_CORE so it runs with the default build.
//
// What we verify (forward is the deliverable for M3; backward/gradcheck is M4):
//   - Shape: logits is [B, A], values is [B].
//   - Determinism: two independently-constructed nets (same seed) produce
//     IDENTICAL logits + values for the same obs.
//   - Sanity: logits and values are finite (no NaN/Inf) and within a sane
//     range for the small net.
//   - SampleActions: greedy argmax is consistent with the logits, and the
//     log-prob is <= 0.

#if SEAM >= CARDSWORLD_CORE

using namespace ::_;
namespace CWTest {

namespace {
inline ISC& POLFAILS() {
  static ISC count = 0;
  return count;
}
#define POL_CHECK(cond)                                       \
  do {                                                        \
    if (!::_::Test(cond)) {                                   \
      ::_::StdOut() << "\nFAILURE Policy at line:" << __LINE__ \
                    << " in \"" << __FILE__ << "\"\n";        \
      ++POLFAILS();                                           \
    }                                                         \
  } while (0)
#define POL_CLOSE(a, b)                                       \
  do {                                                        \
    { FPC _d = (a) - (b); if (_d < 0) _d = -_d;              \
      if (!::_::Test(_d < 1e-4f)) {                          \
        ::_::StdOut() << "\nFAILURE Policy (close " << (a)   \
                      << " vs " << (b) << ") at line:"       \
                      << __LINE__ << " in \"" << __FILE__ << "\"\n"; \
        ++POLFAILS();                                         \
      } } } while (0)
}  // namespace

inline const CHA* Policy(const CHA* args) {
  A_TEST_BEGIN;
  const ISC B = 4, A = 2, OBS = 4, DIM = 16, LAYERS = 2;

  // Build a fixed observation batch (4 tables, 4 obs features each).
  TTensor obs = TTensorAlloc(B, OBS);
  obs.At(0,0)=12.0f; obs.At(0,1)=5.0f; obs.At(0,2)=1.0f; obs.At(0,3)=0.0f;
  obs.At(1,0)=18.0f; obs.At(1,1)=9.0f; obs.At(1,2)=0.0f; obs.At(1,3)=0.0f;
  obs.At(2,0)=7.0f;  obs.At(2,1)=2.0f; obs.At(2,2)=1.0f; obs.At(2,3)=0.0f;
  obs.At(3,0)=21.0f; obs.At(3,1)=10.0f; obs.At(3,2)=1.0f; obs.At(3,3)=0.0f;

  D_COUT("Policy: shape (logits [B,A], values [B])\n");
  {
    TTransformer net(OBS, A, DIM, LAYERS);
    TTensor logits = TTensorAlloc(B, A);
    TTensor values = TTensorAlloc(B, 1);
    net.Forward(obs, logits, values);
    POL_CHECK(logits.rows_ == B && logits.cols_ == A);
    POL_CHECK(values.rows_ == B && values.cols_ == 1);
    // Finite + sane range (small net, inputs <= 21). A value is "finite"
    // here if it is not NaN (x != x) and bounded; for a small net we bound
    // at 1e6 (well above any sane output).
    for (ISC i = 0; i < B; ++i) {
      POL_CHECK(logits.At(i,0) == logits.At(i,0));  // not NaN
      POL_CHECK(logits.At(i,1) == logits.At(i,1));
      POL_CHECK(values.At(i,0) == values.At(i,0));
      POL_CHECK(values.At(i,0) < 1e6f && values.At(i,0) > -1e6f);
      POL_CHECK(logits.At(i,0) < 1e6f && logits.At(i,0) > -1e6f);
    }
    TTensorFree(logits); TTensorFree(values);
  }

  D_COUT("Policy: determinism (two nets, same seed, same obs -> identical)\n");
  {
    TTransformer n1(OBS, A, DIM, LAYERS);
    TTransformer n2(OBS, A, DIM, LAYERS);
    TTensor l1 = TTensorAlloc(B, A), l2 = TTensorAlloc(B, A);
    TTensor v1 = TTensorAlloc(B, 1), v2 = TTensorAlloc(B, 1);
    n1.Forward(obs, l1, v1);
    n2.Forward(obs, l2, v2);
    for (ISC i = 0; i < B; ++i) {
      for (ISC j = 0; j < A; ++j) POL_CLOSE(l1.At(i,j), l2.At(i,j));
      POL_CLOSE(v1.At(i,0), v2.At(i,0));
    }
    // A third forward on the SAME net must also match (stateless w.r.t. obs).
    TTensor l1b = TTensorAlloc(B, A);
    n1.Forward(obs, l1b, v1);
    for (ISC i = 0; i < B; ++i)
      for (ISC j = 0; j < A; ++j) POL_CLOSE(l1.At(i,j), l1b.At(i,j));
    TTensorFree(l1); TTensorFree(l2); TTensorFree(l1b);
    TTensorFree(v1); TTensorFree(v2);
  }

  D_COUT("Policy: different obs -> (generally) different logits\n");
  {
    TTransformer net(OBS, A, DIM, LAYERS);
    TTensor l1 = TTensorAlloc(B, A), l2 = TTensorAlloc(B, A);
    TTensor v1 = TTensorAlloc(B, 1), v2 = TTensorAlloc(B, 1);
    net.Forward(obs, l1, v1);
    // Build a fresh obs with every feature shifted by +5 (different input).
    TTensor obsb = TTensorAlloc(B, OBS);
    for (ISC i = 0; i < B; ++i)
      for (ISC j = 0; j < OBS; ++j) obsb.At(i,j) = obs.AtC(i,j) + 5.0f;
    net.Forward(obsb, l2, v2);
    // At least one (i,j) should differ (deterministic net, different input).
    BOL any_diff = false;
    for (ISC i = 0; i < B && !any_diff; ++i)
      for (ISC j = 0; j < A; ++j) {
        FPC d = l1.At(i,j) - l2.At(i,j); if (d < 0) d = -d;
        if (d > 1e-4f) any_diff = true;
      }
    POL_CHECK(any_diff);
    TTensorFree(l1); TTensorFree(l2); TTensorFree(v1); TTensorFree(v2);
    TTensorFree(obsb);
  }

  D_COUT("Policy: SampleActions (greedy argmax + logp <= 0)\n");
  {
    TTransformer net(OBS, A, DIM, LAYERS);
    TTensor logits = TTensorAlloc(B, A);
    TTensor values = TTensorAlloc(B, 1);
    net.Forward(obs, logits, values);
    ISC actions[B]; FPC logp[B];
    net.SampleActions(logits, actions, logp);
    for (ISC i = 0; i < B; ++i) {
      POL_CHECK(actions[i] == 0 || actions[i] == 1);
      POL_CHECK(logp[i] <= 1e-6f);  // log-prob of a categorical is <= 0.
      // The chosen action must be the argmax of the logits.
      ISC best = (logits.At(i,1) > logits.At(i,0)) ? 1 : 0;
      POL_CHECK(actions[i] == best);
    }
    TTensorFree(logits); TTensorFree(values);
  }

  TTensorFree(obs);
  if (POLFAILS() != 0) {
    D_COUT("\n" << POLFAILS() << " Policy assertion(s) FAILED\n");
    return "cards_world_policy_test_failure";
  }
  return NILP;
}

}  // namespace CWTest
#endif
