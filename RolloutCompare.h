// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_ROLLOUT_COMPARE_H
#define IGEEK_ROLLOUT_COMPARE_H
#include <_Config.h>
#if SEAM >= IGEEK_GYM_MANIFEST
#include "Script.h"
namespace iGeek {

/* Rollout comparison: compare two TScript rings (two Rooms that ran the
same environment) and classify each episode. The Room IS the sandbox; the
Script IS the rollout; comparing Scripts is comparing rollouts.

CompareRollout(a, b):
- Same episode_count: compare frame-by-frame.
- Different episode_count: compare min(counts), flag the extra.
- For each matched pair: classify as identical, flipped, or missing.
- Aggregate: matched, flipped, missing_a, missing_b, reward_delta. */

enum RolloutClass {
  RolloutIdentical = 0,
  RolloutFlipped = 1,
  RolloutMissingA = 2,
  RolloutMissingB = 3,
};

struct RolloutSummary {
  IUD matched;
  IUD flipped;
  IUD missing_a;
  IUD missing_b;
  FPD reward_a;
  FPD reward_b;
  FPD reward_delta;
  BOL identical;
};

void CompareRollout(const TScript* a, const TScript* b,
                    RolloutSummary* summary);
BOL RolloutIsIdentical(const RolloutSummary* summary);
const CHA* RolloutClassText(RolloutClass cls);

}  //< namespace iGeek
#endif
#endif
