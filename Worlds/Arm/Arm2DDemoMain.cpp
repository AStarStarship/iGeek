// Copyright AStarship <https://astarship.net>.
// Arm2D demo entry point.
//
// Built by the seam tree: `cmake -S _Seams -B <build>` creates the Arm2DGui
// target when SDL3 + OpenGL are found (it needs a display at run time; the
// headless ArmHeadless target verifies the same sim and canvas without one).
#include "Arm2DWebGL.hxx"
#include <ASCIICrabs/_Config.h>
#include <ASCIICrabs/Math.hxx>
#include <ASCIICrabs/_Package.hxx>

using namespace _;
using namespace iGeek;

int main(int, char**) {
  Arm2DWebGL demo;
  if (!demo.Open()) {
    return 1;
  }
  demo.Run(true);
  demo.Close();
  return 0;
}
