// Copyright AStarship <https://astarship.net>.
//
// Gym RoomSandbox seam unit: the Chinese Room IS the sandbox.
// Create, status, pause, resume, destroy round-trip.
// Gated on IGEEK_GYM_MANIFEST.
#pragma once
#include <_Config.h>
#if SEAM >= IGEEK_GYM_MANIFEST
#include "../../RoomSandbox.hxx"
namespace iGeekGymTest {
using namespace iGeek;

ISC rs_pass = 0;
ISC rs_fail = 0;

inline void RSCheck(BOL ok, const CHA* what) {
  if (::_::Test(ok)) {
    rs_pass++;
    ::_::StdOut() << "PASS " << what << '\n';
  } else {
    rs_fail++;
    ::_::StdOut() << "FAIL " << what << '\n';
  }
}

inline const CHA* Sandbox(const CHA* args) {
  (void)args;
  A_TEST_BEGIN;
  ::_::StdOut() << Headingf("Testing Gym RoomSandbox (Room IS the sandbox)...");

  RoomSandbox sandbox;

  /* Create. */
  RSCheck(RoomSandboxCreate(&sandbox, "test_sandbox"), "create sandbox");
  RSCheck(sandbox.active, "sandbox is active");
  RSCheck(!sandbox.paused, "sandbox not paused");
  RSCheck(RoomSandboxIsRunning(&sandbox), "sandbox is running");

  /* Status: running (Booting). */
  RSCheck(RoomSandboxStatus(&sandbox) == CRStateBooting,
         "status is Booting (running)");

  /* Floor is initialized. */
  auto* floor = sandbox.room.Floor();
  RSCheck(floor != NILP, "Floor is not nil");
  RSCheck(floor->bytes == TRoom<CHA, CHC, ISC, ISC, DTB, IUC>::FloorSize,
         "Floor size is FloorSize");

  /* Room name is set. */
  RSCheck(sandbox.room.GetRoomName()[0] == 't' &&
          sandbox.room.GetRoomName()[1] == 'e' &&
          sandbox.room.GetRoomName()[2] == 's',
         "room name is test_sandbox");

  /* Pause. */
  RSCheck(RoomSandboxPause(&sandbox), "pause sandbox");
  RSCheck(sandbox.paused, "sandbox is paused");
  RSCheck(!RoomSandboxIsRunning(&sandbox), "sandbox not running when paused");
  RSCheck(RoomSandboxStatus(&sandbox) == CRStateSleeping,
         "status is Sleeping (paused)");

  /* Double-pause fails. */
  RSCheck(!RoomSandboxPause(&sandbox), "double-pause fails");

  /* Resume. */
  RSCheck(RoomSandboxResume(&sandbox), "resume sandbox");
  RSCheck(!sandbox.paused, "sandbox not paused after resume");
  RSCheck(RoomSandboxIsRunning(&sandbox), "sandbox running after resume");
  RSCheck(RoomSandboxStatus(&sandbox) == CRStateBooting,
         "status is Booting (resumed)");

  /* Double-resume fails. */
  RSCheck(!RoomSandboxResume(&sandbox), "double-resume fails");

  /* Destroy. */
  RoomSandboxDestroy(&sandbox);
  RSCheck(!sandbox.active, "sandbox not active after destroy");
  RSCheck(!RoomSandboxIsRunning(&sandbox), "sandbox not running after destroy");
  RSCheck(RoomSandboxStatus(&sandbox) == CRStateShutdown,
         "status is Shutdown (destroyed)");

  /* Null safety. */
  RSCheck(!RoomSandboxCreate(NILP, "x"), "null create fails");
  RSCheck(RoomSandboxStatus(NILP) == CRStateInvalid, "null status is Invalid");
  RSCheck(!RoomSandboxPause(NILP), "null pause fails");
  RSCheck(!RoomSandboxResume(NILP), "null resume fails");
  RoomSandboxDestroy(NILP);  /* should not crash */
  RSCheck(!RoomSandboxIsRunning(NILP), "null is not running");

  ::_::StdOut() << "rs_pass=" << rs_pass << " rs_fail=" << rs_fail << '\n';
  return rs_fail ? "GYM_ROOM_SANDBOX_FAIL: " : NILP;
}

}  //< namespace iGeekGymTest
#endif
