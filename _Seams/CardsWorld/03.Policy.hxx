// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekCardsWorldCore
namespace CardsWorldTest {
using namespace ::_;
using namespace iGeek; 

namespace {
inline ISC& POLFAILS() {
  static ISC count = 0;
  return count;
}
#define POL_CHECK(cond)                                       \
  do {                                                        \
    if (!::_::Test(cond)) {                                   \
      ::_::RefOut() << "\nFAILURE Policy at line:" << __LINE__ \
                    << " in \"" << __FILE__ << "\"\n";        \
      ++POLFAILS();                                           \
    }                                                         \
  } while (0)
#define POL_CLOSE(a, b)                                       \
  do {                                                        \
    { FPC _d = (a) - (b); if (_d < 0) _d = -_d;              \
      if (!::_::Test(_d < 1e-4f)) {                          \
        ::_::RefOut() << "\nFAILURE Policy (close " << (a)   \
                      << " vs " << (b) << ") at line:"       \
                      << __LINE__ << " in \"" << __FILE__ << "\"\n"; \
        ++POLFAILS();                                         \
      } } } while (0)
}  //< namespace

inline const CHA* Policy(const CHA* args) {
  _ATestBegin;
  const ISC B = 4, A = 2, OBS = 4, DIM = 16, LAYERS = 2;

  // Build a fixed observation batch (4 tables, 4 observation features each).
  TensorFPC observation = TensorFPCAlloc(B, OBS);
  observation.At(0,0)=12.0f; observation.At(0,1)=5.0f; observation.At(0,2)=1.0f; observation.At(0,3)=0.0f;
  observation.At(1,0)=18.0f; observation.At(1,1)=9.0f; observation.At(1,2)=0.0f; observation.At(1,3)=0.0f;
  observation.At(2,0)=7.0f;  observation.At(2,1)=2.0f; observation.At(2,2)=1.0f; observation.At(2,3)=0.0f;
  observation.At(3,0)=21.0f; observation.At(3,1)=10.0f; observation.At(3,2)=1.0f; observation.At(3,3)=0.0f;

  _DROut("Policy: shape (logits [B,A], values [B])\n");
  {
    TransformerPolicy net(OBS, A, DIM, LAYERS);
    TensorFPC logits = TensorFPCAlloc(B, A);
    TensorFPC values = TensorFPCAlloc(B, 1);
    net.Forward(observation, logits, values);
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
    TensorFPCFree(logits); TensorFPCFree(values);
  }

  _DROut("Policy: determinism (two nets, same seed, same observation -> identical)\n");
  {
    TransformerPolicy n1(OBS, A, DIM, LAYERS);
    TransformerPolicy n2(OBS, A, DIM, LAYERS);
    TensorFPC l1 = TensorFPCAlloc(B, A), l2 = TensorFPCAlloc(B, A);
    TensorFPC v1 = TensorFPCAlloc(B, 1), v2 = TensorFPCAlloc(B, 1);
    n1.Forward(observation, l1, v1);
    n2.Forward(observation, l2, v2);
    for (ISC i = 0; i < B; ++i) {
      for (ISC j = 0; j < A; ++j) POL_CLOSE(l1.At(i,j), l2.At(i,j));
      POL_CLOSE(v1.At(i,0), v2.At(i,0));
    }
    // A third forward on the SAME net must also match (stateless w.r.t. observation).
    TensorFPC l1b = TensorFPCAlloc(B, A);
    n1.Forward(observation, l1b, v1);
    for (ISC i = 0; i < B; ++i)
      for (ISC j = 0; j < A; ++j) POL_CLOSE(l1.At(i,j), l1b.At(i,j));
    TensorFPCFree(l1); TensorFPCFree(l2); TensorFPCFree(l1b);
    TensorFPCFree(v1); TensorFPCFree(v2);
  }

  _DROut("Policy: different observation -> (generally) different logits\n");
  {
    TransformerPolicy net(OBS, A, DIM, LAYERS);
    TensorFPC l1 = TensorFPCAlloc(B, A), l2 = TensorFPCAlloc(B, A);
    TensorFPC v1 = TensorFPCAlloc(B, 1), v2 = TensorFPCAlloc(B, 1);
    net.Forward(observation, l1, v1);
    // Build a fresh observation with every feature shifted by +5 (different input).
    TensorFPC observationb = TensorFPCAlloc(B, OBS);
    for (ISC i = 0; i < B; ++i)
      for (ISC j = 0; j < OBS; ++j) observationb.At(i,j) = observation.At(i,j) + 5.0f;
    net.Forward(observationb, l2, v2);
    // At least one (i,j) should differ (deterministic net, different input).
    BOL any_diff = false;
    for (ISC i = 0; i < B && !any_diff; ++i)
      for (ISC j = 0; j < A; ++j) {
        FPC d = l1.At(i,j) - l2.At(i,j); if (d < 0) d = -d;
        if (d > 1e-4f) any_diff = true;
      }
    POL_CHECK(any_diff);
    TensorFPCFree(l1); TensorFPCFree(l2); TensorFPCFree(v1); TensorFPCFree(v2);
    TensorFPCFree(observationb);
  }

  _DROut("Policy: SampleActions (greedy argmax + logp <= 0)\n");
  {
    TransformerPolicy net(OBS, A, DIM, LAYERS);
    TensorFPC logits = TensorFPCAlloc(B, A);
    TensorFPC values = TensorFPCAlloc(B, 1);
    net.Forward(observation, logits, values);
    ISC actions[B]; FPC logp[B];
    net.SampleActions(logits, actions, logp);
    for (ISC i = 0; i < B; ++i) {
      POL_CHECK(actions[i] == 0 || actions[i] == 1);
      POL_CHECK(logp[i] <= 1e-6f);  // log-prob of a categorical is <= 0.
      // The chosen action must be the argmax of the logits.
      ISC best = (logits.At(i,1) > logits.At(i,0)) ? 1 : 0;
      POL_CHECK(actions[i] == best);
    }
    TensorFPCFree(logits); TensorFPCFree(values);
  }

  TensorFPCFree(observation);
  if (POLFAILS() != 0) {
    _DROut("\n" << POLFAILS() << " Policy assertion(s) FAILED\n");
    return "cards_world_policy_test_failure";
  }
  return _NilP;
}

}  //< namespace CardsWorldTest
#endif
