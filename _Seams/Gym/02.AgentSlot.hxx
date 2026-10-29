// Copyright AStarship <https://astarship.net>.
//
// Gym AgentSlot seam unit: join/leave/heartbeat/eviction round-trip.
// Gated on IGEEK_GYM_MANIFEST.
#pragma once
#include <_Config.h>
#if SEAM >= IGEEK_GYM_MANIFEST
#include "../../AgentSlot.hxx"
namespace iGeekGymTest {
using namespace iGeek;

ISC a_pass = 0;
ISC a_fail = 0;

inline void ACheck(BOL ok, const CHA* what) {
  if (::_::Test(ok)) {
    a_pass++;
    ::_::StdOut() << "PASS " << what << '\n';
  } else {
    a_fail++;
    ::_::StdOut() << "FAIL " << what << '\n';
  }
}

inline const CHA* AgentSlot(const CHA* args) {
  (void)args;
  A_TEST_BEGIN;
  ::_::StdOut() << Headingf("Testing Gym AgentSlot (join/leave/heartbeat)...");

  TAgentSlot slots[AgentSlotMax];
  TAgentSlotArrayInit(slots, AgentSlotMax);

  /* Initially no active agents. */
  ACheck(TAgentSlotCountActive(slots, AgentSlotMax) == ISC(0),
         "initially no active agents");

  /* Join agent 1. */
  ACheck(RoomJoin(slots, AgentSlotMax, IUC(1), "alpha", IUD(0)),
         "join agent 1");
  ACheck(slots[0].agent_id == IUC(1), "agent 1 id");
  ACheck(slots[0].agent_name[0] == 'a', "agent 1 name");
  ACheck(slots[0].state == AgentStateJoining, "agent 1 state Joining");
  ACheck(slots[0].join_frame == IUD(0), "agent 1 join frame");

  /* Promote to Active (simulates manifest write + activation). */
  slots[0].state = AgentStateActive;

  /* Join agent 2. */
  ACheck(RoomJoin(slots, AgentSlotMax, IUC(2), "beta", IUD(10)),
         "join agent 2");
  ACheck(slots[1].agent_id == IUC(2), "agent 2 id");
  ACheck(slots[1].state == AgentStateJoining, "agent 2 state Joining");
  slots[1].state = AgentStateActive;

  /* Two active agents. */
  ACheck(TAgentSlotCountActive(slots, AgentSlotMax) == ISC(2),
         "two active agents");

  /* Heartbeat agent 1. */
  ACheck(RoomHeartbeat(slots, AgentSlotMax, IUC(1), IUD(20)),
         "heartbeat agent 1");
  ACheck(slots[0].last_heartbeat_frame == IUD(20), "agent 1 heartbeat frame");

  /* Heartbeat agent 2. */
  ACheck(RoomHeartbeat(slots, AgentSlotMax, IUC(2), IUD(25)),
         "heartbeat agent 2");

  /* Evict at frame 25: no stale agents (both heartbeated recently). */
  ACheck(RoomEvictStale(slots, AgentSlotMax, IUD(25)) == ISC(0),
         "no eviction at frame 25");
  ACheck(TAgentSlotCountActive(slots, AgentSlotMax) == ISC(2),
         "still two active at frame 25");

  /* Evict at frame 126: agent 1 (last hb 20, timeout 100, elapsed 106 > 100)
     and agent 2 (last hb 25, timeout 100, elapsed 101 > 100) are both stale. */
  ISC evicted = RoomEvictStale(slots, AgentSlotMax, IUD(126));
  ACheck(evicted == ISC(2), "evict 2 stale agents at frame 126");
  ACheck(TAgentSlotCountActive(slots, AgentSlotMax) == ISC(0),
         "no active agents after eviction");

  /* Leave a non-existent agent. */
  ACheck(!RoomLeave(slots, AgentSlotMax, IUC(99)),
         "leave non-existent agent fails");

  /* Join and leave. */
  ACheck(RoomJoin(slots, AgentSlotMax, IUC(3), "gamma", IUD(200)),
         "join agent 3");
  ACheck(RoomLeave(slots, AgentSlotMax, IUC(3)), "leave agent 3");
  ACheck(slots[0].state == AgentStateGone || slots[1].state == AgentStateGone ||
         slots[2].state == AgentStateGone,
         "agent 3 slot is Gone after leave");

  /* Fill all slots, then try to join one more. */
  TAgentSlotArrayInit(slots, AgentSlotMax);
  for (ISC i = 0; i < AgentSlotMax; ++i) {
    RoomJoin(slots, AgentSlotMax, IUC(i + 10), "x", IUD(300));
  }
  ACheck(TAgentSlotFindFree(slots, AgentSlotMax) < 0,
         "no free slots when full");
  ACheck(!RoomJoin(slots, AgentSlotMax, IUC(99), "overflow", IUD(300)),
         "join fails when full");

  ::_::StdOut() << "a_pass=" << a_pass << " a_fail=" << a_fail << '\n';
  return a_fail ? "GYM_AGENT_SLOT_FAIL: " : NILP;
}

}  //< namespace iGeekGymTest
#endif
