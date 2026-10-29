// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_AGENT_SLOT_HXX
#define IGEEK_AGENT_SLOT_HXX 1
#include "AgentSlot.h"
namespace iGeek {

void TAgentSlotInit(TAgentSlot* slot) {
  if (!slot) return;
  slot->agent_id = IUC(0);
  for (ISN i = 0; i < 32; ++i) slot->agent_name[i] = 0;
  slot->state = AgentStateGone;
  slot->join_frame = IUD(0);
  slot->last_heartbeat_frame = IUD(0);
  slot->heartbeat_timeout = ISC(100);
}

void TAgentSlotArrayInit(TAgentSlot* slots, ISC count) {
  if (!slots) return;
  for (ISC i = 0; i < count; ++i) TAgentSlotInit(&slots[i]);
}

ISC TAgentSlotFindFree(const TAgentSlot* slots, ISC count) {
  if (!slots) return -1;
  for (ISC i = 0; i < count; ++i) {
    if (slots[i].state == AgentStateGone) return i;
  }
  return -1;
}

BOL RoomJoin(TAgentSlot* slots, ISC count, IUC agent_id,
             const CHA* agent_name, IUD frame) {
  if (!slots || !agent_name) return false;
  ISC idx = TAgentSlotFindFree(slots, count);
  if (idx < 0) return false;
  slots[idx].agent_id = agent_id;
  CopyStr(slots[idx].agent_name, 32, agent_name);
  slots[idx].state = AgentStateJoining;
  slots[idx].join_frame = frame;
  slots[idx].last_heartbeat_frame = frame;
  return true;
}

BOL RoomLeave(TAgentSlot* slots, ISC count, IUC agent_id) {
  if (!slots) return false;
  for (ISC i = 0; i < count; ++i) {
    if (slots[i].agent_id == agent_id &&
        slots[i].state != AgentStateGone) {
      slots[i].state = AgentStateGone;
      slots[i].agent_id = IUC(0);
      for (ISN j = 0; j < 32; ++j) slots[i].agent_name[j] = 0;
      return true;
    }
  }
  return false;
}

BOL RoomHeartbeat(TAgentSlot* slots, ISC count, IUC agent_id, IUD frame) {
  if (!slots) return false;
  for (ISC i = 0; i < count; ++i) {
    if (slots[i].agent_id == agent_id &&
        slots[i].state == AgentStateActive) {
      slots[i].last_heartbeat_frame = frame;
      return true;
    }
  }
  return false;
}

ISC RoomEvictStale(TAgentSlot* slots, ISC count, IUD frame) {
  if (!slots) return ISC(0);
  ISC evicted = 0;
  for (ISC i = 0; i < count; ++i) {
    if (slots[i].state == AgentStateActive) {
      IUD elapsed = frame - slots[i].last_heartbeat_frame;
      if (ISC(elapsed) > slots[i].heartbeat_timeout) {
        slots[i].state = AgentStateGone;
        slots[i].agent_id = IUC(0);
        for (ISN j = 0; j < 32; ++j) slots[i].agent_name[j] = 0;
        ++evicted;
      }
    }
  }
  return evicted;
}

ISC TAgentSlotCountActive(const TAgentSlot* slots, ISC count) {
  if (!slots) return ISC(0);
  ISC n = 0;
  for (ISC i = 0; i < count; ++i) {
    if (slots[i].state == AgentStateActive) ++n;
  }
  return n;
}

}  //< namespace iGeek
#endif
