// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_AGENT_SLOT_H
#define IGEEK_AGENT_SLOT_H
#include <_Config.h>
#if SEAM >= IGEEK_GYM_MANIFEST
#include "GymManifest.h"
namespace iGeek {

/* Max agents that can be active in one Gym (one per Slot). */
enum { AgentSlotMax = GymEnvMax };

/* Agent lifecycle states. */
enum {
  AgentStateGone = 0,
  AgentStateJoining,
  AgentStateActive,
  AgentStateLeaving,
};

/* A fixed-size agent slot: one entry per agent in the Gym.
   Stored in the Room's Floor (or a Wall). The agent reads the manifest
   through its Slot; the Gym writes the manifest on join. */
struct TAgentSlot {
  IUC agent_id;
  CHA agent_name[32];
  ISC state;
  IUD join_frame;
  IUD last_heartbeat_frame;
  ISC heartbeat_timeout;
};

/* Initializes an agent slot to Gone state. */
void TAgentSlotInit(TAgentSlot* slot);

/* Initializes an array of agent slots (all Gone). */
void TAgentSlotArrayInit(TAgentSlot* slots, ISC count);

/* Finds a free slot (state == Gone). Returns the index or -1 if full. */
ISC TAgentSlotFindFree(const TAgentSlot* slots, ISC count);

/* Joins an agent: finds a free slot, writes the agent_id/name, sets state
   to Joining, records join_frame. The caller writes the manifest into the
   agent's Floor view and then sets state to Active.
   @return True on success, false if no free slot. */
BOL RoomJoin(TAgentSlot* slots, ISC count, IUC agent_id,
             const CHA* agent_name, IUD frame);

/* Leaves an agent: sets state to Leaving, then Gone.
   @return True on success, false if agent not found. */
BOL RoomLeave(TAgentSlot* slots, ISC count, IUC agent_id);

/* Heartbeat: updates last_heartbeat_frame for an active agent.
   @return True on success, false if agent not found or not active. */
BOL RoomHeartbeat(TAgentSlot* slots, ISC count, IUC agent_id, IUD frame);

/* Evicts stale agents: any Active agent with no heartbeat for
   heartbeat_timeout frames is set to Gone.
   @return Number of agents evicted. */
ISC RoomEvictStale(TAgentSlot* slots, ISC count, IUD frame);

/* Counts active agents. */
ISC TAgentSlotCountActive(const TAgentSlot* slots, ISC count);

}  //< namespace iGeek
#endif
#endif
