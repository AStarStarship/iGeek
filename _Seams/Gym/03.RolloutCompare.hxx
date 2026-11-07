// Copyright AStarship <https://astarship.net>.
#include <_Config.h>
#if _Seam >= _iGeekEnvironmentRolloutCompare
#include "../../RolloutCompare.hxx"
namespace iGeekGymTest {
using namespace ::_;
using namespace iGeek;

ISC rc_pass = 0;
ISC rc_fail = 0;

inline void RCCheck(BOL ok, const CHA* what) {
  if (::_::Test(ok)) {
    rc_pass++;
    ::_::RefOut() << "PASS " << what << '\n';
  } else {
    rc_fail++;
    ::_::RefOut() << "FAIL " << what << '\n';
  }
}

/* Records a rollout of rewards into a Room's Floor. */
inline void SeedRoom(TRoom<ROM_D>* room, const FPD* rewards, IUD count) {
  RolloutClear(room);
  for (IUD i = IUD(0); i < count; ++i) RolloutRecord(room, rewards[i]);
}

inline const CHA* RolloutCompare(const CHA* args) {
  _ATestBegin;
  ::_::RefOut() << Headingf(
      "Testing Gym RolloutCompare (two Rooms, not two Scripts)...");

  const FPD rewards1[3] = {0.5, 0.7, 0.9};
  const FPD rewards2[3] = {0.5, 0.3, 0.9};
  const FPD rewards3[4] = {0.5, 0.7, 0.9, 1.0};

  /* Case 1: identical rollouts. */
  {
    TRoom<ROM_D> a("rollout_a"), b("rollout_b");
    SeedRoom(&a, rewards1, IUD(3));
    SeedRoom(&b, rewards1, IUD(3));

    RCCheck(RolloutCount(&a) == IUD(3), "count: a has 3 entries");
    RCCheck(RolloutRewardAt(&a, IUD(0)) == FPD(0.5), "at(0): 0.5");
    RCCheck(RolloutRewardAt(&a, IUD(2)) == FPD(0.9), "at(2): 0.9");
    RCCheck(RolloutRewardAt(&a, IUD(3)) == FPD(0.0), "at(3): past end is 0");

    RolloutSummary s1;
    CompareRollout(&a, &b, &s1);
    RCCheck(s1.matched == IUD(3), "identical: matched=3");
    RCCheck(s1.flipped == IUD(0), "identical: flipped=0");
    RCCheck(s1.reward_a == FPD(2.1), "identical: reward_a=2.1");
    RCCheck(s1.reward_b == FPD(2.1), "identical: reward_b=2.1");
    RCCheck(s1.reward_delta == FPD(0.0), "identical: reward_delta=0");
    RCCheck(RolloutIsIdentical(&s1), "identical: is identical");
  }

  /* Case 2: same length, one differing reward -> flipped. */
  {
    TRoom<ROM_D> a("rollout_a"), c("rollout_c");
    SeedRoom(&a, rewards1, IUD(3));
    SeedRoom(&c, rewards2, IUD(3));

    RolloutSummary s2;
    CompareRollout(&a, &c, &s2);
    RCCheck(s2.matched == IUD(3), "flipped: matched=3");
    RCCheck(s2.flipped == IUD(1), "flipped: flipped=1 (0.7 vs 0.3 only)");
    RCCheck(!RolloutIsIdentical(&s2), "flipped: not identical");
    RCCheck(s2.reward_delta != FPD(0.0), "flipped: reward_delta != 0");
  }

  /* Case 3: b is longer -> entries a cannot account for. */
  {
    TRoom<ROM_D> a("rollout_a"), d("rollout_d");
    SeedRoom(&a, rewards1, IUD(3));
    SeedRoom(&d, rewards3, IUD(4));

    RolloutSummary s3;
    CompareRollout(&a, &d, &s3);
    RCCheck(s3.matched == IUD(3), "diff_len: matched=3");
    RCCheck(s3.missing_a == IUD(1), "diff_len: missing_a=1");
    RCCheck(s3.missing_b == IUD(0), "diff_len: missing_b=0");
    RCCheck(!RolloutIsIdentical(&s3), "diff_len: not identical");
  }

  /* Case 4: reversed -> a is longer. */
  {
    TRoom<ROM_D> a("rollout_a"), d("rollout_d");
    SeedRoom(&a, rewards1, IUD(3));
    SeedRoom(&d, rewards3, IUD(4));

    RolloutSummary s4;
    CompareRollout(&d, &a, &s4);
    RCCheck(s4.missing_b == IUD(1), "diff_len_rev: missing_b=1");
    RCCheck(s4.missing_a == IUD(0), "diff_len_rev: missing_a=0");
    RCCheck(!RolloutIsIdentical(&s4), "diff_len_rev: not identical");
  }

  /* Case 5: one nil. */
  {
    TRoom<ROM_D> a("rollout_a");
    SeedRoom(&a, rewards1, IUD(3));

    RolloutSummary s5;
    CompareRollout(_NilP, &a, &s5);
    RCCheck(s5.missing_a == IUD(1), "null_a: missing_a=1");
    RCCheck(!RolloutIsIdentical(&s5), "null_a: not identical");

    RolloutSummary s6;
    CompareRollout(&a, _NilP, &s6);
    RCCheck(s6.missing_b == IUD(1), "null_b: missing_b=1");
    RCCheck(!RolloutIsIdentical(&s6), "null_b: not identical");
  }

  /* Case 6: both nil. */
  {
    RolloutSummary s7;
    CompareRollout(_NilP, _NilP, &s7);
    RCCheck(!RolloutIsIdentical(&s7), "null_both: not identical");
    RCCheck(s7.matched == IUD(0), "null_both: matched=0");
  }

  /* Case 7: an empty Room is a zero-length rollout, not a failure. */
  {
    TRoom<ROM_D> empty("rollout_empty"), a("rollout_a");
    SeedRoom(&a, rewards1, IUD(3));

    RolloutSummary s8;
    CompareRollout(&empty, &a, &s8);
    RCCheck(RolloutCount(&empty) == IUD(0), "empty: count=0");
    RCCheck(s8.matched == IUD(0), "empty: matched=0");
    RCCheck(s8.missing_a == IUD(3), "empty: missing_a=3");
    RCCheck(!RolloutIsIdentical(&s8), "empty: not identical");
  }

  /* Class text. */
  RCCheck(::TSCompare<CHA, CHD>(RolloutClassText(RolloutIdentical),
                                "identical") == 0,
          "class text identical");
  RCCheck(::TSCompare<CHA, CHD>(RolloutClassText(RolloutFlipped),
                                "flipped") == 0,
          "class text flipped");
  RCCheck(::TSCompare<CHA, CHD>(RolloutClassText(RolloutMissingA),
                                "missing_a") == 0,
          "class text missing_a");
  RCCheck(::TSCompare<CHA, CHD>(RolloutClassText(RolloutMissingB),
                                "missing_b") == 0,
          "class text missing_b");

  ::_::RefOut() << "rc_pass=" << rc_pass << " rc_fail=" << rc_fail << '\n';
  return rc_fail ? "GYM_ROLLOUT_COMPARE_FAIL: " : _NilP;
}

}  //< namespace iGeekGymTest
#endif
