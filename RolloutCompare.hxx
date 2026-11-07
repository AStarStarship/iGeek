// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekRolloutCompareHXX
#define _iGeekRolloutCompareHXX
#include "RolloutCompare.h"
namespace iGeek {
using namespace ::_;

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

/* Returns the Room's Floor value list, or nil when the Room is nil.
   Floor() is typed with the Room's own template params; the Dic's value
   list always uses the LST_ defaults, so TList<> names it exactly. */
static const TList<>* RolloutValues(const TRoom<ROM_D>* room) {
  if (!room) return _NilP;
  auto floor = room->Floor();
  if (!floor) return _NilP;
  return &floor->values;
}

IUD RolloutCount(const TRoom<ROM_D>* room) {
  const TList<>* values = RolloutValues(room);
  if (!values) return IUD(0);
  ISC count = ISC(values->map.count);
  // Index 0 is the Dic's keys Table, not a rollout entry.
  return count > ISC(1) ? IUD(count - ISC(1)) : IUD(0);
}

FPD RolloutRewardAt(const TRoom<ROM_D>* room, IUD index) {
  const TList<>* values = RolloutValues(room);
  if (!values) return FPD(0.0);
  ISC i = ISC(index) + ISC(1);  // skip the keys Table at index 0.
  if (i < ISC(1) || i >= ISC(values->map.count)) return FPD(0.0);
  DTB type = TListType<>(values, ISR(i));
  if (type == _FPD) {
    const FPD* p = TListValue<LST_D, FPD>(values, ISR(i));
    return p ? *p : FPD(0.0);
  }
  if (type == _FPC) {
    const FPC* p = TListValue<LST_D, FPC>(values, ISR(i));
    return p ? FPD(*p) : FPD(0.0);
  }
  if (type == _ISC) {
    const ISC* p = TListValue<LST_D, ISC>(values, ISR(i));
    return p ? FPD(*p) : FPD(0.0);
  }
  if (type == _IUD) {
    const IUD* p = TListValue<LST_D, IUD>(values, ISR(i));
    return p ? FPD(*p) : FPD(0.0);
  }
  return FPD(0.0);
}

BOL RolloutRecord(TRoom<ROM_D>* room, FPD reward) {
  if (!room) return false;
  auto floor = room->Floor();
  if (!floor) return false;
  return TListInsert<>(&floor->values, reward) >= ISR(0);
}

void RolloutClear(TRoom<ROM_D>* room) {
  if (!room) return;
  room->ClearFloor();
}

/* Sums every rollout entry recorded in a Room. */
static FPD RolloutRewardSum(const TRoom<ROM_D>* room) {
  FPD sum = FPD(0.0);
  IUD count = RolloutCount(room);
  for (IUD i = IUD(0); i < count; ++i) sum += RolloutRewardAt(room, i);
  return sum;
}

void CompareRollout(const TRoom<ROM_D>* a, const TRoom<ROM_D>* b,
                    RolloutSummary* summary) {
  if (!summary) return;
  summary->matched = IUD(0);
  summary->flipped = IUD(0);
  summary->missing_a = IUD(0);
  summary->missing_b = IUD(0);
  summary->reward_a = FPD(0.0);
  summary->reward_b = FPD(0.0);
  summary->reward_delta = FPD(0.0);
  summary->identical = false;

  /* Nothing on either side: no rollout was recorded at all. */
  if (!a && !b) return;

  /* One side absent: that side's rollout is entirely missing. */
  if (!a) {
    summary->missing_a = IUD(1);
    summary->reward_b = RolloutRewardSum(b);
    return;
  }
  if (!b) {
    summary->missing_b = IUD(1);
    summary->reward_a = RolloutRewardSum(a);
    return;
  }

  IUD count_a = RolloutCount(a);
  IUD count_b = RolloutCount(b);
  IUD min_count = count_a < count_b ? count_a : count_b;

  /* Walk the common prefix pairwise. */
  for (IUD i = IUD(0); i < min_count; ++i) {
    FPD ra = RolloutRewardAt(a, i);
    FPD rb = RolloutRewardAt(b, i);
    summary->reward_a += ra;
    summary->reward_b += rb;
    ++summary->matched;
    if (ra != rb) ++summary->flipped;
  }

  /* Sum the tail of the longer rollout so the totals are complete. */
  for (IUD i = min_count; i < count_a; ++i)
    summary->reward_a += RolloutRewardAt(a, i);
  for (IUD i = min_count; i < count_b; ++i)
    summary->reward_b += RolloutRewardAt(b, i);

  if (count_a > count_b)
    summary->missing_b = count_a - count_b;
  else
    summary->missing_a = count_b - count_a;

  summary->reward_delta = summary->reward_a > summary->reward_b
      ? summary->reward_a - summary->reward_b
      : summary->reward_b - summary->reward_a;

  summary->identical = summary->flipped == IUD(0) &&
                       summary->missing_a == IUD(0) &&
                       summary->missing_b == IUD(0);
}

}  //< namespace iGeek
#endif  //< _iGeekRolloutCompareHXX
