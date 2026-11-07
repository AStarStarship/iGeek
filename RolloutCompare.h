// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekRolloutCompareH
#define _iGeekRolloutCompareH
#include <_Config.h>
#if _Seam >= _iGeekEnvironmentRolloutCompare
#include "Room.hpp"
#include "AType.hpp"
namespace iGeek {
using namespace ::_;

/* Rollout comparison: compare two Rooms that ran the same environment.
The Room IS the sandbox and the Room IS the rollout record, so comparing
two Rooms is comparing two rollouts. There is no separate script or ring
type; the rollout lives in the Room's contiguous Floor memory.

Record convention: a rollout is recorded on the Room's Floor as a sequence
of numeric values, one per episode, starting at values index 1 — index 0 is
the Dic's keys Table. FPC, FPD, ISC and IUD values all read back as rewards.

CompareRollout(a, b, summary):
- Both nil         -> nothing matched, not identical.
- One nil          -> the other is reported missing.
- Equal length     -> walk pairwise; a differing reward counts as flipped.
- Different length -> compare the shorter prefix; the tail is missing.
Aggregate: matched, flipped, missing_a, missing_b, reward_a, reward_b,
reward_delta, identical. */

enum RolloutClass {
  RolloutIdentical = 0,
  RolloutFlipped = 1,
  RolloutMissingA = 2,
  RolloutMissingB = 3,
};

struct RolloutSummary {
  IUD matched;       //< Reward pairs compared.
  IUD flipped;       //< Pairs whose rewards differ.
  IUD missing_a;     //< Entries b has that a does not.
  IUD missing_b;     //< Entries a has that b does not.
  FPD reward_a;      //< Sum of a's rewards.
  FPD reward_b;      //< Sum of b's rewards.
  FPD reward_delta;  //< Absolute difference of the two sums.
  BOL identical;     //< No flips and no missing entries.
};

/* Number of rollout entries recorded on the Room's Floor.
   @param room  The Room holding the rollout; may be nil.
   @return      The entry count, 0 if the Room is nil or empty. */
IUD RolloutCount(const TRoom<ROM_D>* room);

/* Reads the rollout entry at the given index as a reward value.
   @param room   The Room holding the rollout; may be nil.
   @param index  Zero-based rollout entry index.
   @return       The reward, 0.0 if missing or not numeric. */
FPD RolloutRewardAt(const TRoom<ROM_D>* room, IUD index);

/* Appends one reward entry to the Room's rollout.
   @param room    The Room to append to; nil is ignored.
   @param reward  The reward value to record.
   @return        True on success, false if the Floor is full. */
BOL RolloutRecord(TRoom<ROM_D>* room, FPD reward);

/* Clears the Room's rollout, leaving the keys Table in place.
   @param room  The Room to clear; nil is ignored. */
void RolloutClear(TRoom<ROM_D>* room);

/* Compares two Room rollouts and fills the summary.
   @param a        First Room; may be nil.
   @param b        Second Room; may be nil.
   @param summary  Output summary; nil is ignored. */
void CompareRollout(const TRoom<ROM_D>* a, const TRoom<ROM_D>* b,
                    RolloutSummary* summary);

/* @return True when nothing flipped and nothing was missing. */
BOL RolloutIsIdentical(const RolloutSummary* summary);

/* @return The canonical lower-case text for a class. */
const CHA* RolloutClassText(RolloutClass cls);

}       //< namespace iGeek
#endif  //< _Seam >= _iGeekEnvironmentRolloutCompare
#endif  //< _iGeekRolloutCompareH
