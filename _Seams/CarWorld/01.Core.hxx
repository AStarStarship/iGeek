// Copyright AStarship <https://astarship.net>.
#if _Seam >= _iGeekCarWorldCore
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
#include "../../CarWorld/HeadlessTests.hxx"
#include "../../CarWorld/CarRoomTests.hxx"
namespace CarWorldTest {
using namespace ::_;
using namespace iGeek; 

inline const CHA* Core(const CHA* args) {
#if _Seam >= _iGeekCarWorldCore
  _ATestBegin;
  _DROut(Headingf("Testing CarWorld (CarPhysics + CarRoom)..."));

  // Run both suites; each returns a failure count (0 = pass).
  // HeadlessTests::RunAllHeadless is in the global namespace;
  // CarRoomTests::RunAllRoom is in namespace _.
  ISC headless_fails = ::RunAllHeadless();
  ISN room_fails = ::RunAllRoom();
  if (headless_fails != 0 || room_fails != 0) {
    return "CARWORLD_FAIL: test suite reported failures";
  }
#endif
  return _NilP;
}

}  //< namespace CarWorldTest
