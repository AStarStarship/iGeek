// Copyright AStarship <https://astarship.net>.
//
// Gym Script seam unit: SCRIPT (Serial Chinese Room, Interprocess,
// and Telemetry) record/query/summary/iterate round-trip.
// Gated on IGEEK_GYM_MANIFEST.
#pragma once
#include <_Config.h>
#if SEAM >= IGEEK_GYM_MANIFEST
#include "../../Script.hxx"
namespace iGeekGymTest {
using namespace iGeek;

ISC sc_pass = 0;
ISC sc_fail = 0;

inline void SCheck(BOL ok, const CHA* what) {
  if (::_::Test(ok)) {
    sc_pass++;
    ::_::StdOut() << "PASS " << what << '\n';
  } else {
    sc_fail++;
    ::_::StdOut() << "FAIL " << what << '\n';
  }
}

/* Iterator callback: counts rewards. */
struct IterCtx {
  IUD reward_count;
  FPD reward_sum;
};
static BOL CountRewards(const TScrTuple* t, void* user) {
  IterCtx* ctx = static_cast<IterCtx*>(user);
  if (t->event == ScrReward) {
    ++ctx->reward_count;
    ctx->reward_sum += t->value;
  }
  return true;
}

inline const CHA* Script(const CHA* args) {
  (void)args;
  A_TEST_BEGIN;
  ::_::StdOut() << Headingf("Testing Gym Script (SCRIPT record/query/summary)...");

  TScript script;
  TScriptInit(&script, IUD(10));

  /* Initially empty. */
  SCheck(TScriptCount(&script) == IUD(0), "initially empty");

  /* Record 5 frames. */
  SCheck(TScriptRecord(&script, IUD(0), ScrFrame, IUC(1), FPD(0.0), ISC(0)),
         "record frame 0");
  SCheck(TScriptRecord(&script, IUD(1), ScrFrame, IUC(1), FPD(0.0), ISC(0)),
         "record frame 1");
  SCheck(TScriptRecord(&script, IUD(2), ScrFrame, IUC(1), FPD(0.0), ISC(0)),
         "record frame 2");
  SCheck(TScriptRecord(&script, IUD(3), ScrFrame, IUC(1), FPD(0.0), ISC(0)),
         "record frame 3");
  SCheck(TScriptRecord(&script, IUD(4), ScrFrame, IUC(1), FPD(0.0), ISC(0)),
         "record frame 4");
  SCheck(TScriptCount(&script) == IUD(5), "count is 5 after 5 records");

  /* Record rewards for agent 1. */
  SCheck(TScriptRecord(&script, IUD(1), ScrReward, IUC(1), FPD(0.5), ISC(0)),
         "record reward 0.5");
  SCheck(TScriptRecord(&script, IUD(3), ScrReward, IUC(1), FPD(1.0), ISC(0)),
         "record reward 1.0");
  SCheck(TScriptCount(&script) == IUD(7), "count is 7 after 2 rewards");

  /* Record a join. */
  SCheck(TScriptRecord(&script, IUD(0), ScrJoin, IUC(1), FPD(0.0), ISC(0)),
         "record join");
  SCheck(TScriptCount(&script) == IUD(8), "count is 8 after join");

  /* Record a leave. */
  SCheck(TScriptRecord(&script, IUD(4), ScrLeave, IUC(1), FPD(0.0), ISC(0)),
         "record leave");
  SCheck(TScriptCount(&script) == IUD(9), "count is 9 after leave");

  /* Query: most recent reward for agent 1. */
  FPD q_val;
  IUD q_frame;
  SCheck(TScriptQuery(&script, ScrReward, IUC(1), &q_val, &q_frame),
         "query reward for agent 1");
  SCheck(q_val == FPD(1.0), "query reward value 1.0");
  SCheck(q_frame == IUD(3), "query reward frame 3");

  /* Query: join for agent 1. */
  SCheck(TScriptQuery(&script, ScrJoin, IUC(1), &q_val, &q_frame),
         "query join for agent 1");
  SCheck(q_frame == IUD(0), "query join frame 0");

  /* Query: non-existent event. */
  SCheck(!TScriptQuery(&script, ScrError, IUC(1), &q_val, &q_frame),
         "query non-existent event fails");

  /* Summary: rewards for all agents. */
  FPD sum, min_v, max_v;
  IUD count;
  SCheck(TScriptSummary(&script, ScrReward, &sum, &count, &min_v, &max_v),
         "summary rewards");
  SCheck(count == IUD(2), "summary reward count 2");
  SCheck(sum == FPD(1.5), "summary reward sum 1.5");
  SCheck(min_v == FPD(0.5), "summary reward min 0.5");
  SCheck(max_v == FPD(1.0), "summary reward max 1.0");

  /* Summary: frames. */
  SCheck(TScriptSummary(&script, ScrFrame, &sum, &count, &min_v, &max_v),
         "summary frames");
  SCheck(count == IUD(5), "summary frame count 5");

  /* Iterate: count rewards. */
  IterCtx ctx = {IUD(0), FPD(0.0)};
  TScriptIterate(&script, CountRewards, &ctx);
  SCheck(ctx.reward_count == IUD(2), "iterate reward count 2");
  SCheck(ctx.reward_sum == FPD(1.5), "iterate reward sum 1.5");

  /* Fill the ring past max_count: overwrites oldest. */
  TScript small;
  TScriptInit(&small, IUD(3));
  SCheck(TScriptRecord(&small, IUD(0), ScrFrame, IUC(1), FPD(0.0), ISC(0)),
         "small: record 0");
  SCheck(TScriptRecord(&small, IUD(1), ScrFrame, IUC(1), FPD(0.0), ISC(0)),
         "small: record 1");
  SCheck(TScriptRecord(&small, IUD(2), ScrFrame, IUC(1), FPD(0.0), ISC(0)),
         "small: record 2");
  SCheck(TScriptCount(&small) == IUD(3), "small: full at 3");
  SCheck(TScriptRecord(&small, IUD(3), ScrFrame, IUC(1), FPD(0.0), ISC(0)),
         "small: overwrite oldest");
  SCheck(TScriptCount(&small) == IUD(3), "small: still 3 after overwrite");

  /* Query after overwrite: frame 0 is gone, frame 3 is the latest.
     TScriptQuery returns the LATEST matching tuple, which is frame 3. */
  SCheck(TScriptQuery(&small, ScrFrame, IUC(1), &q_val, &q_frame),
         "small: query frame after overwrite");
  SCheck(q_frame == IUD(3), "small: latest frame is 3");

  /* Clear. */
  TScriptClear(&small);
  SCheck(TScriptCount(&small) == IUD(0), "small: cleared");

  /* Null safety. */
  SCheck(!TScriptRecord(NILP, IUD(0), ScrFrame, IUC(1), FPD(0.0), ISC(0)),
         "null script record fails");
  SCheck(!TScriptQuery(NILP, ScrFrame, IUC(1), &q_val, &q_frame),
         "null script query fails");
  SCheck(!TScriptSummary(NILP, ScrFrame, &sum, &count, &min_v, &max_v),
         "null script summary fails");
  SCheck(TScriptCount(NILP) == IUD(0), "null script count is 0");

  ::_::StdOut() << "sc_pass=" << sc_pass << " sc_fail=" << sc_fail << '\n';
  return sc_fail ? "GYM_SCRIPT_FAIL: " : NILP;
}

}  //< namespace iGeekGymTest
#endif
