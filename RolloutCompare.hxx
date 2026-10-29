// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_ROLLOUT_COMPARE_HXX
#define IGEEK_ROLLOUT_COMPARE_HXX 1
#include "RolloutCompare.h"
namespace iGeek {

const CHA* RolloutClassText(RolloutClass cls) {
  switch (cls) {
    case RolloutIdentical: return "identical";
    case RolloutFlipped: return "flipped";
    case RolloutMissingA: return "missing_a";
    case RolloutMissingB: return "missing_b";
  }
  return "unknown";
}

BOL RolloutIsIdentical(const RolloutSummary* summary) {
  return summary && summary->identical;
}

void CompareRollout(const TScript* a, const TScript* b,
                    RolloutSummary* summary) {
  if (!summary) return;
  summary->matched = IUD(0);
  summary->flipped = IUD(0);
  summary->missing_a = IUD(0);
  summary->missing_b = IUD(0);
  summary->reward_a = FPD(0.0);
  summary->reward_b = FPD(0.0);
  summary->reward_delta = FPD(0.0);
  summary->identical = true;

  if (!a || !b) {
    summary->identical = false;
    if (!a && b)
      summary->missing_a = IUD(1);
    if (!b && a)
      summary->missing_b = IUD(1);
    return;
  }

  IUD count_a = TScriptCount(a);
  IUD count_b = TScriptCount(b);
  IUD min_count = count_a < count_b ? count_a : count_b;

  /* Aggregate rewards for episode_complete events (event=2). */
  FPD sum_a, min_a, max_a;
  IUD cnt_a;
  FPD sum_b, min_b, max_b;
  IUD cnt_b;
  BOL has_a = TScriptSummary(a, IUC(2), &sum_a, &cnt_a, &min_a, &max_a);
  BOL has_b = TScriptSummary(b, IUC(2), &sum_b, &cnt_b, &min_b, &max_b);
  if (has_a)
    summary->reward_a = sum_a;
  if (has_b)
    summary->reward_b = sum_b;
  summary->reward_delta = summary->reward_b - summary->reward_a;

  /* Compare counts and totals. */
  summary->matched = min_count;
  if (count_a != count_b) {
    if (count_a > count_b)
      summary->missing_b = count_a - count_b;
    else
      summary->missing_a = count_b - count_a;
    summary->identical = false;
  }
  if (!has_a || !has_b) {
    summary->identical = false;
    if (!has_a && has_b)
      summary->missing_a += IUD(1);
    if (!has_b && has_a)
      summary->missing_b += IUD(1);
  }
  if (has_a && has_b && sum_a != sum_b) {
    summary->flipped = min_count;
    summary->identical = false;
  }
}

}  //< namespace iGeek
#endif
