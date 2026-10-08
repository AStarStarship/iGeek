// Copyright AStarship <https://astarship.net>.

// The card world's GYM seam unit: unit tests for the vectorized BlackjackGym
// (vectorized-batch) and the TTensor ops it's built on. Gated on
// CARDSWORLD_CORE so it runs with the default build (the first seam).

#if SEAM >= CARDSWORLD_CORE

using namespace ::_;
namespace CWTest {

namespace {
// Separate fail counter for the Gym unit (CWFails is private to 00.Core.hxx).
inline ISC& GYMFails() {
  static ISC count = 0;
  return count;
}

#define GYM_CHECK(cond)                                        \
  do {                                                         \
    if (!::_::Test(cond)) {                                    \
      ::_::StdOut() << "\nFAILURE Gym at line:" << __LINE__    \
                    << " in \"" << __FILE__ << "\"\n";         \
      ++GYMFails();                                            \
    }                                                          \
  } while (0)

#define GYM_EQ(a, b)                                           \
  do {                                                         \
    if (!::_::TestEq(a, b)) {                                  \
      ::_::StdOut() << "\nFAILURE Gym (expecting " << (a)      \
                    << " found " << (b) << ") at line:"        \
                    << __LINE__ << " in \"" << __FILE__ << "\"\n"; \
      ++GYMFails();                                            \
    }                                                          \
  } while (0)

#define GYM_CLOSE(a, b)                                        \
  do {                                                         \
    { FPC _d = (a) - (b); if (_d < 0) _d = -_d;               \
      if (!::_::Test(_d < 1e-3f)) {                           \
        ::_::StdOut() << "\nFAILURE Gym (close " << (a)       \
                      << " vs " << (b) << ") at line:"        \
                      << __LINE__ << " in \"" << __FILE__ << "\"\n"; \
        ++GYMFails();                                          \
      } } } while (0)

#define GYM_PTR(p)                                             \
  do {                                                         \
    if (IsError(p)) {                                          \
      ::_::StdOut() << "\nFAILURE Gym (nil ptr) at line:"     \
                    << __LINE__ << " in \"" << __FILE__ << "\"\n"; \
      ++GYMFails();                                            \
    }                                                          \
  } while (0)

// --- TTensor ops (M1) ---------------------------------------------------
inline void TensorChecks() {
  D_COUT("Gym: TTensor MatMul\n");
  {
    TTensor A = TTensorAlloc(1, 2); A.At(0,0)=1.0f; A.At(0,1)=2.0f;
    TTensor B = TTensorAlloc(2, 1); B.At(0,0)=3.0f; B.At(1,0)=4.0f;
    TTensor C = TTensorAlloc(1, 1);
    TTensorMatMul(A, B, C);
    GYM_CLOSE(C.At(0,0), 11.0f);  // 1*3 + 2*4
    TTensorFree(A); TTensorFree(B); TTensorFree(C);
  }
  D_COUT("Gym: TTensor Softmax\n");
  {
    TTensor X = TTensorAlloc(1,3); X.At(0,0)=1.0f; X.At(0,1)=2.0f; X.At(0,2)=3.0f;
    TTensorSoftmaxInPlace(X);
    GYM_CLOSE(X.At(0,2), 0.66524f);
    FPC s = X.At(0,0)+X.At(0,1)+X.At(0,2);
    GYM_CLOSE(s, 1.0f);
    TTensorFree(X);
  }
  D_COUT("Gym: TTensor LayerNorm\n");
  {
    TTensor X = TTensorAlloc(1,3); X.At(0,0)=1.0f; X.At(0,1)=2.0f; X.At(0,2)=3.0f;
    TTensorLayerNormInPlace(X);
    GYM_CLOSE(X.At(0,0), -1.22474f);
    GYM_CLOSE(X.At(0,1), 0.0f);
    GYM_CLOSE(X.At(0,2), 1.22474f);
    TTensorFree(X);
  }
  D_COUT("Gym: TTensor Relu/Add/Scale/LogSumExp\n");
  {
    TTensor X = TTensorAlloc(1,3); X.At(0,0)=-1.0f; X.At(0,1)=0.0f; X.At(0,2)=2.0f;
    TTensorReluInPlace(X);
    GYM_CLOSE(X.At(0,0), 0.0f); GYM_CLOSE(X.At(0,2), 2.0f);
    TTensorFree(X);
    TTensor A = TTensorAlloc(1,2); A.At(0,0)=1; A.At(0,1)=2;
    TTensor B = TTensorAlloc(1,2); B.At(0,0)=10; B.At(0,1)=20;
    TTensor C = TTensorAlloc(1,2);
    TTensorAdd(A, B, C);
    GYM_CLOSE(C.At(0,0), 11.0f); GYM_CLOSE(C.At(0,1), 22.0f);
    TTensor D = TTensorAlloc(1,2);
    TTensorScale(A, 5.0f, D);
    GYM_CLOSE(D.At(0,0), 5.0f); GYM_CLOSE(D.At(0,1), 10.0f);  // A=[1,2]*5
    TTensor LSE = TTensorAlloc(1,1);
    TTensor E = TTensorAlloc(1,2); E.At(0,0)=0; E.At(0,1)=0;
    TTensorRowLogSumExp(E, LSE);
    GYM_CLOSE(LSE.At(0,0), 0.69315f);  // ln(2)
    TTensorFree(A); TTensorFree(B); TTensorFree(C); TTensorFree(D);
    TTensorFree(E); TTensorFree(LSE);
  }
}

// --- BlackjackGym (M2) --------------------------------------------------
inline void GymChecks() {
  D_COUT("Gym: BlackjackGym constructs + resets (4 tables)\n");
  CardsWorld::BlackjackGym gym(4);
  GYM_EQ(gym.Tables(), 4);
  GYM_PTR(gym.Observations());
  GYM_PTR(gym.Rewards());
  GYM_PTR(gym.Terminals());
  GYM_PTR(gym.ActionMask());
  // After reset, every table has dealt 2 cards each: player hand value is in
  // [2, 21] and dealer up-card value in [1, 10].
  for (ISC i = 0; i < 4; ++i) {
    FPC pval = gym.Observations()[i * CardsWorld::BlackjackGym::ObsLength + 0];
    FPC dup = gym.Observations()[i * CardsWorld::BlackjackGym::ObsLength + 1];
    GYM_CHECK(pval >= 2.0f && pval <= 21.0f);
    GYM_CHECK(dup >= 1.0f && dup <= 10.0f);
  }
  D_COUT("Gym: step the batch (always stand -> rounds settle)\n");
  // Stand on every table: each round must settle (terminal) on this step.
  ISC stand[4] = {1, 1, 1, 1};
  gym.StepBatch(stand);
  for (ISC i = 0; i < 4; ++i) {
    // Standing immediately settles the round (dealer draws to 17).
    GYM_CLOSE(gym.Terminals()[i], 1.0f);
    // The reward is in the engine's set {-2, +1, +2, +3}.
    FPC r = gym.Rewards()[i];
    GYM_CHECK((r == -2.0f) || (r == 1.0f) || (r == 2.0f) || (r == 3.0f));
  }
  // The round-over flag in the obs must now be 1.0 for every table.
  for (ISC i = 0; i < 4; ++i) {
    GYM_CLOSE(gym.Observations()[i * CardsWorld::BlackjackGym::ObsLength + 3],
              1.0f);
  }
  D_COUT("Gym: reset re-deals (terminals back to 0 unless a natural)\n");
  gym.ResetBatch();
  for (ISC i = 0; i < 4; ++i) {
    // After a fresh deal, a table is only terminal if it dealt a natural.
    // In the common case it's running (0.0). We only assert the flag is a
    // valid 0.0/1.0 and that a fresh player value is in range.
    FPC t = gym.Terminals()[i];
    GYM_CHECK((t == 0.0f) || (t == 1.0f));
    FPC pval = gym.Observations()[i * CardsWorld::BlackjackGym::ObsLength + 0];
    GYM_CHECK(pval >= 2.0f && pval <= 21.0f);
  }
  D_COUT("Gym: many steps stay in-bounds (no crash, obs bounded)\n");
  // Run 200 batch-steps with a hit-until-17-ish policy; assert no crash and
  // that player hand values stay in the valid range every step.
  for (ISC step = 0; step < 200; ++step) {
    ISC actions[4];
    for (ISC i = 0; i < 4; ++i) {
      FPC pval = gym.Observations()[i * CardsWorld::BlackjackGym::ObsLength + 0];
      actions[i] = (pval < 17.0f) ? 0 : 1;  // hit under 17, else stand.
    }
    gym.StepBatch(actions);
    for (ISC i = 0; i < 4; ++i) {
      FPC pval = gym.Observations()[i * CardsWorld::BlackjackGym::ObsLength + 0];
      GYM_CHECK(pval >= 2.0f && pval <= 31.0f);  // may bust (>21), bounded.
      FPC t = gym.Terminals()[i];
      GYM_CHECK((t == 0.0f) || (t == 1.0f));
    }
    // If every table is done, reset to keep stepping.
    BOL all_done = true;
    for (ISC i = 0; i < 4; ++i)
      if (gym.Terminals()[i] != 1.0f) all_done = false;
    if (all_done) gym.ResetBatch();
  }
}

}  // namespace

inline const CHA* Gym(const CHA* args) {
  A_TEST_BEGIN;
  TensorChecks();
  GymChecks();
  if (GYMFails() != 0) {
    D_COUT("\n" << GYMFails() << " Gym assertion(s) FAILED\n");
    return "cards_world_gym_test_failure";
  }
  return NILP;
}

}  // namespace CWTest
#endif
