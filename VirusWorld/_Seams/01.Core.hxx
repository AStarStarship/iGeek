// Copyright AStarship <https://astarship.net>.
#if SEAM >= VIREWWORLD_CORE
#include "../Host.h"
#include "../VirusPopulation.h"
#include "../Vireworld.h"
#if SEAM == VIREWWORLD_CORE
#include "../_Debug.h"
#else
#include "../_Release.h"
#endif
#endif
using namespace ::_;
namespace VireworldTest {

inline const CHA* Core(const CHA* args) {
#if SEAM >= VIREWWORLD_CORE && USING_CONSOLE == YES_0
  A_TEST_BEGIN;

  D_COUT(Headingf("Testing Vireworld..."));

  // Create a small world and run a few frames.
  Vireworld world;
  ISN error = world.Init(48, 24, 8);
  if (error != 0) {
    D_COUT("ERROR: Vireworld::Init failed: " << error);
    return NILP;
  }
  D_COUT("World: " << world.host->width << "x"
    << world.host->height << " with " << world.population->Count()
    << " viruses");
  for (ISN frame = 0; frame < 25; frame++) {
    world.Step();
    D_COUT("\nFrame " << frame << ":\n");
    world.Render();
    world.Summary();
  }
#endif

  return NILP;
}

}  //< namespace VireworldTest
