// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_ROOM_SANDBOX_HXX
#define IGEEK_ROOM_SANDBOX_HXX 1
#include "RoomSandbox.h"
namespace iGeek {

BOL RoomSandboxCreate(RoomSandbox* sandbox, const CHA* name) {
  if (!sandbox || !name) return false;
  new (&sandbox->room) TRoom<CHA, CHC, ISC, ISC, DTB, IUC>(name);
  sandbox->active = true;
  sandbox->paused = false;
  return true;
}

CRState RoomSandboxStatus(const RoomSandbox* sandbox) {
  if (!sandbox) return CRStateInvalid;
  if (!sandbox->active) return CRStateShutdown;
  if (sandbox->paused) return CRStateSleeping;
  return CRStateBooting;
}

BOL RoomSandboxPause(RoomSandbox* sandbox) {
  if (!sandbox || !sandbox->active || sandbox->paused) return false;
  sandbox->paused = true;
  return true;
}

BOL RoomSandboxResume(RoomSandbox* sandbox) {
  if (!sandbox || !sandbox->active || !sandbox->paused) return false;
  sandbox->paused = false;
  return true;
}

void RoomSandboxDestroy(RoomSandbox* sandbox) {
  if (!sandbox) return;
  sandbox->active = false;
  sandbox->paused = false;
}

BOL RoomSandboxIsRunning(const RoomSandbox* sandbox) {
  return sandbox && sandbox->active && !sandbox->paused;
}

}  //< namespace iGeek
#endif
