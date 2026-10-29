// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_ROOM_SANDBOX_H
#define IGEEK_ROOM_SANDBOX_H
#include <_Config.h>
#if SEAM >= IGEEK_GYM_MANIFEST
#include "Room.hpp"
namespace iGeek {

/* Room sandbox: the Chinese Room IS the sandbox.
No Docker, no containers. The Room is one contiguous block of memory
(Floor + Walls + Stack) that is the agent's entire world:
- The Room's Floor is the agent's observation space (the ASCII List).
- The Room's Walls are the agent's memory (contiguous blocks).
- The Room's Stack is the agent's computation (the Crabs interpreter).
- The Room's Doors are the agent's I/O (Slots to the outside).
This is a standalone Room wrapper (not `public TRoom` on the Env/Gym
base, which is broken upstream — see Env.h). The sandbox provides
the lifecycle: create, status, pause, resume, destroy. */

/* The room sandbox: a TRoom with a fixed Floor and optional Walls. */
struct RoomSandbox {
  TRoom<CHA, CHC, ISC, ISC, DTB, IUC> room;
  BOL active;
  BOL paused;
};

/* Creates a room sandbox: allocates the Room (Floor + Walls).
   @param name       The room name.
   @return           True on success, false on allocation failure. */
BOL RoomSandboxCreate(RoomSandbox* sandbox, const CHA* name);

/* Returns the room's status: CRStateBooting/Sleeping/Shutdown/Invalid.
   The sandbox maps its own active/paused flags onto the Room state. */
CRState RoomSandboxStatus(const RoomSandbox* sandbox);

/* Pauses the room: saves state, stops the interpreter. */
BOL RoomSandboxPause(RoomSandbox* sandbox);

/* Resumes the room: restores state, restarts the interpreter. */
BOL RoomSandboxResume(RoomSandbox* sandbox);

/* Destroys the room: frees the Floor and Walls. */
void RoomSandboxDestroy(RoomSandbox* sandbox);

/* Returns true if the sandbox is active and not paused. */
BOL RoomSandboxIsRunning(const RoomSandbox* sandbox);

}  //< namespace iGeek
#endif
#endif
