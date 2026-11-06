// Copyright AStarship <https://astarship.net>.
//
// BikeWorld core seam unit: runs the 12-function BikeWorld test suite
// (tile queries, coast/terrain physics, route planning, balance control,
// headless route, scenarios, native gym batch, invariants, failure paths,
// reset contracts). The test bodies live in BikeWorld/Tests.hxx, where the
// old standalone main() was converted to RunAllTests() (returns the failure
// count, 0 = pass). This unit wraps it in the seam test API.
// No C/C++ standard library: I/O via _DROut (ASCIICrabs console primitive).
#if _Seam >= _iGeekBikeWorldCore
#include "../../../ASCIICrabs/_Debug.h"
#else
#include "../../../ASCIICrabs/_Release.h"
#endif
// Tests.hxx + BikeWorld impl are pulled ONCE by the rollup _Tests.hxx
// (unguarded); this unit relies on that rather than re-including them.
namespace BikeWorldTest {
using namespace ::_;
using namespace iGeek; 

// The core unit test. Returns _NilP on success, an error string on failure
// (the CI exit-code signal).
inline const CHA* Core(const CHA* args) {
#if _Seam >= _iGeekBikeWorldCore
  _ATestBegin;
  _DROut(Headingf("Testing BikeWorld (12 test functions)..."));

  const ISC failures = RunAllTests();
  if (failures != 0) {
    return "BIKEWORLD_FAIL: RunAllTests reported failures";
  }
  _DROut("  BikeWorld core suite clean\n");
#endif
  return _NilP;
}

}  //< namespace BikeWorldTest
