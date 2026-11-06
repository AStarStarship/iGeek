// Copyright AStarship <https://astarship.net>.
#include <_Config.h>
#if _Seam >= _iGeekEnvironmentRoomSandbox
#include "../../RoomSandbox.hxx"
namespace iGeekGymTest {
using namespace ::_;
using namespace iGeek;

ISC rs_pass = 0;
ISC rs_fail = 0;

inline void RSCheck(BOL ok, const CHA* what) {
  if (::_::Test(ok)) {
    rs_pass++;
    ::_::RefOut() << "PASS " << what << '\n';
  } else {
    rs_fail++;
    ::_::RefOut() << "FAIL " << what << '\n';
  }
}

inline const CHA* Sandbox(const CHA* args) {
  _ATestBegin;
  ::_::RefOut() << Headingf("Testing Gym RoomSandbox (Room IS the sandbox)...");

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
  RSCheck(floor != _NilP, "Floor is not nil");
  RSCheck(floor->values.bytes == TRoom<CHA, CHC, ISC, ISC, DTB, IUC>::FloorSize,
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
  RSCheck(!RoomSandboxCreate(_NilP, "x"), "null create fails");
  RSCheck(RoomSandboxStatus(_NilP) == CRStateInvalid, "null status is Invalid");
  RSCheck(!RoomSandboxPause(_NilP), "null pause fails");
  RSCheck(!RoomSandboxResume(_NilP), "null resume fails");
  RoomSandboxDestroy(_NilP);  /* should not crash */
  RSCheck(!RoomSandboxIsRunning(_NilP), "null is not running");

  ::_::RefOut() << "rs_pass=" << rs_pass << " rs_fail=" << rs_fail << '\n';
  return rs_fail ? "GYM_ROOM_SANDBOX_FAIL: " : _NilP;
}

}  //< namespace iGeekGymTest
#endif
