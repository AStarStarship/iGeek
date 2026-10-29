// Copyright AStarship <https://astarship.net>.
//
// Gym RolloutCompare seam unit: compare two TScript rings.
// Identical, flipped, missing, different-length, null cases.
// Gated on IGEEK_GYM_MANIFEST.
#pragma once
#include <_Config.h>
#if SEAM >= IGEEK_GYM_MANIFEST
#include "../../RolloutCompare.hxx"
namespace iGeekGymTest {
using namespace iGeek;

ISC rc_pass = 0;
ISC rc_fail = 0;

inline void RCCheck(BOL ok, const CHA* what) {
  if (::_::Test(ok)) {
    rc_pass++;
    ::_::StdOut() << "PASS " << what << '\n';
  } else {
    rc_fail++;
    ::_::StdOut() << "FAIL " << what << '\n';
  }
}

/* Seed a script with episode_complete events (event=2). */
inline void SeedScript(TScript* ring, const FPD* rewards,
                       const IUD* frames, IUD count) {
  TScriptInit(ring, 256);
  for (IUD i = IUD(0); i < count; ++i) {
    TScriptRecord(ring, frames[i], IUC(2), IUC(0), rewards[i], ISC(10));
  }
}

inline const CHA* RolloutCompare(const CHA* args) {
  (void)args;
  A_TEST_BEGIN;
  ::_::StdOut() << Headingf("Testing Gym RolloutCompare (compare two Rooms)...");

  /* Case 1: identical rollouts. */
  TScript a, b;
  const FPD rewards1[3] = {0.5, 0.7, 0.9};
  const IUD frames1[3] = {1, 2, 3};
  SeedScript(&a, rewards1, frames1, 3);
  SeedScript(&b, rewards1, frames1, 3);

  RolloutSummary s1;
  CompareRollout(&a, &b, &s1);
  RCCheck(s1.reward_a == FPD(2.1), "identical: reward_a=2.1 (sum)");
  RCCheck(s1.reward_b == FPD(2.1), "identical: reward_b=2.1 (sum)");
  RCCheck(s1.reward_delta == FPD(0.0), "identical: reward_delta=0");

  /* Case 2: flipped (different rewards). */
  const FPD rewards2[3] = {0.5, 0.3, 0.9};
  const IUD frames2[3] = {1, 2, 3};
  TScript c;
  SeedScript(&c, rewards2, frames2, 3);

  RolloutSummary s2;
  CompareRollout(&a, &c, &s2);
  RCCheck(s2.flipped == IUD(3), "flipped: flipped=3");
  RCCheck(!RolloutIsIdentical(&s2), "flipped: not identical");
  RCCheck(s2.reward_delta != FPD(0.0), "flipped: reward_delta != 0");

  /* Case 3: different length (b has more episodes). */
  const FPD rewards3[4] = {0.5, 0.7, 0.9, 1.0};
  const IUD frames3[4] = {1, 2, 3, 4};
  TScript d;
  SeedScript(&d, rewards3, frames3, 4);

  RolloutSummary s3;
  CompareRollout(&a, &d, &s3);
  RCCheck(s3.missing_a >= IUD(1), "diff_len: missing_a >= 1");
  RCCheck(!RolloutIsIdentical(&s3), "diff_len: not identical");

  /* Case 4: reversed (a has more episodes). */
  RolloutSummary s4;
  CompareRollout(&d, &a, &s4);
  RCCheck(s4.missing_b >= IUD(1), "diff_len_rev: missing_b >= 1");
  RCCheck(!RolloutIsIdentical(&s4), "diff_len_rev: not identical");

  /* Case 5: one null. */
  RolloutSummary s5;
  CompareRollout(NILP, &a, &s5);
  RCCheck(s5.missing_a == IUD(1), "null_a: missing_a=1");
  RCCheck(!RolloutIsIdentical(&s5), "null_a: not identical");

  RolloutSummary s6;
  CompareRollout(&a, NILP, &s6);
  RCCheck(s6.missing_b == IUD(1), "null_b: missing_b=1");
  RCCheck(!RolloutIsIdentical(&s6), "null_b: not identical");

  /* Case 6: both null. */
  RolloutSummary s7;
  CompareRollout(NILP, NILP, &s7);
  RCCheck(!RolloutIsIdentical(&s7), "null_both: not identical");

  /* Class text. */
  RCCheck(::_::StrCmp(RolloutClassText(RolloutIdentical), "identical") == 0,
         "class text identical");
  RCCheck(::_::StrCmp(RolloutClassText(RolloutFlipped), "flipped") == 0,
         "class text flipped");
  RCCheck(::_::StrCmp(RolloutClassText(RolloutMissingA), "missing_a") == 0,
         "class text missing_a");
  RCCheck(::_::StrCmp(RolloutClassText(RolloutMissingB), "missing_b") == 0,
         "class text missing_b");

  ::_::StdOut() << "rc_pass=" << rc_pass << " rc_fail=" << rc_fail << '\n';
  return rc_fail ? "GYM_ROLLOUT_COMPARE_FAIL: " : NILP;
}

}  //< namespace iGeekGymTest
#endif
