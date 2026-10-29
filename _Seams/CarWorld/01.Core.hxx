// Copyright AStarship <https://astarship.net>.
//
// CarWorld core seam unit: runs the headless physics/gym suite + the CarRoom
// takeover suite. Converted from the standalone HeadlessTests.hxx /
// CarRoomTests.hxx drivers (which each had their own main() and were not
// included by anything) and the old broken CarWorld/_Main.hxx maze code
// (which included a non-existent mazeagent.h). Gated on IGEEK_CARWORLD_CORE.
#if SEAM >= IGEEK_CARWORLD_CORE
#if SEAM == IGEEK_CARWORLD_CORE
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#endif
#include "../../CarWorld/HeadlessTests.hxx"
#include "../../CarWorld/CarRoomTests.hxx"

using namespace ::_;
namespace CarWorldTest {
using namespace iGeek;   // engine types (in iGeek) used unqualified

inline const CHA* Core(const CHA* args) {
#if SEAM >= IGEEK_CARWORLD_CORE
  A_TEST_BEGIN;
  D_COUT(Headingf("Testing CarWorld (CarPhysics + CarRoom)..."));

  // Run both suites; each returns a failure count (0 = pass).
  // HeadlessTests::RunAllHeadless is in the global namespace;
  // CarRoomTests::RunAllRoom is in namespace _.
  ISC headless_fails = ::RunAllHeadless();
  ISN room_fails = ::RunAllRoom();
  if (headless_fails != 0 || room_fails != 0) {
    return "CARWORLD_FAIL: test suite reported failures";
  }
#endif
  return NILP;
}

}  //< namespace CarWorldTest
